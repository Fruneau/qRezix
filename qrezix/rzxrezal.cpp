/***************************************************************************
                          rzxrezal.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is  software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the  Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qapplication.h>
#include <qobjectlist.h>

#include "rzxrezal.h"

#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxitem.h"
#include "rzxcomputer.h"
#include "rzxpluginloader.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"


#define USER_HASH_TABLE_LENGTH 1663


const char * RzxRezal::colNames[RzxRezal::numColonnes] =
		{ QT_TR_NOOP("Icon"), QT_TR_NOOP("Computer name"), QT_TR_NOOP("Comment"),
			QT_TR_NOOP("Samba"), QT_TR_NOOP("FTP"), 
			/*QT_TR_NOOP("Hotline"),*/ QT_TR_NOOP("Web"), 
			QT_TR_NOOP("News"), QT_TR_NOOP("OS"), 
			QT_TR_NOOP("Gateway"), QT_TR_NOOP("Promo") };


RzxRezal::RzxRezal(QWidget * parent, const char * name) : QListView(parent, name), itemByIp(USER_HASH_TABLE_LENGTH)
{
	selected = NULL;
	
	int i;
	for (i = 0; i < numColonnes; i++) {
		addColumn(tr(colNames[i]));
		setColumnWidthMode(i, Manual);
		if (i > ColRemarque) setColumnAlignment(i, Qt::AlignCenter);
	}
	setColumnAlignment(ColNom, Qt::AlignCenter);
	setColumnAlignment(ColRemarque, Qt::AlignVCenter);

	setHScrollBarMode(QScrollView::Auto);

	setAllColumnsShowFocus(true);
 
	client = RzxClientListener::object();
	lister = RzxConnectionLister::global();
	
	connect(lister, SIGNAL(login(RzxComputer*)), this, SLOT(login(RzxComputer*)));
	connect(lister, SIGNAL(connectionEtablished()), this, SLOT(init()));
	connect(lister, SIGNAL(loginEnd()), this, SLOT(forceSort()));
	connect(lister, SIGNAL(logout(const QString& )), this, SLOT(logout(const QString& )));

	// On est obligé d'utiliser ce signal pour savoir dans quelle colonne le
	// double-clic suivant a lieu
	connect(this, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(onListClicked(QListViewItem *, const QPoint &, int)));
	connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(onListDblClicked(QListViewItem *)));

	lastColumnClicked = ColIcone;
  
	// GESTION DU MENU CONTEXTUEL
	connect(this,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int )),this,SLOT(creePopUpMenu(QListViewItem *,const QPoint &,int )));
	connect(this, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(chatCreate()));
	connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(redrawSelectedIcon(QListViewItem*)));
}

RzxRezal::~RzxRezal(){
}

void RzxRezal::showNotFavorites(bool val)
{
	dispNotFavorites = val;
}

RzxPopupMenu::RzxPopupMenu(QWidget * parent, const char * name) : QPopupMenu(parent, name) {
}

void RzxPopupMenu::keyPressEvent(QKeyEvent *e) {
	if(e->key()!=Qt::Key_Left) {
		QPopupMenu::keyPressEvent(e);
		return;
	}
	close();
} 

void RzxRezal::creePopUpMenu(QListViewItem *ordinateurSelect,const QPoint & pos, int){
	if(ordinateurSelect)
	{ 
		RzxItem* item=(RzxItem*) ordinateurSelect;
		RzxComputer *computer = item->getComputer();
		int serveurs=item->servers;
		popup.clear();
  
		if(computer->getName() != RzxConfig::localHost()->getName() && !computer->getRepondeur())
			popup.insertItem(*RzxConfig::themedIcon("chat"), tr("begin &Chat"),this,SLOT(chatCreate()));
		if(serveurs & 1) popup.insertItem(*RzxConfig::themedIcon("samba"), tr("Samba connect"),this,SLOT(samba()));
		if((serveurs>>1) & 1) popup.insertItem(*RzxConfig::themedIcon("ftp"), tr("FTP connect"), this, SLOT(ftp()));
		if((serveurs>>3) & 1) popup.insertItem(*RzxConfig::themedIcon("http"), tr("browse Web"), this, SLOT(http()));
		if((serveurs>>4) & 1) popup.insertItem(*RzxConfig::themedIcon("news"), tr("read News"), this, SLOT(news()));
		popup.insertSeparator();
		popup.insertItem(*RzxConfig::themedIcon("historique"), tr("History"),this,SLOT(historique()));
		popup.insertItem(*RzxConfig::themedIcon("prop"), tr("Properties"),this,SLOT(proprietes()));
		popup.insertSeparator();
		if(RzxConfig::globalConfig()->favorites->find(ordinateurSelect->text(1)))
			popup.insertItem(*RzxConfig::themedIcon("not_favorite"), tr("Remove from favorites"),this,SLOT(removeFromFavorites()));
		else
			popup.insertItem(*RzxConfig::themedIcon("favorite"), tr("Add to favorites"),this,SLOT(addToFavorites()));
		popup.insertSeparator();
		popup.insertItem(*RzxConfig::themedIcon("cancel"), tr("Cancel"), &popup, SLOT(hide()));
		RzxPlugInLoader::global()->menuItem(popup);
		popup.popup(pos);
	}
}

void RzxRezal::proprietes(const RzxHostAddress& peer)
{
	RzxChat * object = lister->chats.find(peer.toString());
	RzxComputer * computer = lister->iplist.find(peer.toString());
	if (!computer)
		return;
	if (!object)
		client->checkProperty(peer);
	else
	{
		if(object->getSocket())
			object->getSocket()->sendPropQuery();
		else
			client->checkProperty(peer);
	}
}

void RzxRezal::proprietes(){
	RzxItem* item=(RzxItem*) currentItem();
	proprietes(item->ip);
}

void RzxRezal::historique(){
	RzxItem * item=(RzxItem*) currentItem();
	QString hostname = lister->iplist.find(item -> ip.toString()) -> getName();
	if(!RzxChatSocket::showHistorique( item -> ip.toRezix(), hostname))
		emit status(tr("No history file for user %1").arg(hostname), false);
}

void RzxRezal::removeFromFavorites(){
	QString temp=currentItem()->text(ColNom);
	RzxConfig::globalConfig()->favorites->remove(temp);
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteChanged();
}

void RzxRezal::addToFavorites(){
	QString temp=currentItem()->text(ColNom);
	RzxConfig::globalConfig()->favorites->insert(temp,new QString("1"));
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteChanged();
}

///Lancement du client ftp
void RzxRezal::ftp()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::ftp(temp);
}

///Lancement du client http
void RzxRezal::http()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::http(temp);
}

///Lancement du client news
void RzxRezal::news()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::news(temp);
}

///Lancement du client samba
void RzxRezal::samba()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::samba(temp);
}


void RzxRezal::afficheColonnes(){
	int colonnesAffichees=RzxConfig::colonnes();
	int i;

	for(i =0; i<columns(); i++){
		setColumnWidthMode(i,Manual);
		setColumnWidth(i,0);
		if((colonnesAffichees>>i) & 1){
			if(i==0)
			{
				setColumnWidth(i, ((RzxConfig::computerIconSize() || RzxConfig::computerIconHighlight()) ?64:32 )+4);
			} else if(i==1) {
				adjustColumn (i); setColumnWidthMode(i,QListView::Maximum);
			} else if(i==2) {
				//if(columnWidth(i)==0) setColumnWidth(i,200);
			} else {
				setColumnWidth(i,40);
			}
		}
	}
	adapteColonnes();
}

void RzxRezal::adapteColonnes(){
	int colonnesAffichees=RzxConfig::colonnes();
	int somme=0;
	int i;
	const int colrem=2;

	for(i =0;i<columns();i++){
		if(i!=colrem)
			somme+=columnWidth(i);
	}
  
	if(((colonnesAffichees >> colrem) & 1)) {
		if(width()>(somme+110))
			setColumnWidth(colrem, width()-somme-10);
		else
			setColumnWidth(colrem,100);
	}
	triggerUpdate(); 
}

/** No descriptions */
void RzxRezal::login(RzxComputer *computer)
{
	RzxItem *item = itemByIp.find(computer->getIP().toString());
	if(!item)
	{
		item = new RzxItem(computer, this, dispNotFavorites);
		itemByIp.insert(computer->getIP().toString(), item);
		connect(this, SIGNAL(favoriteChanged()), item, SLOT(update()));
	}
	
	connect(computer, SIGNAL(isUpdated()), item, SLOT(update()));

	item -> update();
}

/** Déconnexion d'un personne */
void RzxRezal::logout(const QString& ip)
{
	RzxItem *item = itemByIp.take(ip);
	if((selected == item))
		selected = NULL;
	item->deleteLater();
}

/** Réinitialisation de la liste des personnes connectées */
void RzxRezal::init()
{
	itemByIp.clear();
	selected = NULL;
}

/** Tri les items */
void RzxRezal::forceSort()
{
	afficheColonnes();
	sort();
}


/*************************
* GESTION DU CHAT
*/

RzxChat * RzxRezal::chatCreate(const QString& login)
{
	QString m_login = login;
	if(!login)
		m_login = currentItem()->text(ColNom);
	return lister->chatCreate(m_login);
}


/** CRASSOU AU POSSIBLE
* TODO: gerer ca autrement */
void RzxRezal::onListDblClicked(QListViewItem * sender){
	RzxItem * item = static_cast<RzxItem*> (sender);
	int serveurs=item->servers;

	bool gere = false;

	// Quelle colonne a été cliquée ?
	switch( lastColumnClicked )
	{
	case ColSamba:
        	if( serveurs & 0x01 ) { gere = true; samba(); }
		break;

	case ColFTP:
        	if( serveurs & 0x02 ) { gere = true; ftp(); }
		break;

	case ColHTTP:
        	if( serveurs & 0x08 ) { gere = true; http(); }
		break;

	case ColNews:
        	if( serveurs & 0x10 ) { gere = true; news(); }
		break;

	default: ;
	}

	if( !gere ) // par défaut -> chat ou ftp.
	{
		if(RzxConfig::globalConfig()->doubleClicRole() && (serveurs & RzxComputer::SERVER_FTP)){
			ftp();
		}
		else {/*chat*/
			RzxItem * listItem = (RzxItem *) sender;
			lister->chatCreate(listItem -> ip);
		}
	}
}

/** Memorise la colonne cliquee */
void RzxRezal::onListClicked( QListViewItem * item, const QPoint & pnt, int c ){
	lastColumnClicked = (NumColonne) c;
}

void RzxRezal::resizeEvent(QResizeEvent * e) {
	QListView::resizeEvent(e);
	adapteColonnes();
}

void RzxRezal::keyPressEvent(QKeyEvent *e) {
	//QListView::keyPressEvent(e);
	QString s=e->text();

#ifndef WIN32
	if(s.isNull() == TRUE)
		return;
	QChar c=s.at(0);
	if(!c.isLetter()) {
#else
	QChar c;
	if(s.isNull() != TRUE)
		c = s.at(0);
	if(s.isNull() == TRUE || !c.isLetter()) {
#endif
		if(e->key()==Qt::Key_Right) {
			QRect r(itemRect( currentItem())); //r != 0 car the currentItem IS visible
			QPoint qp=r.center();
			qp.setY(qp.y()+r.height());
			creePopUpMenu(currentItem(), qp,0);
			return;
		}
		QListView::keyPressEvent(e); //on laisse Qt gérer
		return;
	}
	RzxItem* item=static_cast<RzxItem*> (currentItem());
	RzxHostAddress ipSaved=item->ip; //y a des chances que item soit ! null :)
	while(true) {
		item=static_cast<RzxItem*>(item->itemBelow());
		if(!item)
			item=static_cast<RzxItem*>(firstChild()); //au bout on revient au début
		if(item->ip == ipSaved)
			return; //on a fait le tour, personne avec cette lettre
		RzxComputer * comp=lister->iplist.find((item->ip).toString());
		if(comp->getName().lower().at(0)==c) {
			setCurrentItem(item);
			ensureItemVisible(item);
			return;
		}
	}
	//ICI JAI LAISSE DS TRUCS EN PLAN
	//
	//
	// OUI ICI OUI !!!!!!!!!!!!!!!!!!
	//
	//
	//BON CA DEVRAIT SUFFIRE LA  C ASSEZ VISIBLE
}

/** No descriptions */
void RzxRezal::redrawAllIcons(){
//	if (!RzxConfig::computerIconSize())
		setColumnWidth(0, 64);
 
	for (QListViewItemIterator it(this); it.current(); ++it) {
		RzxItem * item = static_cast<RzxItem *> (it.current());
		item -> update();
	}
}

///Mise à jour de la taille de l'icône sélectionnée
void RzxRezal::redrawSelectedIcon(QListViewItem *sel)
{
	if(selected) selected->update();
	selected = (RzxItem*)sel;
	if(selected) selected->update();
}

void RzxRezal::languageChanged(){
	for (int i = 0; i < numColonnes; i++) {
		setColumnText(i, tr(colNames[i]));
	}
}


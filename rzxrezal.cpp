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
#include <qtooltip.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qimage.h>

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
			QT_TR_NOOP("Gateway"), QT_TR_NOOP("Promo"),
			QT_TR_NOOP("Place"), QT_TR_NOOP("IP"), QT_TR_NOOP("Client") };


RzxRezal::RzxRezal(QWidget * parent, const char * name) : QListView(parent, name), itemByIp(USER_HASH_TABLE_LENGTH)
{
	timer = false;
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
	
	//connect(lister, SIGNAL(login(RzxComputer*)), this, SLOT(login(RzxComputer*)));
	connect(lister, SIGNAL(login(RzxComputer*)), this, SLOT(bufferedLogin(RzxComputer *)));
	connect(lister, SIGNAL(connectionEtablished()), this, SLOT(init()));
	connect(lister, SIGNAL(logout(const QString& )), this, SLOT(logout(const QString& )));

	// On est oblig� d'utiliser ce signal pour savoir dans quelle colonne le
	// double-clic suivant a lieu
	connect(this, SIGNAL(pressed(QListViewItem *, const QPoint &, int)),
		this, SLOT(onListClicked(QListViewItem *, const QPoint &, int)));
	connect(this, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(onListDblClicked(QListViewItem *)));

	lastColumnClicked = ColIcone;
  
	// GESTION DU MENU CONTEXTUEL
	connect(this,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int )),this,SLOT(creePopUpMenu(QListViewItem *,const QPoint &,int )));
	connect(this, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(onListDblClicked(QListViewItem *)));
	connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(redrawSelectedIcon(QListViewItem*)));
	connect(this, SIGNAL(onItem(QListViewItem*)), this, SLOT(buildToolTip(QListViewItem*)));
	filter = QString::null;
	filterOn = false;
	setUpdatesEnabled (TRUE);
}

RzxRezal::~RzxRezal(){
	if(!timer)
		delete timer;
}

void RzxRezal::showNotFavorites(bool val)
{
	dispNotFavorites = val;
}

void RzxRezal::loginFromLister(bool val)
{
	if(!val)
		disconnect(lister, SIGNAL(login(RzxComputer*)), this, SLOT(bufferedLogin(RzxComputer*)));
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
		
		QPixmap pixmap;
		#define newItem(name, trad, receiver, slot)  { pixmap = *RzxConfig::themedIcon(name); \
			if(!pixmap.isNull()) \
			{ \
				QImage image = pixmap.convertToImage(); \
				image = image.smoothScale(16,16); \
				pixmap.convertFromImage(image); }\
			popup.insertItem(pixmap, trad, receiver, slot); }
  
		if(item->ignored) {
			if(serveurs & 1) newItem("samba", tr("Samba connect"), this, SLOT(samba()));
			if((serveurs>>1) & 1) newItem("ftp", tr("FTP connect"), this, SLOT(ftp()));
			if((serveurs>>3) & 1) newItem("http", tr("browse Web"), this, SLOT(http()));
			if((serveurs>>4) & 1) newItem("news", tr("read News"), this, SLOT(news()));
			popup.insertSeparator();
			newItem("unban", tr("Remove from ignore list"), this, SLOT(removeFromIgnoreList()));
		}
		else {
			if(computer->getName() != RzxConfig::localHost()->getName() && !computer->getRepondeur() && computer->can(RzxComputer::CAP_CHAT))
				newItem("chat", tr("begin &Chat"), this, SLOT(chatCreate()));
				//popup.insertItem(*RzxConfig::themedIcon("chat"), tr("begin &Chat"),this,SLOT(chatCreate()));
			if(serveurs & 1) newItem("samba", tr("Samba connect"), this, SLOT(samba()));
			if((serveurs>>1) & 1) newItem("ftp", tr("FTP connect"), this, SLOT(ftp()));
			if((serveurs>>3) & 1) newItem("http", tr("browse Web"), this, SLOT(http()));
			if((serveurs>>4) & 1) newItem("news", tr("read News"), this, SLOT(news()));
			popup.insertSeparator();
			newItem("historique", tr("History"), this, SLOT(historique()));
			newItem("prop", tr("Properties"), this, SLOT(proprietes()));
			popup.insertSeparator();
			if(RzxConfig::globalConfig()->favorites->find(ordinateurSelect->text(1)))
			{
				newItem("not_favorite", tr("Remove from favorites"), this, SLOT(removeFromFavorites()));
			}
			else {
				newItem("favorite", tr("Add to favorites"), this, SLOT(addToFavorites()));
				newItem("ban", tr("Add to ignore list"), this, SLOT(addToIgnoreList()));
			}
		}
		popup.insertSeparator();
		newItem("cancel", tr("Cancel"), &popup, SLOT(hide()));
		RzxPlugInLoader::global()->menuItem(popup);
		popup.popup(pos);
		
		#undef newItem
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

void RzxRezal::setFilter(const QString& text)
{
	filter = text;
	activeFilter(filterOn);
}

void RzxRezal::activeFilter(bool on)
{
	RzxItem *item = (RzxItem*)firstChild();
	while(item)
	{
		if(!on) item->setVisible(true);
		else item->setVisible(!item->text(ColNom).find(filter, 0, false));
		
		item = (RzxItem*)item->nextSibling();
	}

	filterOn = on;
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

void RzxRezal::removeFromFavorites()
{
	RzxItem *item = (RzxItem*)currentItem();
	QString temp= item->text(ColNom);
	RzxConfig::globalConfig()->favorites->remove(temp);
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteRemoved(item->getComputer());
}

void RzxRezal::addToFavorites()
{
	RzxItem *item = (RzxItem*)currentItem();
	QString temp= item->text(ColNom);
	RzxConfig::globalConfig()->favorites->insert(temp,new QString("1"));
	RzxConfig::globalConfig()->writeFavorites();
	emit favoriteAdded(item->getComputer());
}

void RzxRezal::removeFromIgnoreList()
{
	RzxItem *item = (RzxItem*)currentItem();
	item -> ignored = false;
	RzxComputer * c = item->getComputer();
	QString ip = c->getIP().toString();
	RzxConfig::globalConfig()->ignoreList->remove(ip);
	RzxConfig::globalConfig()->writeIgnoreList();
}

void RzxRezal::addToIgnoreList()
{
	RzxItem *item = (RzxItem*)currentItem();
	item -> ignored = true;
	QString temp= item->text(ColNom);
	RzxComputer * c = item->getComputer();
	QString ip = c->getIP().toString();
	RzxConfig::globalConfig()->ignoreList->insert(ip,new QString("1"));
	RzxConfig::globalConfig()->writeIgnoreList();
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
	setUpdatesEnabled ( FALSE);
	int colonnesAffichees=RzxConfig::colonnes();
	int i;
	for(i =0; i<columns(); i++){
		setColumnWidthMode(i,Manual);
		if((colonnesAffichees>>i) & 1){
			switch(i)
			{
				case ColIcone:
					setColumnWidth(i, ((RzxConfig::computerIconSize() || RzxConfig::computerIconHighlight()) ?64:32 )+4);
					break;
					
				case ColNom: case ColResal: case ColIP: case ColClient:
					adjustColumn (i);
					setColumnWidthMode(i,QListView::Maximum);
					break;
					
				case ColRemarque: break;
				default:
					setColumnWidth(i,40);
			}
		}
		else
			setColumnWidth(i,0);
	}
	adapteColonnes();
}

void RzxRezal::adapteColonnes(){
	setUpdatesEnabled ( FALSE);
	int colonnesAffichees=RzxConfig::colonnes();
	int somme=0;
	int i;

	for(i =0;i<columns();i++){
		if(i!=ColRemarque)
			somme+=columnWidth(i);
	}
  
	if(((colonnesAffichees >> ColRemarque) & 1)) {
		if(width()>(somme+110))
			setColumnWidth(ColRemarque, width()-somme-20);
		else
			setColumnWidth(ColRemarque, 100);
	}
	setUpdatesEnabled (TRUE);
	triggerUpdate(); 
}

void RzxRezal::bufferedLogin(RzxComputer *computer) {
	if(!dispNotFavorites && !RzxConfig::globalConfig()->favorites->find( computer->getName())) return;
	
	RzxItem *item = itemByIp.find(computer->getIP().toString());
	if(!item)
	{
		setUpdatesEnabled(!dispNotFavorites);
		item = new RzxItem(computer, this, dispNotFavorites);
		itemByIp.insert(computer->getIP().toString(), item);
		item -> setVisible(!dispNotFavorites);
		if(!dispNotFavorites)
			emit newFavorite(computer);
	}
	else if(!dispNotFavorites)
		emit changeFavorite(computer);

	connect(computer, SIGNAL(isUpdated()), item, SLOT(update()));

	item -> update();
	setUpdatesEnabled(TRUE);
}

void RzxRezal::logBufLogins() { //en fait vu que le QPtrList faisait des segfaults, je trace toute la listview, c pas tr�s long
	QListViewItem *item;
	QListViewItemIterator it(this);
	while(item=(it++).current())
	{
		if(!item->isVisible())
			item->setVisible(!filterOn || !item->text(ColNom).find(filter, 0, false));
	}
}

/** D�connexion d'un personne */
void RzxRezal::logout(const QString& ip)
{
	RzxItem *item = itemByIp.take(ip);
	if(!item) return;

	if(!dispNotFavorites)
		emit lostFavorite(item->getComputer());
		
	if((selected == item))
		selected = NULL;
	item->deleteLater();
}

/** D�connexion � partir d'un RzxComputer* */
void RzxRezal::logout(RzxComputer *computer)
{
	logout(computer->getIP().toString());
}

/** On vide la liste des gens connect�s */
void RzxRezal::clear()
{
	RzxItem *item = (RzxItem*)firstChild();
	while(item)
	{
		RzxItem *next = (RzxItem*)item->nextSibling();
		logout(item->getComputer());
		item = next;
	}
	init();
	QListView::clear();
}

/** R�initialisation de la liste des personnes connect�es */
void RzxRezal::init()
{
	itemByIp.clear();
	if(!timer)
	{
		timer = new QTimer(this, "timer");
		connect( timer, SIGNAL(timeout()), this, SLOT(logBufLogins()));
	}
	if(!timer->isActive())
		timer -> start(1000); //ttes les 4 secondes
	selected = NULL;
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
	if(item->ignored) //ignored user, donc on ne va pas communiquer avec lui, faut savoir ce qu'on veut hein
		return;
	
	int serveurs=item->servers;

	bool gere = false;

	// Quelle colonne a �t� cliqu�e ?
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

	if( !gere ) // par d�faut -> chat ou ftp.
	{
		if(RzxConfig::globalConfig()->doubleClicRole() && (serveurs & RzxComputer::SERVER_FTP)){
			ftp();
		}
		else {/*chat*/
			RzxItem * listItem = (RzxItem *) sender;
			RzxComputer *computer = listItem->getComputer();
			if(computer->getName() != RzxConfig::localHost()->getName() && !computer->getRepondeur() && computer->can(RzxComputer::CAP_CHAT))
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
		QListView::keyPressEvent(e); //on laisse Qt g�rer
		return;
	}
	RzxItem* item=static_cast<RzxItem*> (currentItem());
	RzxHostAddress ipSaved=item->ip; //y a des chances que item soit ! null :)
	while(true) {
		item=static_cast<RzxItem*>(item->itemBelow());
		if(!item)
			item=static_cast<RzxItem*>(firstChild()); //au bout on revient au d�but
		if(item->ip == ipSaved)
			return; //on a fait le tour, personne avec cette lettre
		RzxComputer * comp=lister->iplist.find((item->ip).toString());
		if(comp->getName().lower().at(0)==c.lower()) {
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

///Mise � jour de la taille de l'ic�ne s�lectionn�e
void RzxRezal::redrawSelectedIcon(QListViewItem *sel)
{
	if(selected) selected->update();
	selected = (RzxItem*)sel;
	if(selected) selected->update();
}

///Mise � jour du tooltip de la fen�tre
/** Le tooltip permet d'avoir des informations sur la personne, �a peut �tre pratique pour avoir en particulier une vue r�duite mais en conservant l'acc�s faciles aux donn�es ftp, http, promo... */
void RzxRezal::buildToolTip(QListViewItem *i)
{
	QToolTip::remove(this->viewport());
	int tooltipFlags = RzxConfig::tooltip();
	if(!i || !(tooltipFlags & (int)RzxConfig::Enable) || tooltipFlags==(int)RzxConfig::Enable) return;
	
	RzxItem *item = (RzxItem*)i;
	RzxComputer *computer = item->getComputer();
	QString tooltip = "<b>"+ i->text(ColNom) + " </b>";
	if(tooltipFlags & (int)RzxConfig::Promo)
		tooltip += "<i>(" + computer->getPromoText() + ")</i>";
	tooltip += "<br><br>";
 	tooltip += "<b><i>" + tr("Informations :") + "</b></i><br><ul>";
	
	int servers = computer->getServers();
	if(servers & RzxComputer::SERVER_FTP && (tooltipFlags & (int)RzxConfig::Ftp))
		tooltip += "<li>" + tr("ftp server : ") + tr("<b>on</b>") + "</li>";
	if(servers & RzxComputer::SERVER_HTTP && (tooltipFlags & (int)RzxConfig::Http))
		tooltip += "<li>" + tr("web server : ") + tr("<b>on</b>") + "</li>";
	if(servers & RzxComputer::SERVER_NEWS && (tooltipFlags & (int)RzxConfig::News))
		tooltip += "<li>" + tr("news server : ") + tr("<b>on</b>") + "</li>";
	if(servers & RzxComputer::SERVER_SAMBA && (tooltipFlags & (int)RzxConfig::Samba))
		tooltip += "<li>" + tr("samba server : ") + tr("<b>on</b>") + "</li>";
	if(tooltipFlags & (int)RzxConfig::OS)
	{
		tooltip += "<li> os : ";
		switch((int)computer->getOptions().SysEx)
		{
			case 1: tooltip += "Windows 9x/Me"; break;
			case 2: tooltip += "Windows NT/2000/XP"; break;
			case 3: tooltip += "Linux"; break;
			case 4: tooltip += "MacOS"; break;
			case 5: tooltip += "MacOS X"; break;
			default: tooltip += tr("Unknown");
		}
		tooltip += "</li>";
	}
	if(tooltipFlags & (int)RzxConfig::Client)
		tooltip += "<li>" + computer->getClient() + "</li>";
	if(tooltipFlags & (int)RzxConfig::Features)
	{
		if(computer->can(RzxComputer::CAP_ON))
		{
			int nb = 0;
			tooltip += "<li>" + tr("features : ");
			if(computer->can(RzxComputer::CAP_CHAT))
			{
				tooltip += tr("chat");
				nb++;
			}
			if(computer->can(RzxComputer::CAP_XPLO))
			{
				tooltip += QString(nb?", ":"") + "Xplo";
				nb++;
			}
			if(!nb) tooltip += tr("none");
		}
	}
	if(tooltipFlags & (int)RzxConfig::IP)
		tooltip += "<li> ip : <i>" + computer->getIP().toString() + "</i></li>";
	if(tooltipFlags & (int)RzxConfig::Resal)
		tooltip += "<li>" + tr("place : ") + computer->getResal(false) + "</li>";
	tooltip +="</ul>";
	
	QToolTip::add(this->viewport(), itemRect(i), tooltip);
}

void RzxRezal::languageChanged(){
	for (int i = 0; i < numColonnes; i++) {
		setColumnText(i, tr(colNames[i]));
	}
}


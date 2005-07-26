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
#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QPixmap>
#include <QImage>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMenu>

#include "rzxrezal.h"

#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxitem.h"
#include "rzxcomputer.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"
#include "rzxrezalpopup.h"

#define USER_HASH_TABLE_LENGTH 1663

const QString RzxRezal::colNames[RzxRezal::numColonnes] =
		{ QT_TR_NOOP("Icon"), QT_TR_NOOP("Computer name"), QT_TR_NOOP("Comment"),
			QT_TR_NOOP("Samba"), QT_TR_NOOP("FTP"), QT_TR_NOOP("Web"), 
			QT_TR_NOOP("News"), QT_TR_NOOP("OS"), QT_TR_NOOP("Gateway"),
			QT_TR_NOOP("Promo"), QT_TR_NOOP("Place"), QT_TR_NOOP("IP"),
			QT_TR_NOOP("Client") };


RzxRezal::RzxRezal(QWidget * parent, const char * name) : Q3ListView(parent, name)
{
	search_patern = QString();
	search_timeout.start();
	timer = NULL;
	selected = NULL;
	
	for(int i = 0; i < numColonnes; i++)
	{
		addColumn(tr(colNames[i]));
		setColumnWidthMode(i, Manual);
		if (i > ColRemarque) setColumnAlignment(i, Qt::AlignCenter);
	}
	setColumnAlignment(ColNom, Qt::AlignCenter);
	setColumnAlignment(ColRemarque, Qt::AlignVCenter);

	setHScrollBarMode(Q3ScrollView::Auto);

	setAllColumnsShowFocus(true);
 
	client = RzxClientListener::global();
	lister = RzxConnectionLister::global();
	
	connect(lister, SIGNAL(login(RzxComputer*)), this, SLOT(bufferedLogin(RzxComputer *)));
	connect(lister, SIGNAL(logout(RzxComputer*)), this, SLOT(logout(RzxComputer*)));
	connect(lister, SIGNAL(update(RzxComputer*)), this, SLOT(bufferedLogin(RzxComputer*)));
	connect(lister, SIGNAL(connectionEtablished()), this, SLOT(init()));

	// On est obligé d'utiliser ce signal pour savoir dans quelle colonne le
	// double-clic suivant a lieu
	connect(this, SIGNAL(pressed(Q3ListViewItem *, const QPoint &, int)),
		this, SLOT(onListClicked(Q3ListViewItem *, const QPoint &, int)));
	connect(this, SIGNAL(doubleClicked(Q3ListViewItem *)), this, SLOT(onListDblClicked(Q3ListViewItem *)));

	lastColumnClicked = ColIcone;
  
	// GESTION DU MENU CONTEXTUEL
	connect(this,SIGNAL(rightButtonPressed(Q3ListViewItem *,const QPoint &,int )),this,SLOT(creePopUpMenu(Q3ListViewItem *,const QPoint &,int )));
	connect(this, SIGNAL(spacePressed(Q3ListViewItem *)), this, SLOT(onListDblClicked(Q3ListViewItem *)));
	connect(this, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(redrawSelectedIcon(Q3ListViewItem*)));
	connect(this, SIGNAL(onItem(Q3ListViewItem*)), this, SLOT(buildToolTip(Q3ListViewItem*)));
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

///Génère le popup contextuel de l'item sélectionné
/** Cette fonction génère et affiche le menu contextuel associé à l'item sélectionné. Il tiens compte :
 * - Des features du client distant
 * - Des choix de l'utilisateur (ban, favoris...)
 */
void RzxRezal::creePopUpMenu(Q3ListViewItem *ordinateurSelect,const QPoint & pos, int)
{
	RzxItem* item = qobject_cast<RzxItem*>((RzxItem*)ordinateurSelect);
	if(item)
		new RzxRezalPopup(item->getComputer(), pos, this);
}

void RzxRezal::proprietes(const RzxHostAddress& peer)
{
	RzxChat *object = lister->getChatByIP(peer);
	RzxComputer *computer = lister->getComputerByIP(peer);
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
	QString hostname = lister->getComputerByIP(item->ip)->name();
	if(!RzxChatSocket::showHistorique(item->ip, hostname))
		emit status(tr("No history file for user %1").arg(hostname), false);
}

void RzxRezal::removeFromFavorites()
{
	RzxItem *item = (RzxItem*)currentItem();
	RzxConfig::global()->delFromFavorites(item->text(ColNom));
	RzxConfig::global()->writeFavorites();
	emit favoriteRemoved(item->getComputer());
}

void RzxRezal::addToFavorites()
{
	RzxItem *item = (RzxItem*)currentItem();
	RzxConfig::global()->addToFavorites(item->text(ColNom));
	RzxConfig::global()->writeFavorites();
	emit favoriteAdded(item->getComputer());
}

void RzxRezal::removeFromIgnoreList()
{
	RzxItem *item = (RzxItem*)currentItem();
	item -> ignored = false;
	RzxConfig::global()->delFromBanlist(*(item->getComputer()));
	RzxConfig::global()->writeIgnoreList();
}

void RzxRezal::addToIgnoreList()
{
	RzxItem *item = (RzxItem*)currentItem();
	item -> ignored = true;
	QString temp= item->text(ColNom);
	RzxConfig::global()->addToBanlist(*(item->getComputer()));
	RzxConfig::global()->writeIgnoreList();
}

///Lancement du client ftp
void RzxRezal::ftp()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::global()->ftp(temp);
}

///Lancement du client http
void RzxRezal::http()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::global()->http(temp);
}

///Lancement du client news
void RzxRezal::news()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::global()->news(temp);
}

///Lancement du client samba
void RzxRezal::samba()
{
	QString temp=currentItem()->text(ColNom);
	RzxUtilsLauncher::global()->samba(temp);
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
					setColumnWidthMode(i,Q3ListView::Maximum);
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
	RzxItem *item = itemByIp[computer->ip()];
	if(!item)
	{
		if(!dispNotFavorites && !RzxConfig::global()->isFavorite(*computer))
			return;
		setUpdatesEnabled(!dispNotFavorites);
		item = new RzxItem(computer, this, dispNotFavorites);
		itemByIp.insert(computer->ip(), item);
		item->setVisible(!dispNotFavorites);
		if(!dispNotFavorites)
			emit newFavorite(computer);
		connect(computer, SIGNAL(isUpdated()), item, SLOT(update()));
	}
	else
	{
		if(!dispNotFavorites)
		{
			if(!RzxConfig::global()->isFavorite(*computer))
			{
				RzxConfig::global()->addToFavorites(*computer);
				RzxConfig::global()->writeFavorites();
			}
			emit changeFavorite(computer);
		}
		QString old_name = nameByIP[computer->ip()];
		if(!old_name.isNull())
		{
			itemByName.remove(old_name);
			nameByIP.remove(computer->ip());
		}
	}

	itemByName.insert(computer->name().lower(), new RzxItem*(item));
	nameByIP.insert(computer->ip(), computer->name().lower());

	item->update();
	setUpdatesEnabled(TRUE);
}

void RzxRezal::logBufLogins() { //en fait vu que le QPtrList faisait des segfaults, je trace toute la listview, c pas très long
	Q3ListViewItem *item;
	Q3ListViewItemIterator it(this);
	while((item=(it++).current())!=NULL)
	{
		if(!item->isVisible())
			item->setVisible(!filterOn || !item->text(ColNom).find(filter, 0, false));
	}
}

/** Déconnexion à partir d'un RzxComputer* */
void RzxRezal::logout(RzxComputer *computer)
{
	RzxItem *item = itemByIp.take(computer->ip());
	
	if(!item) return;
	
	if(!dispNotFavorites)
		emit lostFavorite(computer);
	
	if((selected == item))
		selected = NULL;
	
	itemByName.remove(computer->name().lower());
	nameByIP.remove(computer->ip());
	item->deleteLater();
}

/** On vide la liste des gens connectés */
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
	Q3ListView::clear();
}

/** Réinitialisation de la liste des personnes connectées */
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
	if(login.isNull())
		m_login = currentItem()->text(ColNom);
	return lister->chatCreate(m_login);
}


/** CRASSOU AU POSSIBLE
* TODO: gerer ca autrement */
void RzxRezal::onListDblClicked(Q3ListViewItem * sender){
	RzxItem * item = static_cast<RzxItem*> (sender);
	if(item->ignored) //ignored user, donc on ne va pas communiquer avec lui, faut savoir ce qu'on veut hein
		return;
	
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
		if(RzxConfig::global()->doubleClicRole() && (serveurs & RzxComputer::SERVER_FTP)){
			ftp();
		}
		else {/*chat*/
			RzxItem * listItem = (RzxItem *) sender;
			RzxComputer *computer = listItem->getComputer();
			if(computer->name() != RzxConfig::localHost()->name() && !computer->repondeur() && computer->can(RzxComputer::CAP_CHAT))
				lister->chatCreate(listItem -> ip);
		}
	}
}

/** Memorise la colonne cliquee */
void RzxRezal::onListClicked( Q3ListViewItem *, const QPoint &, int c ){
	lastColumnClicked = (NumColonne) c;
}

void RzxRezal::resizeEvent(QResizeEvent * e) {
	Q3ListView::resizeEvent(e);
	adapteColonnes();
}

void RzxRezal::keyPressEvent(QKeyEvent *e) {
	QString s=e->text();

#ifndef WIN32
	QChar c;
	if(e->key()!=Qt::Key_Backspace)
	{
	  if(s.isEmpty())
		return;
	  c=s.at(0);
	}
	if((e->key()!=Qt::Key_Backspace)&&(!c.isLetter())) {
#else
	QChar c;
	if(!s.isEmpty())
		c = s.at(0);
	if((e->key()!=Qt::Key_Backspace) && (s.isNull() == TRUE || !c.isLetter())) {
#endif
		if(e->key()==Qt::Key_Right) {
			QRect r(itemRect( currentItem())); //r != 0 car the currentItem IS visible
			QPoint qp=r.center();
			qp.setY(qp.y()+r.height());
			creePopUpMenu(currentItem(), qp,0);
			return;
		}
		search_patern = QString();
		emit set_search(tr("%1").arg(search_patern));
		
		Q3ListView::keyPressEvent(e); //on laisse Qt gérer
		return;
	}

	if(search_timeout.elapsed()>5000) // On a attendu trop longtemps depuis la dernière pression
		search_patern = QString();
	// On continue à augmenter le patern
	if(e->key()==Qt::Key_Backspace)
		if(search_patern.length())
			search_patern=search_patern.left(search_patern.length()-1);
		else
		{
			emit set_search(tr("%1").arg(search_patern));
			return;
		}
	else
		search_patern += c.lower();
	search_timeout.start();

	RzxItem **item;
	QString lower, higher;
	if(!itemByName.find_nearest(search_patern,lower,higher))
	{
		bool lmatch, hmatch;
		lmatch = lower.left(search_patern.length())==search_patern;
		hmatch = higher.left(search_patern.length())==search_patern;
		if((!lmatch) && (!hmatch))
		{
			int i;
			for(i=0, lmatch = true, hmatch=true;  lmatch && hmatch && (i<search_patern.length()); i++)
			     lmatch = lower.at(i)==search_patern.at(i),
			     hmatch = higher.at(i)==search_patern.at(i);
			i--;
			if((!hmatch) && (!lmatch))
			{
				char c  = search_patern.at(i).latin1(),
				     lc = lower.at(i).latin1(),
				     hc = higher.at(i).latin1();
				if(qAbs(c-hc)<qAbs(c-lc))
					hmatch=true;
			}
			search_patern = search_patern.left(search_patern.length()-1);
		}
		if(hmatch && (!lmatch))
			lower = higher;
	}
	emit set_search(tr("%1").arg(search_patern));

	itemByName.find(lower,item);

	setCurrentItem(*item);
	ensureItemVisible(*item);
	return;
}

/** No descriptions */
void RzxRezal::redrawAllIcons(){
	setColumnWidth(0, 64);
	for (Q3ListViewItemIterator it(this) ; it.current() ; ++it) {
		RzxItem *item = qobject_cast<RzxItem*>((RzxItem*)it.current());
		item->update();
	}
}

///Mise à jour de la taille de l'icône sélectionnée
void RzxRezal::redrawSelectedIcon(Q3ListViewItem *sel)
{
	if(selected) selected->update();
	selected = (RzxItem*)sel;
	if(selected) selected->update();
}

///Mise à jour du tooltip de la fenêtre
/** Le tooltip permet d'avoir des informations sur la personne, ça peut être pratique pour avoir en particulier une vue réduite mais en conservant l'accès faciles aux données ftp, http, promo... */
void RzxRezal::buildToolTip(Q3ListViewItem *i) const
{
	int tooltipFlags = RzxConfig::tooltip();
	if(!i || !(tooltipFlags & (int)RzxConfig::Enable) || tooltipFlags==(int)RzxConfig::Enable) return;
	
	RzxItem *item = qobject_cast<RzxItem*>((RzxItem*)i);
	if(!item) return;
	
	RzxComputer *computer = item->getComputer();
	viewport()->setToolTip(computer->tooltipText());
}

void RzxRezal::languageChanged(){
	for (int i = 0; i < numColonnes; i++) {
		setColumnText(i, tr(colNames[i]));
	}
}


/***************************************************************************
                          qrezix.cpp  -  description
                             -------------------
    begin                : lun jan 28 16:27:20 CET 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qtoolbox.h>
#include <qpopupmenu.h>
#include <qrect.h>
#include <qpoint.h>
#include <qstring.h>
#include <qfile.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qlayout.h> 

#include "qrezix.h"
#include "rzxquit.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxproperty.h"
#include "rzxpluginloader.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"
#include "trayicon.h"

#include "defaults.h"

#include "t.xpm"
#include "q.xpm"

QRezix *QRezix::object = 0;

QRezix::QRezix(QWidget *parent, const char *name)
 : QRezixUI(parent, name), m_properties(0), tray(0)
{
	object = this;
	byTray = false;
	statusFlag = false;
	wellInit = FALSE;
	
	///Chargement des plug-ins
	RzxPlugInLoader::global();
	///Préparation du lanceur des clients http...
	new RzxUtilsLauncher(rezal);
	
	rezal->showNotFavorites(true);
	rezalFavorites->showNotFavorites(false);
	
	connect(btnPreferences, SIGNAL(clicked()), this, SLOT(boitePreferences()));
	connect(btnMAJcolonnes, SIGNAL(clicked()), rezal, SLOT(afficheColonnes()));
	connect(btnMAJcolonnes, SIGNAL(clicked()), rezalFavorites, SLOT(afficheColonnes()));
	connect(btnAutoResponder, SIGNAL(toggled(bool)), this, SLOT(activateAutoResponder(bool)));
	connect(btnPlugins, SIGNAL(toggled(bool)), this, SLOT(pluginsMenu(bool)));
	connect(btnSearch, SIGNAL(toggled(bool)), rezal, SLOT(activeFilter(bool)));
	connect(btnSearch, SIGNAL(toggled(bool)), rezalFavorites, SLOT(activeFilter(bool)));
	connect(leSearch, SIGNAL(returnPressed()), this, SLOT(launchSearch()));
	connect(&menuPlugins, SIGNAL(aboutToHide()), btnPlugins, SLOT(toggle()));
 
	connect(RzxClientListener::object(), SIGNAL(chatSent()), this, SLOT(chatSent()));

	// Préparation de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	connect(rezal, SIGNAL(favoriteRemoved(RzxComputer*)), rezalFavorites, SLOT(logout(RzxComputer*)));
	connect(rezalFavorites, SIGNAL(favoriteRemoved(RzxComputer*)), rezalFavorites, SLOT(logout(RzxComputer*)));
	connect(rezal, SIGNAL(favoriteAdded(RzxComputer*)), rezalFavorites, SLOT(login(RzxComputer*)));
	
	clearWFlags(WStyle_SysMenu|WStyle_Minimize);
	alreadyOpened=false;
	
	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(lister, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));


	m_properties = new RzxProperty(this);
	if(!RzxConfig::globalConfig()->find() || !m_properties->infoCompleted())
	{
		m_properties->initDlg();
		m_properties -> exec();
	}
	if(!m_properties->infoCompleted()) { wellInit = FALSE; return;}
	
	//RzxConfig::loadTranslators();
	lister -> initConnection();

	connect(rezal, SIGNAL(selectionChanged(QListViewItem*)), RzxPlugInLoader::global(), SLOT(itemChanged(QListViewItem*)));

	connect(RzxConfig::globalConfig(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));
	menuFormatChange();

	tbRezalContainer -> setCurrentIndex(RzxConfig::defaultTab());
	leSearch -> setShown(RzxConfig::globalConfig()->useSearch());
	btnSearch -> setShown(RzxConfig::globalConfig()->useSearch());

	changeTheme();
	wellInit = TRUE; 
}

void QRezix::launchPlugins()
{
	RzxPlugInLoader::global()->init();
}

QRezix *QRezix::global()
{
	return object;
}

void QRezix::languageChanged(){
	qDebug("Language changed");
	languageChange();
	rezal->languageChanged();
	rezalFavorites->languageChanged();
}

QRezix::~QRezix() {
}

void QRezix::status(const QString& msg, bool fatal){
	lblStatus -> setText(msg);
	statusFlag = !fatal;

	changeTheme();
	if(statusFlag)
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("off"));
		
	/* parceque je veux avoir une trace de ce qui s'est passé ! */
	qDebug( "[%s] status%s = %s", QDateTime::currentDateTime().toString().latin1(),
		    fatal ? " (FATAL)" : "", msg.latin1() );
}

void QRezix::closeByTray()
{
	byTray = true;
	close();
}

///Sauvegarde des données au moment de la fermeture
/** Lance la sauvegarde des données principales lors de la fermeture de rezix. Cette méthode est censée permettre l'enregistrement des données lors de la fermeture de l'environnement graphique... */
void QRezix::saveSettings()
{
	byTray = true;
	QSize s = size();       // store size
	QString height="";
	height.sprintf("%4d",s.height());
	QString width = "";
	width.sprintf("%4d",s.width());
	QString windowSize;
	if( statusMax ) windowSize = "100000000";
	else windowSize="0"+height+width;
	qDebug("Fermeture des plugins");
	delete RzxPlugInLoader::global();
	qDebug("Fermeture de l'enregistrement des configurations");
	RzxConfig::globalConfig()->writeWindowSize(windowSize);
	RzxConfig::globalConfig() -> closeSettings();
	qDebug("Fermeture des fenêtres de chat");
	RzxConnectionLister::global() ->closeChats();
	qDebug("Fermeture de l'écoute réseau");
	RzxClientListener::object()->close();
	qDebug("Fermeture de la connexion avec le serveur");
	if (!RzxConnectionLister::global() -> isSocketClosed()){
		lblStatus -> setText(tr("Closing socket..."));
		rezal-> setEnabled(false);
		rezalFavorites->setEnabled(false);
		connect(RzxConnectionLister::global(), SIGNAL(socketClosed()), this, SLOT(socketClosed()));
		RzxConnectionLister::global() -> closeSocket();
	}
	disconnect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
	qDebug("Fermeture de qRezix terminée");
}

void QRezix::closeEvent(QCloseEvent * e){
	//pour éviter de fermer rezix par mégarde, on affiche un boite de dialogue laissant le choix
	//de fermer qrezix, de minimiser la fenêtre principale --> trayicon, ou d'annuler l'action
	if(!byTray && isShown() && !isMinimized())
	{
		int i;
		if(RzxConfig::showQuit())
		{
			RzxQuit quitDialog(this);
			i = quitDialog.exec();
		}
		else
			i = RzxConfig::quitMode();
		if(i!=RzxQuit::selectQuit)
		{
			if(i==RzxQuit::selectMinimize)
				showMinimized();
#ifdef WIN32 //c'est très très très très très très moche, mais g pas trouvé d'autre manière de le faire
			 //c'est pas ma fautre à moi si windows se comporte comme de la merde
			QEvent ev(QEvent::WindowDeactivate); 
			event(&ev);
#endif
			e -> ignore();
			return;
		}
	}

	saveSettings();

	if (RzxConnectionLister::global()-> isSocketClosed())
		e -> accept();
	else
		e -> ignore();
}

bool QRezix::event(QEvent * e){
#ifdef WIN32
	if(e->type()==QEvent::WindowDeactivate)
	{
		if(isMinimized() && RzxConfig::globalConfig()->useSystray())
			hide();
		return true;
	}
#else //WIN32
	if(e->type()==QEvent::ShowMinimized || e->type()==QEvent::Hide){
		if(RzxConfig::globalConfig()->useSystray()) hide();
		return true;
	}
#endif //WIN32
	else if( e->type() == QEvent::Resize && alreadyOpened && !isMinimized())
		statusMax = isMaximized();

	return QWidget::event(e);
}

void QRezix::delayedInit() {
	disconnect(m_properties, SIGNAL(end()), this, SLOT(delayedInit()));
	RzxConnectionLister::global()  -> initConnection();
}

void QRezix::boitePreferences(){
	if (m_properties) {
		if (!(m_properties -> isVisible())) {
			m_properties -> initDlg();
			m_properties -> show();
		}
	}
	else {
		m_properties = new RzxProperty(this);
		m_properties -> show();
	}
}

void QRezix::socketClosed(){
	close();
}


void QRezix::toggleAutoResponder()
{
	activateAutoResponder( !btnAutoResponder->isOn());
}

void QRezix::toggleButtonResponder()
{
	activateAutoResponder( !btnAutoResponder -> isOn() );
}

void QRezix::activateAutoResponder( bool state )
{
	if(!state == btnAutoResponder->isOn())
	{
		btnAutoResponder->setOn(state);
		return;
	}
	if (state == (RzxConfig::autoResponder() != 0)) return;
	RzxConfig::setAutoResponder( state );
	RzxProperty::serverUpdate();
	RzxPlugInLoader::global()->sendQuery(RzxPlugIn::DATA_AWAY, NULL);
	changeTrayIcon();
}

///Lancement d'une recherche sur le pseudo dans la liste des personnes connectées
void QRezix::launchSearch()
{
	rezal->setFilter(leSearch->text());
	rezalFavorites->setFilter(leSearch->text());
	if(!leSearch->text().length()) btnSearch->setOn(false);
	else btnSearch->setOn(true);
}

void QRezix::changeTrayIcon(){
	// Change l'icone dans la tray
	QPixmap trayIcon;
	if(!RzxConfig::autoResponder())
	{
		trayIcon = *(RzxConfig::themedIcon("systray"));
		if(trayIcon.isNull())
			trayIcon = QPixmap::QPixmap(q);

	}
	else
	{
		trayIcon = *(RzxConfig::themedIcon("systrayAway"));
		if(trayIcon.isNull())
			trayIcon = QPixmap::QPixmap(t);
	}
	if(tray) tray->setIcon(trayIcon);
}

void QRezix::toggleVisible(){
	if(isVisible())
	{
		bool dispProp = (m_properties && m_properties->isVisible());
		hide();
		if(dispProp) m_properties->show();
	}
	else{
		bool saveStatusMax = statusMax;
		showNormal();	//pour forcer l'affichage de la fenêtre ==> modifie statusMax
		if(saveStatusMax)
		{
			showMaximized();
			statusMax = saveStatusMax;
		}
		show();
		setActiveWindow();
		raise();
		alreadyOpened=true;
		rezal->afficheColonnes();
		rezalFavorites->afficheColonnes();
	}
}

void QRezix::languageChange()
{
	QRezixUI::languageChange();
	setCaption(caption() + " v" + RZX_VERSION);
#ifdef WIN32
	setCaption(caption() + " [Qt]");
#endif

	//Parce que Qt oublie de traduire les deux noms
	//Alors faut le faire à la main, mais franchement, c du foutage de gueule
	//à mon avis ça leur prendrait 5 minutes chez trolltech pour corriger le pb
	tbRezalContainer->setItemLabel(0, tr("Favorites"));
	tbRezalContainer->setItemLabel(1, tr("Everybody"));
}

void QRezix::chatSent() {
	// Desactive le répondeur lorsqu'on envoie un chat
	activateAutoResponder( false );
}

/// Changement du thème d'icone
/** Cette méthode met à jour les icônes de l'interface principale (menu), mais aussi celles des listes de connectés */
void QRezix::changeTheme()
{
	rezal -> redrawAllIcons();
	rezalFavorites -> redrawAllIcons();
	QIconSet pi, away, columns, prefs,favorite,not_favorite, search;
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();
	if(icons || !texts)
	{
		pi.setPixmap(*RzxConfig::themedIcon("plugin"), QIconSet::Automatic);
		away.setPixmap(*RzxConfig::themedIcon("away"), QIconSet::Automatic, QIconSet::Normal, QIconSet::Off);
		away.setPixmap(*RzxConfig::themedIcon("here"), QIconSet::Automatic, QIconSet::Normal, QIconSet::On);
		columns.setPixmap(*RzxConfig::themedIcon("column"), QIconSet::Automatic);
		prefs.setPixmap(*RzxConfig::themedIcon("pref"), QIconSet::Automatic);
		favorite.setPixmap(*RzxConfig::themedIcon("favorite"), QIconSet::Automatic);
		not_favorite.setPixmap(*RzxConfig::themedIcon("not_favorite"), QIconSet::Automatic);
		search.setPixmap(*RzxConfig::themedIcon("search"), QIconSet::Automatic);
	}
	btnPlugins->setIconSet(pi);
	btnAutoResponder->setIconSet(away);
	btnMAJcolonnes->setIconSet(columns);
	btnPreferences->setIconSet(prefs);
	btnSearch->setIconSet(search);
	tbRezalContainer->setItemIconSet(1,not_favorite);
	tbRezalContainer->setItemIconSet(0,favorite);
	tbRezalContainer->setItemIconSet(2,search);
	if(statusFlag)
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(*RzxConfig::themedIcon("off"));
	if(wellInit) changeTrayIcon();
}

///Changement de format des boutons de la barre d'outils
/** Change la position du texte, ou la taille des icônes de la barre d'outil */
void QRezix::menuFormatChange()
{
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();

	//On transforme le cas 'pas d'icônes et pas de texte' en 'petites icônes et pas de texte'
	if(!texts && !icons) icons = 1;

	//Si on a pas d'icône, on met le texte sur le côté... pour éviter un bug d'affichage
	if(!icons) texts = 1;
	
	//Mise à jour de la taille des icônes
	switch(icons)
	{
		case 0: //pas d'icône
			{
				QIconSet empty;
				QPixmap emptyIcon;
				btnPlugins->setIconSet(empty);
				btnAutoResponder->setIconSet(empty);
				btnMAJcolonnes->setIconSet(empty);
				btnPreferences->setIconSet(empty);
				btnSearch->setIconSet(empty);
				tbRezalContainer->setItemIconSet(0,empty);
				tbRezalContainer->setItemIconSet(1,empty);
				tbRezalContainer->setItemIconSet(2,empty);
				lblStatusIcon->setHidden(TRUE);
			}
			break;
		
		case 1: //petites icônes
		case 2: //grandes icones
			{
				bool big = (icons == 2);
				if(btnPlugins->iconSet().isNull()) changeTheme(); //pour recharcher les icônes s'il y a besoin
				btnPlugins->setUsesBigPixmap(big);
				btnAutoResponder->setUsesBigPixmap(big);
				btnMAJcolonnes->setUsesBigPixmap(big);
				btnPreferences->setUsesBigPixmap(big);
				btnSearch->setUsesBigPixmap(big);
				lblStatusIcon->setShown(TRUE);
			}
			break;
	}
	
	//Mise à jour de la position du texte
	btnPlugins->setUsesTextLabel(texts);
	btnAutoResponder->setUsesTextLabel(texts);
	btnMAJcolonnes->setUsesTextLabel(texts);
	btnPreferences->setUsesTextLabel(texts);
	btnSearch->setUsesTextLabel(texts);
	if(texts)
	{
		QToolButton::TextPosition pos = (texts == 1)? QToolButton::BesideIcon : QToolButton::BelowIcon;
		btnPlugins->setTextPosition(pos);
		btnAutoResponder->setTextPosition(pos);
		btnMAJcolonnes->setTextPosition(pos);
		btnPreferences->setTextPosition(pos);
		btnSearch->setTextPosition(pos);
		lblStatus->setShown(TRUE);
		spacerStatus->changeSize(1,1,QSizePolicy::Minimum,QSizePolicy::Minimum);
	}
	else
	{
		lblStatus->setShown(FALSE);
		spacerStatus->changeSize(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
	}
}

/// Affichage du menu plug-ins lors d'un clic sur le bouton
/** Les actions sont gérées directement par le plug-in s'il a bien été programmé */
void QRezix::pluginsMenu(bool show)
{
	if(!show)
		return;
	
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuAction(menuPlugins);
	if(!menuPlugins.count())
		menuPlugins.insertItem("<none>");
	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}

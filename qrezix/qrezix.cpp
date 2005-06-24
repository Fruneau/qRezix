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
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QToolBox>
#include <QMenuBar>
#include <QMenu>
#include <QRect>
#include <QPoint>
#include <QString>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QApplication>
#include <QProcess>
#include <QSound>
#include <QCloseEvent>
#include <QEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QToolBar>
#include <QAction>

#include "qrezix.h"

#include "rzxquit.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxproperty.h"
#include "rzxpluginloader.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"
#include "rzxtraywindow.h"
#include "rzxcomputer.h"
#include "trayicon.h"

#include "defaults.h"

#include "t.xpm"
#ifdef Q_OS_MAC
#include "q_mac.xpm"
#else
#include "q.xpm"
#endif

QRezix *QRezix::object = 0;

QRezix::QRezix(QWidget *parent)
 : QMainWindow(parent, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowContextHelpButtonHint),
	m_properties(0), accel(0), tray(0)
{
	setupUi(this);
	object = this;
	byTray = false;
	statusFlag = false;
	favoriteWarn = true;
	wellInit = false;
	alreadyOpened=false;

	
	///Chargement de la config
	RzxConfig::globalConfig();
	///Chargement des plug-ins
	RzxPlugInLoader::global();
	///Préparation du lanceur des clients http...
	new RzxUtilsLauncher(rezal);
	buildActions();
	
#ifdef Q_OS_MAC
	QMenuBar *menu = new QMenuBar();
	QMenu *popup = menu->addMenu("Tools");
//	popup->addAction(prefAction); //Problème de traduction
	popup->addAction("Preferences", this, SLOT(boitePreferences()));
#endif

	//Construction de la ligne d'édtion des recherche
	leSearch = new QLineEdit();
	leSearch->setMinimumSize(50, 22);
	leSearch->setMaximumSize(150, 22);
	connect(leSearch, SIGNAL(returnPressed()), this, SLOT(launchSearch()));
	connect(leSearch, SIGNAL(textChanged(const QString&)), rezal, SLOT(setFilter(const QString&)));
	connect(leSearch, SIGNAL(textChanged(const QString&)), rezalFavorites, SLOT(setFilter(const QString&)));
	
	//Construction des barres d'outils
	QToolBar *bar = addToolBar("Main");
	bar->addAction(pluginsAction);
	bar->addSeparator();
	bar->addWidget(leSearch);
	bar->addAction(searchAction);
	bar->setMovable(false);

	QLabel *spacer = new QLabel(); //CRAAAAAAAAAAAAAAAAAAAADE
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	bar->addWidget(spacer);
	bar->addAction(awayAction);
	bar->addAction(columnsAction);
	bar->addAction(prefAction);
	bar->setMovable(false);
	
	rezal->showNotFavorites(true);
	rezalFavorites->showNotFavorites(false);
	
 
	connect(RzxClientListener::global(), SIGNAL(chatSent()), this, SLOT(chatSent()));
	
	// Préparation de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	connect(rezal, SIGNAL(favoriteAdded(RzxComputer*)), this, SLOT(newFavorite()));
	connect(rezalFavorites, SIGNAL(favoriteRemoved(RzxComputer*)), this, SLOT(newFavorite()));
	connect(rezal, SIGNAL(favoriteRemoved(RzxComputer*)), this, SLOT(newFavorite()));
	
	connect(rezal, SIGNAL(favoriteRemoved(RzxComputer*)), rezalFavorites, SLOT(logout(RzxComputer*)));
	connect(rezalFavorites, SIGNAL(favoriteRemoved(RzxComputer*)), rezalFavorites, SLOT(logout(RzxComputer*)));
	connect(rezal, SIGNAL(favoriteAdded(RzxComputer*)), rezalFavorites, SLOT(bufferedLogin(RzxComputer*)));
	
	connect(rezal, SIGNAL(set_search(const QString&)), leSearch, SLOT(setText(const QString &)));
	connect(rezalFavorites, SIGNAL(set_search(const QString&)), leSearch, SLOT(setText(const QString &)));
	
	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(lister, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));
	
	/* Gestion des favoris */
	connect(rezalFavorites, SIGNAL(newFavorite(RzxComputer*)), this, SLOT(warnForFavorite(RzxComputer*)));
	connect(rezalFavorites, SIGNAL(lostFavorite(RzxComputer*)), this, SLOT(warnForDeparture(RzxComputer*)));
	connect(rezalFavorites, SIGNAL(changeFavorite(RzxComputer*)), this, SLOT(warnForFavorite(RzxComputer*)));

	m_properties = new RzxProperty(this);
	if(!RzxConfig::globalConfig()->find() || !m_properties->infoCompleted())
	{
		m_properties->initDlg();
		m_properties -> exec();
	}
	if(!m_properties->infoCompleted()) { wellInit = FALSE; return;}
	
	qDebug("=== qRezix Running ===");
	lister->initConnection();

	connect(rezal, SIGNAL(selectionChanged(Q3ListViewItem*)), RzxPlugInLoader::global(), SLOT(itemChanged(Q3ListViewItem*)));
	connect(RzxConfig::globalConfig(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));

	tbRezalContainer->setCurrentIndex(RzxConfig::defaultTab());
	showSearch(RzxConfig::globalConfig()->useSearch());
	
	//Raccourcis claviers particuliers
	accel = new QShortcut(Qt::Key_Tab + Qt::SHIFT, this, SLOT(switchTab()));
	menuFormatChange();

#ifdef Q_OS_MAC
	rezal->afficheColonnes();
	rezalFavorites->afficheColonnes();
#endif
	wellInit = TRUE;
}

///Construction des actions
/** Les actions définissent les appels de base des menus/barre d'outils */
void QRezix::buildActions()
{
	pluginsAction = new QAction(tr("Plug-ins"), this);
	pluginsAction->setCheckable(true);
	connect(pluginsAction, SIGNAL(toggled(bool)), this, SLOT(pluginsMenu(bool)));
	connect(&menuPlugins, SIGNAL(aboutToHide()), pluginsAction, SLOT(toggle()));

	prefAction = new QAction(tr("Preferences"), this);
	connect(prefAction, SIGNAL(triggered()), this, SLOT(boitePreferences()));
	
	columnsAction = new QAction(tr("Adjust columns"), this);
	connect(columnsAction, SIGNAL(triggered()), rezal, SLOT(afficheColonnes()));
	connect(columnsAction, SIGNAL(triggered()), rezalFavorites, SLOT(afficheColonnes()));
	
	searchAction = new QAction(tr("Search"), this);
	searchAction->setCheckable(true);
	connect(searchAction, SIGNAL(toggled(bool)), rezal, SLOT(activeFilter(bool)));
	connect(searchAction, SIGNAL(toggled(bool)), rezalFavorites, SLOT(activeFilter(bool)));

	awayAction = new QAction(tr("Away"), this);
	awayAction->setCheckable(true);	
	connect(awayAction, SIGNAL(toggled(bool)), this, SLOT(activateAutoResponder(bool)));
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
	delete RzxUtilsLauncher::global();
	qDebug("Bye Bye\n");
}

void QRezix::status(const QString& msg, bool fatal){
	lblStatus -> setText(msg);
	statusFlag = !fatal;

	if(statusFlag)
		lblStatusIcon->setPixmap(RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(RzxConfig::themedIcon("off"));
		
	qDebug("Connection status : " + QString(fatal?"disconnected ":"connected ") + "(" + msg + ")");
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
	qDebug("\n=== qRezix stopping ===");
	byTray = true;
	QSize s = size();       // store size
	
	//Pas très beau, mais c'est juste pour voir si ça améliore le rétablissement de l'état de la fenêtre sur mac
	QString height = QString("%1").arg(s.height(), 4, 10).replace(' ', '0');
	QString width =  QString("%1").arg(s.width(), 4, 10).replace(' ', '0');
	QString windowSize;
#ifndef Q_OS_MAC
	if( statusMax )
		windowSize = "100000000";
	else
#endif
		windowSize="0"+height+width;
	qDebug("Fermeture des plugins");
	delete RzxPlugInLoader::global();
	qDebug("Fermeture de l'enregistrement des configurations");
	RzxConfig::globalConfig()->writeWindowSize(windowSize);
	RzxConfig::globalConfig()->writeWindowPosition(pos());
	RzxConfig::globalConfig() -> closeSettings();
	qDebug("Fermeture des fenêtres de chat");
	RzxConnectionLister::global() ->closeChats();
	qDebug("Fermeture de l'écoute réseau");
	RzxClientListener::global()->close();
	qDebug("Fermeture de la connexion avec le serveur");
	if (!RzxConnectionLister::global() -> isSocketClosed()){
		lblStatus -> setText(tr("Closing socket..."));
		rezal-> setEnabled(false);
		rezalFavorites->setEnabled(false);
		connect(RzxConnectionLister::global(), SIGNAL(socketClosed()), this, SLOT(socketClosed()));
		RzxConnectionLister::global() -> closeSocket();
		RzxConnectionLister::global() -> deleteLater();
	}
	disconnect(qApp, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
	qDebug("Fermeture de l'interface");
	if(m_properties)
	{
		delete m_properties;
		m_properties = NULL;
	}
	if(accel)
	{
		delete accel;
		accel = NULL;
	}
	if(tray)
	{
		delete tray;
		tray = NULL;
	}
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
	if(e->type()==QEvent::WindowDeactivate)
	{
		if(isMinimized() && RzxConfig::globalConfig()->useSystray())
			hide();
		return true;
	}
	else if( e->type() == QEvent::Resize && alreadyOpened && !isMinimized())
		statusMax = isMaximized();

	return QWidget::event(e);
}

void QRezix::switchTab()
{
	tbRezalContainer->setCurrentIndex(1 - tbRezalContainer->currentIndex());
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
	activateAutoResponder( !awayAction->isChecked());
}

void QRezix::toggleButtonResponder()
{
	activateAutoResponder( !awayAction->isChecked());
}

void QRezix::activateAutoResponder( bool state )
{
	if(!state == awayAction->isChecked())
	{
		awayAction->setChecked(state);
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
	if(searchAction->isChecked())
	{
		searchAction->setChecked(false);
		return;
	}
	if(!leSearch->text().length()) searchAction->setChecked(false);
	else searchAction->setChecked(true);
}

void QRezix::changeTrayIcon(){
	// Change l'icone dans la tray
	QPixmap trayIcon;
	if(!RzxConfig::autoResponder())
	{
		trayIcon = RzxConfig::themedIcon("systray");
		if(trayIcon.isNull())
			trayIcon = QPixmap::QPixmap(q);

	}
	else
	{
		trayIcon = RzxConfig::themedIcon("systrayAway");
		if(trayIcon.isNull())
			trayIcon = QPixmap::QPixmap(t);
	}
#ifdef Q_WS_MAC
	tray->buildMenu();
#endif
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

///Montre/cache la parte de recherche
void QRezix::showSearch(bool state)
{
	leSearch->setVisible(state);
	searchAction->setVisible(state);
}

void QRezix::languageChange()
{
	QWidget::languageChange();
	setWindowTitle(windowTitle() + " v" + RZX_VERSION);

	//Parce que Qt oublie de traduire les deux noms
	//Alors faut le faire à la main, mais franchement, c du foutage de gueule
	//à mon avis ça leur prendrait 5 minutes chez trolltech pour corriger le pb
	tbRezalContainer->setItemText(0, tr("Favorites"));
	tbRezalContainer->setItemText(1, tr("Everybody"));
	
	//Parce que sous Mac, les boutons sont gérés avec des QPushButton
#ifdef Q_OS_MAC
	menuFormatChange();
#endif
}

void QRezix::chatSent() {
	// Desactive le répondeur lorsqu'on envoie un chat
	activateAutoResponder( false );
}

/// Pour l'affichage d'une notification lors de la connexion d'un favoris
/** Permet d'alerter lors de l'arrivée d'un favoris, que ce soit par l'émission d'un son, ou par l'affichage l'affichage d'un message indiquant la présence de ce favoris */
void QRezix::warnForFavorite(RzxComputer *computer)
{
	//ne garde que les favoris avec en plus comme condition que ce ne soit pas les gens présents à la connexion
	//evite de notifier la présence de favoris si en fait c'est nous qui arrivons.
	if(!RzxConnectionLister::global()->isInitialized() || !favoriteWarn || computer->getName() == RzxConfig::localHost()->getName())
	{
		favoriteWarn = true;
		return;
	}
		
	//Bah, beep à la connexion
	if(RzxConfig::beepConnection() && computer->getRepondeur()) {

#if defined (WIN32) || defined (Q_OS_MAC)
		QString file = RzxConfig::connectionSound();
		if( !file.isEmpty() && QFile(file).exists() )
			QSound::play( file );
        	else
			QApplication::beep();
#else
		QString cmd = RzxConfig::beepCmd(), file = RzxConfig::connectionSound();
		if (!cmd.isEmpty() && !file.isEmpty()) {
			QProcess process;
			process.start(cmd, QStringList(file));
		}
#endif
	}
	
	//Affichage de la fenêtre de notification de connexion
	if(RzxConfig::showConnection())
		new RzxTrayWindow(computer);
}

/// Pour la notification de la déconnexion d'un favoris
void QRezix::warnForDeparture(RzxComputer *computer)
{
	//Affichage de la fenêtre de notification de déconnexion
	if(RzxConfig::showConnection() && favoriteWarn)
		new RzxTrayWindow(computer, false);
	favoriteWarn = true;
}

/// Pour se souvenir que la prochaine connexion sera celle d'un nouveau favoris
/** Permet d'éviter la notification d'arrivée d'un favoris, lorsqu'une personne change uniquement de statut non-favoris -> favoris */
void QRezix::newFavorite()
{
	favoriteWarn = false;
}

/// Changement du thème d'icone
/** Cette méthode met à jour les icônes de l'interface principale (menu), mais aussi celles des listes de connectés */
void QRezix::changeTheme()
{
	qDebug("Theme changed");
	rezal -> redrawAllIcons();
	rezalFavorites -> redrawAllIcons();
	QIcon pi, away, columns, prefs,favorite,not_favorite, search;
	int icons = RzxConfig::menuIconSize();
	int texts = RzxConfig::menuTextPosition();
	if(icons || !texts)
	{
		pi.addPixmap(RzxConfig::themedIcon("plugin"));
		away.addPixmap(RzxConfig::themedIcon("away"), QIcon::Normal, QIcon::Off);
		away.addPixmap(RzxConfig::themedIcon("here"), QIcon::Normal, QIcon::On);
		columns.addPixmap(RzxConfig::themedIcon("column"));
		prefs.addPixmap(RzxConfig::themedIcon("pref"));
		favorite.addPixmap(RzxConfig::themedIcon("favorite"));
		not_favorite.addPixmap(RzxConfig::themedIcon("not_favorite"));
		search.addPixmap(RzxConfig::themedIcon("search"));
	}
	pluginsAction->setIcon(pi);
	awayAction->setIcon(away);
	columnsAction->setIcon(columns);
	prefAction->setIcon(prefs);
	searchAction->setIcon(search);
	lblCountIcon->setPixmap(RzxConfig::themedIcon("not_favorite").scaled(16,16));
	tbRezalContainer->setItemIcon(1,not_favorite);
	tbRezalContainer->setItemIcon(0,favorite);
	if(statusFlag)
		lblStatusIcon->setPixmap(RzxConfig::themedIcon("on"));
	else
		lblStatusIcon->setPixmap(RzxConfig::themedIcon("off"));
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
	Qt::ToolButtonStyle style;
	if(icons && !texts) style = Qt::ToolButtonIconOnly;
	else if(!icons && texts) style = Qt::ToolButtonTextOnly;
	else if(icons && texts == 1) style = Qt::ToolButtonTextBesideIcon;
	else if(icons && texts == 2) style = Qt::ToolButtonTextUnderIcon;	
	setToolButtonStyle(style);
	
	//Mise à jour de la taille des icônes
	switch(icons)
	{
		case 0: //pas d'icône
			{
				QIcon empty;
				QPixmap emptyIcon;
				tbRezalContainer->setItemIconSet(0,empty);
				tbRezalContainer->setItemIconSet(1,empty);
				lblStatusIcon->hide();
				lblCountIcon->hide();
			}
			break;
		case 1: //petites icônes
		case 2: //grandes icones
			{
				if(pluginsAction->icon().isNull()) changeTheme();
				int dim = (icons == 2)?32:16;
				QSize size = QSize(dim,dim);
				setIconSize(size);
				lblStatusIcon->show();
				lblCountIcon->show();
			}
			break;
	}
	
	//Mise à jour de la position du texte
	if(texts)
	{
		lblStatus->show();
		lblCountIcon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}
	else
	{
		lblStatus->hide();
		lblCountIcon->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
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
		menuPlugins.addAction("<none>");
//	menuPlugins.popup();//btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}

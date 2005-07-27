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

#include "rzxglobal.h"

#include "rzxquit.h"
#include "rzxconfig.h"
#include "rzxiconcollection.h"
#include "rzxproperty.h"
#include "rzxpluginloader.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"
#include "rzxtraywindow.h"
#include "rzxcomputer.h"
#include "rzxtrayicon.h"
#include "rzxrezalmodel.h"
#include "rzxrezalview.h"

#include QREZIX_ICON
#include QREZIX_AWAY_ICON

QRezix *QRezix::object = NULL;

QRezix::QRezix(QWidget *parent)
 : QMainWindow(parent, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowContextHelpButtonHint | Qt::WindowContextHelpButtonHint),
	m_properties(0), accel(0), tray(0)
{
	hereIcon = QPixmap(q);
	awayIcon = QPixmap(t);
	
	object = this;
	setupUi(this);
	byTray = false;
	statusFlag = false;
	favoriteWarn = true;
	wellInit = false;
	alreadyOpened=false;

	///Chargement de la config
	RzxConfig::global();
	///Chargement des plug-ins
	RzxPlugInLoader::global();
	buildActions();
	
#ifdef Q_OS_MAC
	QMenuBar *menu = new QMenuBar();
	QMenu *popup = menu->addMenu("Tools");
	popup->addAction("Preferences", this, SLOT(boitePreferences()));
#endif

	//Construction de la ligne d'édtion des recherche
	leSearch = new QLineEdit();
	leSearch->setMinimumSize(50, 22);
	leSearch->setMaximumSize(150, 22);
	connect(leSearch, SIGNAL(returnPressed()), this, SLOT(launchSearch()));
//	connect(leSearch, SIGNAL(textChanged(const QString&)), rezal, SLOT(setFilter(const QString&)));
//	connect(leSearch, SIGNAL(textChanged(const QString&)), rezalFavorites, SLOT(setFilter(const QString&)));
	
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
	
	rezal->setRootIndex(RzxRezalModel::global()->everybodyGroup);
	rezalFavorites->setRootIndex(RzxRezalModel::global()->favoriteIndex);

	connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), this, SLOT(changeTheme()));
	connect(RzxClientListener::global(), SIGNAL(chatSent()), this, SLOT(chatSent()));
	
	// Préparation de l'insterface
	activateAutoResponder( RzxConfig::autoResponder() != 0 );

	
	connect(rezal, SIGNAL(searchPatternChanged(const QString&)), leSearch, SLOT(setText(const QString &)));
	connect(rezalFavorites, SIGNAL(searchPatternChanged(const QString&)), leSearch, SLOT(setText(const QString &)));

	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(status(const QString&,bool)), this, SLOT(status(const QString&, bool)));
	connect(lister, SIGNAL(countChange(const QString&)), lblCount, SLOT(setText(const QString&)));
	connect(lister, SIGNAL(countChange(const QString&)), this, SIGNAL(setToolTip(const QString&)));
	
	m_properties = new RzxProperty(this);
	if(!RzxConfig::global()->find() || !m_properties->infoCompleted())
	{
		m_properties->initDlg();
		m_properties -> exec();
	}
	if(!m_properties->infoCompleted()) { wellInit = FALSE; return;}
	
	qDebug("=== qRezix Running ===");
	lister->initConnection();

	connect(RzxConfig::global(), SIGNAL(iconFormatChange()), this, SLOT(menuFormatChange()));

	tbRezalContainer->setCurrentIndex(RzxConfig::defaultTab());
	showSearch(RzxConfig::global()->useSearch());
	
	//Raccourcis claviers particuliers
	accel = new QShortcut(Qt::Key_Tab + Qt::SHIFT, this, SLOT(switchTab()));
	menuFormatChange();

	wellInit = TRUE;
}

///Construction des actions
/** Les actions définissent les appels de base des menus/barre d'outils */
void QRezix::buildActions()
{
	pluginsAction = new QAction(tr("Plug-ins"), this);
	//pluginsAction->setCheckable(true);
	pluginsMenu();
	pluginsAction->setMenu(&menuPlugins);
//	connect(pluginsAction, SIGNAL(toggled(bool)), this, SLOT(pluginsMenu(bool)));
//	connect(&menuPlugins, SIGNAL(aboutToHide()), pluginsAction, SLOT(toggle()));

	prefAction = new QAction(tr("Preferences"), this);
	connect(prefAction, SIGNAL(triggered()), this, SLOT(boitePreferences()));
	
	columnsAction = new QAction(tr("Adjust columns"), this);
	connect(columnsAction, SIGNAL(triggered()), rezal, SLOT(afficheColonnes()));
	connect(columnsAction, SIGNAL(triggered()), rezalFavorites, SLOT(afficheColonnes()));
	
	searchAction = new QAction(tr("Search"), this);
	searchAction->setCheckable(true);
/*	connect(searchAction, SIGNAL(toggled(bool)), rezal, SLOT(activeFilter(bool)));
	connect(searchAction, SIGNAL(toggled(bool)), rezalFavorites, SLOT(activeFilter(bool)));*/

	awayAction = new QAction(tr("Away"), this);
	awayAction->setCheckable(true);	
	connect(awayAction, SIGNAL(toggled(bool)), this, SLOT(activateAutoResponder(bool)));
}

void QRezix::launchPlugins()
{
	RzxPlugInLoader::global()->init();
}

void QRezix::languageChanged(){
	qDebug("Language changed");
	languageChange();
/*	rezal->languageChanged();
	rezalFavorites->languageChanged();*/
}

QRezix::~QRezix() {
	qDebug("Bye Bye\n");
}

void QRezix::status(const QString& msg, bool fatal)
{
	lblStatus -> setText(msg);
	statusFlag = !fatal;

	if(statusFlag)
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
		
	qDebug(("Connection status : " + QString(fatal?"disconnected ":"connected ") + "(" + msg + ")").toAscii().constData());
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
	RzxConfig::global()->writeWindowSize(windowSize);
	RzxConfig::global()->writeWindowPosition(pos());
	RzxConfig::global() -> closeSettings();
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
	if(!byTray && !isHidden() && !isMinimized())
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
		if(isMinimized() && RzxConfig::global()->useSystray())
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
	RzxConfig::setAutoResponder(state);
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
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYHERE);
		if(trayIcon.isNull())
			trayIcon = qRezixIcon();
	}
	else
	{
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYAWAY);
		if(trayIcon.isNull())
			trayIcon = qRezixAwayIcon();
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
		activateWindow();
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
	setWindowTitle("qRezix v" + RZX_VERSION);

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
	if(!RzxConnectionLister::global()->isInitialized() || !favoriteWarn)
	{
		favoriteWarn = true;
		return;
	}
	
	//Bah, beep à la connexion
	if(RzxConfig::beepConnection() && computer->state() == Rzx::STATE_HERE)
	{
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
	pluginsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PLUGIN));
	awayAction->setIcon(RzxIconCollection::getResponderIcon());
	columnsAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_COLUMN));
	prefAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_PREFERENCES));
	searchAction->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SEARCH));
	lblCountIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE)
		.scaled(16,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	tbRezalContainer->setItemIcon(1,RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE));
	tbRezalContainer->setItemIcon(0,RzxIconCollection::getPixmap(Rzx::ICON_FAVORITE));
	if(statusFlag)
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_ON));
	else
		lblStatusIcon->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_OFF));
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
	Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
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
				tbRezalContainer->setItemIcon(0,empty);
				tbRezalContainer->setItemIcon(1,empty);
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
void QRezix::pluginsMenu()
{
	menuPlugins.clear();
	RzxPlugInLoader::global()->menuAction(menuPlugins);
	if(!menuPlugins.actions().count())
		menuPlugins.addAction("<none>");
//	menuPlugins.popup(btnPlugins->mapToGlobal(btnPlugins->rect().topRight()));
}

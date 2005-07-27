/***************************************************************************
                          rzxapplication  -  description
                             -------------------
    begin                : Wed Jul 27 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QProcess>

#include "rzxapplication.h"

#include "rzxglobal.h"

#include "qrezix.h"
#include "rzxtrayicon.h"
#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxiconcollection.h"
#include "rzxpluginloader.h"
#include "rzxconnectionlister.h"
#include "rzxclientlistener.h"
#include "rzxserverlistener.h"
#include "rzxtraywindow.h"
#include "rzxproperty.h"

///Chargement de qRezix et de ses différents modules
RzxApplication::RzxApplication(int argc, char **argv)
	:QApplication(argc, argv)
{
	mainui = NULL;
	tray = NULL;
	properties = NULL;
	wellInit = false;

	//Analyse des arguments
	for(int i=1; i<argc; i++)
		if(strncmp(argv[i],"--log-debug=",12)==0)
		{
			int len = strlen(argv[i])-12;
			char *logfile_name = new char[len+1];
			memcpy(logfile_name,argv[i]+12,len+1);
			FILE *logfile = fopen(logfile_name,"a");
			Rzx::useOutputFile(logfile);
			delete[] logfile_name;
			break;
		}

	//Installation du message handler... à partir de maintenant, on peut
	//faire des qDebug...
	Rzx::installMsgHandler();
	qDebug("qRezix %s%s\n", VERSION.toAscii().constData(), RZX_TAG_VERSION);

	//Chargement des configs
	RzxConfig::global();
	setWindowIcon(RzxIconCollection::qRezixIcon());
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
	if(!RzxConfig::global()->find() || !RzxConfig::infoCompleted())
	{
		properties = new RzxProperty(NULL);
		properties->initDlg();
		properties -> exec();
		if(!RzxConfig::infoCompleted())
		{
			wellInit = false;
			return;
		}
	}

	//Chargement des différents modules
	Rzx::beginModuleLoading("Modules loading");
	//Chargement des plugins
	RzxPlugInLoader::global();
	//Chargement de l'ui principale
	mainui = QRezix::global();
	if(mainui && !mainui->isInitialised())
	{
		mainui->deleteLater();
		mainui = NULL;
	}
	connect(mainui, SIGNAL(wantQuit()), this, SLOT(quit()));
	connect(mainui, SIGNAL(wantPreferences()), this, SLOT(preferences()));
	connect(mainui, SIGNAL(wantToggleResponder()), this, SLOT(toggleResponder()));
	if(properties)
		properties->setParent(mainui);

	//Chargement de la trayicon
	tray = new TrayIcon(RzxIconCollection::qRezixIcon(), "qRezix", this);
	if(tray)
	{
		connect(tray, SIGNAL(wantQuit()), this, SLOT(quit()));
		connect(tray, SIGNAL(wantPreferences()), this, SLOT(preferences()));
		connect(tray, SIGNAL(wantToggleResponder()), this, SLOT(toggleResponder()));
		connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), tray, SLOT(changeTrayIcon()));
		connect(RzxIconCollection::global(), SIGNAL(themeChanged(const QString& )), tray, SLOT(changeTrayIcon()));
		connect(RzxConnectionLister::global(), SIGNAL(countChange(const QString& )), tray, SLOT(setToolTip(const QString& )));
		tray->changeTrayIcon();
	}

	//Installation des intéractions entre les modules
	Rzx::beginModuleLoading("Modules interactions");
	if(tray && (!mainui || RzxConfig::global()->useSystray()))
		tray->show();
	else if(mainui)
		mainui->show();

	if(mainui && tray)
	{
		connect(tray,SIGNAL(clicked(const QPoint&)), mainui,SLOT(toggleVisible()));
		connect(mainui,SIGNAL(setToolTip(const QString &)),tray,SLOT(setToolTip(const QString &)));
	}
	Rzx::endModuleLoading("Modules interactions");

	//Fin du chargement des modules
	Rzx::endModuleLoading("Modules loading");

	//Lancement de l'interface réseau
	RzxConnectionLister *lister = RzxConnectionLister::global();
	connect(lister, SIGNAL(login(RzxComputer* )), this, SLOT(installComputer(RzxComputer*)));
	connect(lister, SIGNAL(loginEnd()), this, SLOT(firstLoadingEnd()));
	RzxConnectionLister::global()->initConnection();
	RzxPlugInLoader::global()->init();

	wellInit = true;
}

///Destruction de l'application
/** :( */
RzxApplication::~RzxApplication()
{
}

///Sauvegarde des données au moment de la fermeture
/** Lance la sauvegarde des données principales lors de la fermeture de rezix. Cette méthode est censée permettre l'enregistrement des données lors de la fermeture de l'environnement graphique... */
void RzxApplication::saveSettings()
{
	Rzx::beginModuleLoading("Quitting");
	Rzx::beginModuleLoading("Closing Interface");
	if(mainui)
	{
		qDebug("Closing main window");
		QSize s = mainui->size();       // store size
	
		//Pas très beau, mais c'est juste pour voir si ça améliore le rétablissement de l'état de la fenêtre sur mac
		QString height = QString("%1").arg(s.height(), 4, 10).replace(' ', '0');
		QString width =  QString("%1").arg(s.width(), 4, 10).replace(' ', '0');
		QString windowSize;
#ifndef Q_OS_MAC
		if(mainui->isMaximized())
			windowSize = "100000000";
		else
#endif
			windowSize="0"+height+width;
		RzxConfig::global()->writeWindowSize(windowSize);
		RzxConfig::global()->writeWindowPosition(mainui->pos());
		mainui->deleteLater();
		mainui = NULL;
	}
	if(tray)
	{
		qDebug("Closing trayicon");
		tray->deleteLater();
		tray = NULL;
	}
	qDebug("Closing chats");
	RzxConnectionLister::global() ->closeChats();
	RzxClientListener::global()->close();
	Rzx::endModuleLoading("Closing Interface");
	Rzx::beginModuleLoading("Closing qRezix core");
	qDebug("Closing plugins");
	delete RzxPlugInLoader::global();
	qDebug("Closing connection with server");
	if (!RzxConnectionLister::global() -> isSocketClosed())
	{
		RzxConnectionLister::global() -> closeSocket();
		RzxConnectionLister::global() -> deleteLater();
	}
	qDebug("Closing configuration center");
	RzxConfig::global() -> closeSettings();
	Rzx::endModuleLoading("Closing qRezix core");
	Rzx::endModuleLoading("Quitting");
	qDebug("Bye Bye\n");
}

///Install les connexions du RzxComputer*
void RzxApplication::installComputer(RzxComputer *computer)
{
	connect(computer, SIGNAL(favoriteStateChanged(RzxComputer*)), this, SLOT(warnForFavorite(RzxComputer* )));
}

/// Pour l'affichage d'une notification lors de la connexion d'un favoris
/** Permet d'alerter lors de l'arrivée d'un favoris, que ce soit par l'émission d'un son, ou par l'affichage l'affichage d'un message indiquant la présence de ce favoris */
void RzxApplication::warnForFavorite(RzxComputer *computer)
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
void RzxApplication::firstLoadingEnd()
{
	favoriteWarn = false;
}

/// Affiche la boite de préférences
void RzxApplication::preferences()
{
	if (properties) {
		if (!(properties -> isVisible())) {
			properties -> initDlg();
			properties -> show();
		}
	}
	else {
		properties = new RzxProperty(mainui);
		properties -> show();
	}
}

///Change l'état du répondeur
void RzxApplication::toggleResponder()
{
	RzxConfig::setAutoResponder(!RzxComputer::localhost()->isOnResponder());
	RzxProperty::serverUpdate();
}

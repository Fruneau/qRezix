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
#include "rzxapplication.h"

#include "core/rzxglobal.h"
#include "core/rzxconfig.h"
#include "core/rzxcomputer.h"
#include "core/rzxiconcollection.h"
#include "core/rzxpluginloader.h"
#include "core/rzxconnectionlister.h"
#include "core/rzxserverlistener.h"
#include "core/rzxproperty.h"
#include "core/rzxmodule.h"

#include "mainui/rzxui.h"
#include "tray/rzxtrayicon.h"
#include "chat/rzxchatlister.h"
#include "notifier/rzxnotifier.h"

///Chargement de qRezix et de ses différents modules
RzxApplication::RzxApplication(int argc, char **argv)
	:QApplication(argc, argv)
{
	mainui = NULL;
	properties = NULL;
	chat = NULL;
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

	//Chargement du coeur de qRezix
	if(!loadCore())
		return;
	//Chargement des modules de qRezix
	if(!loadModules())
		return;

	//Lancement de l'interface réseau
	RzxConnectionLister *lister = RzxConnectionLister::global();
	lister->initConnection();
	RzxPlugInLoader::global()->init();

	wellInit = true;
}

///Destruction de l'application
/** :( */
RzxApplication::~RzxApplication()
{
}

///Chargement du coeur de qRezix
/** Le qRezix Core contient :
 * 	- la config
 * 	- l'objet RzxComputer global
 * 	- la gestion de la liste des connexion
 * 	- par extension du précédent, les interfaces réseaux client
 *
 * Tout le reste (interface graphique, chat, trayicon, plugins...)
 * ne fait pas partie de qRezix Core.
 */
bool RzxApplication::loadCore()
{
	//Début du chargement
	Rzx::beginModuleLoading("qRezix Core");

	//Chargement des configs
	RzxConfig::global();
	setWindowIcon(RzxIconCollection::qRezixIcon());

	//Initialisation de l'objet représentant localhost
	RzxComputer::localhost();

	//Vérification du remplissage des propriétés
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));
	if(!RzxConfig::global()->find() || !RzxConfig::infoCompleted())
	{
		properties = new RzxProperty(NULL);
		properties->initDlg();
		properties -> exec();
		if(!RzxConfig::infoCompleted())
			return false;
	}

	//Chargement de la base réseau de qRezix
	RzxConnectionLister::global();
	Rzx::endModuleLoading("qRezix Core");
	return true;
}

///Chargement des modules
/** Charge les modules de qRezix. Les modules doivent absolument
 * être désactivable par suppression tout bonnement de son chargement
 * qui doit se résumer à 1 ligne...
 * Des connexions sont envisageables entre les modules ou plutôt entre
 * catégories de modules...
 */
bool RzxApplication::loadModules()
{
	//Chargement des différents modules
	Rzx::beginModuleLoading("Modules loading");

	//Chargement des plugins
	RzxPlugInLoader::global();

	Rzx::beginModuleLoading("Built-ins");
#define loadBuiltIn(a) installModule(new a)
	loadBuiltIn(RzxUi);
	loadBuiltIn(RzxChatLister);
	loadBuiltIn(RzxNotifier);
	loadBuiltIn(RzxTrayIcon);
#undef loadBuiltIn
	Rzx::endModuleLoading("Built-ins");

	//Installation des intéractions entre les modules
	Rzx::beginModuleLoading("Modules interactions");
	if(properties)
		properties->setParent(mainWindow());

	if(mainui)
	{
		foreach(RzxModule *hider, hiders)
		{
			connect(hider, SIGNAL(wantToggleVisible()), mainui, SLOT(toggleVisible()));
			connect(hider, SIGNAL(wantShow()), mainui, SLOT(show()));
			connect(hider, SIGNAL(wantHide()), mainui, SLOT(hide()));
			hider->show();
		}
		if(!hiders.count())
			mainui->show();
	}

	Rzx::endModuleLoading("Modules interactions");

	//Fin du chargement des modules
	Rzx::endModuleLoading("Modules loading");
	return true;
}

///Installe le module
void RzxApplication::installModule(RzxModule *mod)
{
	if(mod)
	{
		if(!mod->isInitialised())
		{
			delete mod;
			mod = NULL;
		}
		else
		{
			connect(mod, SIGNAL(wantQuit()), this, SLOT(quit()));
			connect(mod, SIGNAL(wantPreferences()), this, SLOT(preferences()));
			connect(mod, SIGNAL(wantToggleResponder()), this, SLOT(toggleResponder()));
			QFlags<RzxModule::TypeFlags> type = mod->type();
			modules << mod;
			if((type & RzxModule::MOD_GUI) && (type & RzxModule::MOD_MAIN) && !mainui)
				mainui = mod;
			if((type & RzxModule::MOD_CHAT) && !chat)
				chat = mod;
			if(type & RzxModule::MOD_HIDE)
				hiders << mod;
		}
	}
}

///Sauvegarde des données au moment de la fermeture
/** Lance la sauvegarde des données principales lors de la fermeture de rezix. Cette méthode est censée permettre l'enregistrement des données lors de la fermeture de l'environnement graphique... */
void RzxApplication::saveSettings()
{
	Rzx::beginModuleClosing("qRezix");
	Rzx::beginModuleClosing("Modules");

	Rzx::beginModuleClosing("Built-ins");
	qDeleteAll(modules);
	Rzx::endModuleClosing("Built-ins");

	delete RzxPlugInLoader::global();
	Rzx::endModuleClosing("Interfaces");
	Rzx::beginModuleClosing("qRezix core");
	qDebug("Closing connection with server");
	if(!RzxConnectionLister::global() -> isSocketClosed())
	{
		RzxConnectionLister::global() -> closeSocket();
		RzxConnectionLister::global() -> deleteLater();
	}
	qDebug("Closing configuration center");
	delete RzxConfig::global();
	Rzx::endModuleClosing("qRezix core");
	Rzx::endModuleClosing("qRezix");
	qDebug("Bye Bye\n");
}

/// Affiche la boite de préférences
void RzxApplication::preferences()
{
	if(properties)
	{
		if(!properties -> isVisible())
		{
			properties -> initDlg();
			properties -> show();
		}
	}
	else
	{
		properties = new RzxProperty(mainWindow());
		properties -> show();
	}
}

///Change l'état du répondeur
void RzxApplication::toggleResponder()
{
	RzxConfig::setAutoResponder(!RzxComputer::localhost()->isOnResponder());
	RzxServerListener::object()->sendRefresh();
}

///Retourne un pointeur vers la fenêtre principale
/** L'intérêt de cette fonction est de fournir un moyen simple
 * de connaître la fenêtre principale pour avoir par un parent...
 */
QWidget *RzxApplication::mainWindow()
{
	if(instance()->mainui)
		return instance()->mainui->mainWindow();
	else
		return NULL;
}

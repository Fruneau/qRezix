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
#include <QLibrary>
#include <QDir>

#include <RzxApplication>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxConnectionLister>
#include <RzxProperty>

#ifdef RZX_MAINUI_BUILTIN
#	include "../mainui/rzxui.h"
#endif
#ifdef RZX_TRAYICON_BUILTIN
#	include "../tray/rzxtrayicon.h"
#endif
#ifdef RZX_CHAT_BUILTIN
#	include "../chat/rzxchatlister.h"
#endif
#ifdef RZX_NOTIFIER_BUILTIN
#	include "../notifier/rzxnotifier.h"
#endif

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
	Rzx::beginModuleLoading("Modules loading");
	loadModules("plugins", "rzx*", "getModule");
	Rzx::endModuleLoading("Modules loading");

	//Lancement de l'interface réseau
	RzxConnectionLister *lister = RzxConnectionLister::global();
	lister->start();
	wellInit = true;
}

///Destruction de l'application
/** :(
 */
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
	new RzxConfig();
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
void RzxApplication::loadBuiltins()
{
	//Chargmenet des builtins
#define loadBuiltIn(a) installModule(new a)
#ifdef RZX_MAINUI_BUILTIN
	loadBuiltIn(RzxUi);
#endif
#ifdef RZX_CHAT_BUILTIN
	loadBuiltIn(RzxChatLister);
#endif
#ifdef RZX_NOTIFIER_BUILTIN
	loadBuiltIn(RzxNotifier);
#endif
#ifdef RZX_TRAYICON_BUILTIN
	loadBuiltIn(RzxTrayIcon);
#endif
#undef loadBuiltIn
}

///Création des liens entre les modules
/** L'application principale nécessite que les modules aient
 * plusieurs catégories, en particulier pour la trayicon,
 * fenêtre principale...
 */
void RzxApplication::linkModules()
{
	//Installation des intéractions entre les modules
	if(properties)
		properties->setParent(mainWindow());

	foreach(RzxModule *hider, hiders)
	{
		if(mainui)
		{
			connect(hider, SIGNAL(wantToggleVisible()), mainui, SLOT(toggleVisible()));
			connect(hider, SIGNAL(wantShow()), mainui, SLOT(show()));
			connect(hider, SIGNAL(wantHide()), mainui, SLOT(hide()));
		}
		hider->show();
	}
	if(mainui && !hiders.count())
		mainui->show();

	//Fin du chargement des modules
	return;
}

///Installe le module
/** L'installation correspond à :
 * 	# vérification du module
 * 	# connexion du module
 */
bool RzxApplication::installModule(RzxModule *mod)
{
	if(RzxBaseLoader<RzxModule>::installModule(mod))
	{
		connect(mod, SIGNAL(wantQuit()), this, SLOT(quit()));
		connect(mod, SIGNAL(wantPreferences()), this, SLOT(preferences()));
		connect(mod, SIGNAL(wantToggleResponder()), this, SLOT(toggleResponder()));
		connect(mod, SIGNAL(wantActivateResponder()), this, SLOT(activateResponder()));
		connect(mod, SIGNAL(wantDeactivateResponder()), this, SLOT(deactivateResponder()));
		connect(mod, SIGNAL(haveProperties(RzxComputer*, bool*)), this, SIGNAL(haveProperties(RzxComputer*, bool*)));
		QFlags<RzxModule::TypeFlags> type = mod->type();
		if((type & RzxModule::MOD_GUI) && (type & RzxModule::MOD_MAIN) && !mainui)
			mainui = mod;
		if((type & RzxModule::MOD_CHAT) && !chat)
			chat = mod;
		if(type & RzxModule::MOD_HIDE)
			hiders << mod;
		return true;
	}
	return false;
}

///Sauvegarde des données au moment de la fermeture
/** Lance la sauvegarde des données principales lors de la fermeture de rezix.
 * Cette méthode est censée permettre l'enregistrement des données lors de la
 * fermeture de l'environnement graphique...
 */
void RzxApplication::saveSettings()
{
	Rzx::beginModuleClosing("qRezix");

	Rzx::beginModuleClosing("Modules");
	closeModules();
	Rzx::endModuleClosing("Modules");

	Rzx::beginModuleClosing("qRezix core");
	if(properties)
		delete properties;

	//Fermeture des connexions
	RzxConnectionLister::global()->stop();
	delete RzxConnectionLister::global();

	//Fermeture et enregistrement de la configuration
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
	RzxConnectionLister::global()->refresh();
}

///Active le répondeur
void RzxApplication::activateResponder()
{
	if(RzxComputer::localhost()->isOnResponder()) return;
	RzxConfig::setAutoResponder(true);
	RzxConnectionLister::global()->refresh();
}

///Désactive le répondeur
void RzxApplication::deactivateResponder()
{
	if(!RzxComputer::localhost()->isOnResponder()) return;
	RzxConfig::setAutoResponder(false);
	RzxConnectionLister::global()->refresh();
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

///Retourne la liste des modules chargés
QList<RzxModule*> RzxApplication::modulesList()
{
	return instance()->moduleList();
}

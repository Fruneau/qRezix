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
#include <QDir>

#include <RzxApplication>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxConnectionLister>
#include <RzxMessageBox>
#include <RzxProperty>
#include <RzxIntro>

#ifdef Q_OS_MAC
#	include <Carbon/Carbon.h>
#endif

#ifdef RZX_MAINUI_BUILTIN
#	include "../modules/mainui/rzxui.h"
#endif
#ifdef RZX_TRAYICON_BUILTIN
#	include "../modules/tray/rzxtrayicon.h"
#endif
#ifdef RZX_CHAT_BUILTIN
#	include "../modules/chat/rzxchatlister.h"
#endif
#ifdef RZX_NOTIFIER_BUILTIN
#	include "../modules/notifier/rzxnotifier.h"
#endif

extern bool withTS;

///Définition de la version de qRezix
/** La version est générée automatiquement grâce aux informations fournies
 * à la compilation. Pour plus d'information, il suffit de se reporter au
 * code qui suit.
 */
Rzx::Version RzxApplication::m_version = {
	RZX_MAJOR_VERSION,
	RZX_MINOR_VERSION,
	RZX_FUNNY_VERSION,
#ifdef RZX_SVNVERSION
	"-svn"
#endif
#ifdef RZX_RELEASEVERSION
	QString()
#endif
#ifdef RZX_ALPHAVERSION
	QString("_alpha%1").arg(RZX_ALPHAVERSION)
#endif
#ifdef RZX_BETAVERSION
	QString("_beta%1").arg(RZX_BETAVERSION)
#endif
#ifdef RZX_RCVERSION
	QString("_rc%1").arg(RZX_RCVERSION)
#endif
#ifdef RZX_PREVERSION
	QString("_pre%1").arg(RZX_PREVERSION)
#endif
#ifdef RZX_RVERSION
	QString("-r%1").arg(RZX_RVERSION)
#endif
};

///Chargement de qRezix et de ses différents modules
RzxApplication::RzxApplication(int argc, char **argv)
	:QApplication(argc, argv), RzxBaseLoader<RzxModule>("modules", "rzx*", "getModule", "getModuleName", "getModuleVersion", "getModuleDescription", "getModuleIcon")
{
	properties = NULL;
	mainui = chatui = propertiesUi = propertiesProto = chatProto = NULL;
	setQuitOnLastWindowClosed(false);
	wellInit = false;

	//Analyse des arguments
	for(int i=1; i<argc; i++)
	{
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
		if(strncmp(argv[i],"--timestamp", 14)==0)
			withTS = true;
		if(strncmp(argv[i],"--help",6)==0 || strncmp(argv[i],"-h",2)==0)
		{
			displayHelp();
			return;
		}
	}

	//Installation du message handler... à partir de maintenant, on peut
	//faire des qDebug...
	Rzx::installMsgHandler();
	qDebug("qRezix %s\n", Rzx::versionToString(version()).toAscii().constData());
	qRegisterMetaType<RzxComputer*>("RzxComputer*");	

	//Chargement du coeur de qRezix
	if(!loadCore())
		return;
	//Chargement des modules de qRezix
	Rzx::beginModuleLoading("Modules");
	loadModules();
	Rzx::endModuleLoading("Modules");

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
	setSettings(new RzxConfig());
	setWindowIcon(RzxIconCollection::qRezixIcon());
#ifdef Q_OS_MAC
	CGImageRef ir = (CGImageRef)RzxIconCollection::qRezixIcon().macCGHandle();
	SetApplicationDockTileImage(ir);
#endif

	//Initialisation de l'objet représentant localhost
	RzxComputer::localhost();

	//Vérification du remplissage des propriétés
	connect(this, SIGNAL(aboutToQuit()), this, SLOT(saveSettings()));

	bool first = RzxConfig::global()->firstLaunch();
	if(first || !RzxConfig::infoCompleted())
	{
		if(first)
		{
			RzxIntro *intro = new RzxIntro();
			intro->exec();

#ifdef RZX_SVNVERSION
			RzxMessageBox::warning(NULL, tr("Welcome to qRezix"),
				tr("This is a development version, provided as is, <b>without any garantee</b> of stability or functionality.<br><br><i>Use at your own risk.</i>"), true);
#endif
		}
		properties = new RzxProperty(NULL);
		properties->initDlg();
		properties -> exec();
		delete properties;
		properties = NULL;
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
	if(addBuiltin(RzxThemedIcon("mainui"), "Main UI", m_version, "Main UI for qRezix"))
		loadBuiltIn(RzxUi);
#endif
#ifdef RZX_CHAT_BUILTIN
	if(addBuiltin(Rzx::ICON_CHAT, "Chat", m_version, "qRezix graphical chat interface"))
		loadBuiltIn(RzxChatLister);
#endif
#ifdef RZX_NOTIFIER_BUILTIN
	if(addBuiltin(Rzx::ICON_FAVORITE, "Favorite Notifier", m_version, "Notify that a favorite state has changed"))
		loadBuiltIn(RzxNotifier);
#endif
#ifdef RZX_TRAYICON_BUILTIN
	if(addBuiltin(RzxThemedIcon("trayicon"), "Tray icon", m_version, "Systray and Dock integration"))
		loadBuiltIn(RzxTrayIcon);
#endif
#undef loadBuiltIn
}

///Connecte un module 'hider' à la mainui
void RzxApplication::installHider(RzxModule *hider)
{
	if(mainui)
	{
		connect(hider, SIGNAL(wantToggleVisible()), mainui, SLOT(toggleVisible()));
		connect(hider, SIGNAL(wantShow()), mainui, SLOT(show()));
		connect(hider, SIGNAL(wantHide()), mainui, SLOT(hide()));
	}
}

///Création des liens entre les modules
/** L'application principale nécessite que les modules aient
 * plusieurs catégories, en particulier pour la trayicon,
 * fenêtre principale...
 */
void RzxApplication::linkModules()
{
	//Installation des intéractions entre les modules
	foreach(RzxModule *hider, hiders)
	{
		installHider(hider);
		hider->show();
	}
	if(mainui && !hiders.count())
		mainui->show();
}

///Création des liens avec un nouveau module
void RzxApplication::relinkModules(RzxModule *newMod, RzxModule *oldMod)
{
	if(newMod)
	{
		if(newMod->type() & RzxModule::MOD_HIDE)
		{
			installHider(newMod);
			newMod->show();
		}
		else if(mainui == newMod)
		{
			foreach(RzxModule *hider, hiders)
				installHider(hider);
		}
	}

	if(oldMod && mainui && !hiders.count())
		mainui->show();
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
		connect(mod, SIGNAL(haveProperties(RzxComputer*)), this, SLOT(relayProperties(RzxComputer*)));
		QFlags<RzxModule::TypeFlags> type = mod->type();
		if((type & RzxModule::MOD_GUI) && (type & RzxModule::MOD_MAINUI) && !mainui)
			mainui = mod;
		if((type & RzxModule::MOD_CHAT) && !chatProto)
			chatProto = mod;
		if((type & RzxModule::MOD_CHATUI) && !chatui)
			chatui = mod;
		if((type & RzxModule::MOD_PROPERTIES) && !propertiesProto)
			propertiesProto = mod;
		if((type & RzxModule::MOD_PROPERTIESUI) && !propertiesUi)
			propertiesUi = mod;
		if(type & RzxModule::MOD_HIDE)
			hiders << mod;
		return true;
	}
	return false;
}

///Décharge un module
void RzxApplication::unloadModule(RzxModule *module)
{
	hiders.removeAll(module);

#define clear(var) if(var == module) var = NULL
	clear(mainui);
	clear(chatProto);
	clear(chatui);
	clear(propertiesProto);
	clear(propertiesUi);
#undef clear

	RzxBaseLoader<RzxModule>::unloadModule(module);
}

///Sauvegarde des données au moment de la fermeture
/** Lance la sauvegarde des données principales lors de la fermeture de rezix.
 * Cette méthode est censée permettre l'enregistrement des données lors de la
 * fermeture de l'environnement graphique...
 */
void RzxApplication::saveSettings()
{
	Rzx::beginModuleClosing("qRezix");
	if(properties)
		delete properties;

	Rzx::beginModuleClosing("Modules");
	closeModules();
	Rzx::endModuleClosing("Modules");

	Rzx::beginModuleClosing("qRezix core");
	//Fermeture des connexions
	RzxConnectionLister::global()->stop();
	delete RzxConnectionLister::global();

	//Fermeture et enregistrement de la configuration
	delete RzxComputer::localhost();
	delete RzxConfig::global();

	Rzx::endModuleClosing("qRezix core");
	Rzx::endModuleClosing("qRezix");
	qDebug("Bye Bye\n");
	Rzx::closeMsgHandler();
}

/// Affiche la boite de préférences
void RzxApplication::preferences()
{
	if(properties)
	{
		if(!properties -> isVisible())
			properties -> initDlg();
		properties -> raise();
		properties -> show();
	}
	else
	{
		properties = new RzxProperty(NULL);
		properties -> raise();
		properties -> show();
	}
}

///Retourne la version de qRezix
Rzx::Version RzxApplication::version()
{
	return m_version;
}

///Indique si l'application a été initialisée sans encombre
/** Si le flags est faux, l'application est reconnue comme n'étant
 * pas en état de fonctionner..., il faut donc faire attention à ne pas
 * lancer l'application ou utiliser des modules
 */
bool RzxApplication::isInitialised() const
{
	return wellInit;
}

///Instance de l'application
RzxApplication *RzxApplication::instance()
{
	return qobject_cast<RzxApplication*>(QApplication::instance());
}

///Indique si l'application bénéficie d'une trayicon
/** La trayicon a un statut particulier car elle permet à l'application
 * d'avoir une intéraction 'discrète'...
 */
bool RzxApplication::hasHider() const
{
	return hiders.count();
}

///Indique si l'application a une fenêtre principale
/** La fenêtre principale est une fenêtre permettant un intéraction maximale
 * entre l'utilisateur et le programme.
 */
bool RzxApplication::hasMainWindow() const
{
	return mainui != NULL;
}

///Retourne le module de chat
/** Le module retourné ici est un module à utiliser par défaut pour le chat si
 * l'ordinateur utilise un protocole qui ne gère pas le chat.
 */
RzxModule *RzxApplication::chatModule()
{
	return instance()->chatProto;
}

///Retourne le module des propriétés
/** Le module retourné ici est un module à utiliser par défaut pour le check des
 * propriétés de l'utilisateur distant si le protocole réseau via lequel il est connecté
 * ne le permet pas.
 */
RzxModule *RzxApplication::propertiesModule()
{
	return instance()->propertiesProto;
}

///Retourne le module de l'interface utilisateur pour le chat
RzxModule *RzxApplication::chatUiModule()
{
	return instance()->chatui;
}

///Change l'état du répondeur
void RzxApplication::toggleResponder()
{
	RzxConfig::setAutoResponder(!RzxComputer::localhost()->isOnResponder());
}

///Active le répondeur
void RzxApplication::activateResponder()
{
	if(RzxComputer::localhost()->isOnResponder()) return;
	RzxConfig::setAutoResponder(true);
}

///Désactive le répondeur
void RzxApplication::deactivateResponder()
{
	if(!RzxComputer::localhost()->isOnResponder()) return;
	RzxConfig::setAutoResponder(false);
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

///Retourne le fenêtre de préférence
RzxProperty *RzxApplication::preferencesWindow()
{
	return instance()->properties;
}

///Retourne la liste des modules chargés
QList<RzxModule*> RzxApplication::modulesList()
{
	return instance()->moduleList();
}

///Relai l'information que des propriétés sont arrivées
void RzxApplication::relayProperties(RzxComputer *c)
{
	bool used = false;
	emit haveProperties(c, &used);
	if(!used && propertiesUi)
		propertiesUi->showProperties(c);
}

///Affiche l'aide de qRezix
void RzxApplication::displayHelp()
{
	qDebug("Usage: qrezix [options]");
	qDebug("qRezix options:");
	qDebug("  --help -h            Display this information");
	qDebug("  --log-debug=<file>   Output debug information into <file> instead of stdout");
	qDebug("  --timestamp          Add timestamp to debug information");
	qDebug("\nQt options:");
	qDebug("  -nograb -style -session -widgetcount");
#ifdef Q_WS_X11
	qDebug("  -dograb -sync -display -geometry -fn -font -bg -background -fg -foreground");
	qDebug("  -name -title -visual -ncols -cmap");
#endif
	qDebug("\nqRezix %s", Rzx::versionToString(version()).toAscii().constData());
	qDebug("Contact or bug reporting: <mailto:qrezix@frankiz.polytechnique.fr>");
}

#ifdef Q_OS_MAC
bool RzxApplication::macEventFilter( EventHandlerCallRef caller, EventRef event )
{
	if(GetEventClass(event) == kEventClassApplication && GetEventKind(event) == kEventAppActivated && mainui)
		mainui->show();
	return QApplication::macEventFilter(caller,event );
}
#endif

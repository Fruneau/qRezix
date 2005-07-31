/***************************************************************************
                      RzxPlugInloader.cpp  -  description
                             -------------------
    begin                : Thu Jul 20 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLibrary>
#include <QToolButton>
#include <QVariant>
#include <QPixmap>
#include <QImage>
#include <QIcon>
#include <QTextEdit>

#include "rzxpluginloader.h"

#include "rzxglobal.h"

#include "rzxconfig.h"
#include "rzxcomputer.h"
#include "rzxmessagebox.h"
#include "rzxhostaddress.h"
#include "rzxutilslauncher.h"
#include "rzxserverlistener.h"
#include "rzxconnectionlister.h"
#include "rzxiconcollection.h"

///L'object global... normalement seul objet de cette classe lors de l'exécution
RzxPlugInLoader *RzxPlugInLoader::object = NULL;

///Construction d'une interface de gestion des plug-ins
/** La construction du programme se fait simplement par recherche et chargement des plugins existant. Le chargement et le lancement de l'exécution des plug-ins se fait de manière distincte. On peut donc envisager la désactivation des plugs-ins par le programme principale */
RzxPlugInLoader::RzxPlugInLoader() : QObject()
{
	Rzx::beginModuleLoading("Plugins");
	initialized = false;
	
	//on charge les plug-ins dans les rep /usr/share/qrezix/plugins
	loadPlugIn(RzxConfig::global()->systemDir());

	//et $HOME/.rezix/plugins
	if (RzxConfig::global()->userDir() != RzxConfig::global()->systemDir())
		loadPlugIn(RzxConfig::global()->userDir());
	if (RzxConfig::global()->libDir() != RzxConfig::global()->systemDir())
		loadPlugIn(RzxConfig::global()->libDir());

	Rzx::endModuleLoading("Plugins");
}

/// Recherche des plugins et chargement dans un répertoire
/** Le chargement se fait en fait dans sourceDir/plugins */
void RzxPlugInLoader::loadPlugIn(const QDir &dir)
{
	QDir sourceDir(dir);
	
	//vérification de l'existence du rep sourceDir/plugins
	if(!sourceDir.cd("plugins"))
		return;

	//les plugins doivent avoir un nom de fichier qui contient rzxpi
	//		par exemple librzxpixplo.so ou rzxpixplo.dll
#ifdef WIN32 //sous linux c'est librzxpi<nomduplugin>.so
	sourceDir.setNameFilters(QStringList() << "rzxpi*");
#else //sous windows c'est rzxpi<nomduplugin>.dll
#ifdef Q_OS_MAC
	sourceDir.setNameFilters(QStringList() << "librzxpi*.dylib");
#else
	sourceDir.setNameFilters(QStringList() << "librzxpi*.so");
#endif
#endif
	QStringList trans=sourceDir.entryList(QDir::Files|QDir::Readable);
	
	//chargement des plugins dans les fichiers
	QVariant *pipath = new QVariant(sourceDir.canonicalPath());
	QVariant *userpath = new QVariant(RzxConfig::global()->userDir().canonicalPath());
	foreach(QString it, trans)
	{
		//tout plug-in doit avoir une fonction RzxPlugIn *getPlugIn() qui renvoi un plugin
		//à partir duquel on peut traiter.
		QLibrary *lib = new QLibrary(sourceDir.filePath(it));
		loadPlugInProc getPlugIn = (loadPlugInProc)(lib->resolve("getPlugIn"));
		if(getPlugIn)
		{
			RzxPlugIn *pi = getPlugIn();
			if(pi)
			{
				QString log = "Plugin " + pi->getName() + " (version: " + QString::number(pi->getVersion(), 16) + "/" + QString::number(PLUGIN_VERSION, 16) + ")";
				//chargement du plugin et connexion au programme
				if(pi->getVersion() > PLUGIN_VERSION || (pi->getVersion() & 0xfffff000) != (PLUGIN_VERSION & 0xfffff000))
				{
					log += " - not loaded : wrong version number";
					RzxMessageBox::information(NULL,
						tr("Unable to load a plug-in"),
						tr("The plug-in named %1 owns a version number which is not supported by this version of qRezix.\n")
							.arg(pi->getName()) + (pi->getVersion() > PLUGIN_VERSION ? 
								tr("A more recent version of qRezix is certainly available. Update qRezix if you want to use this plug-in.")
								:tr("A new version of the plug-in may be available. Install it if you want to use this plug-in.")));
					delete lib;
				}
				else if(pluginByName[pi->getName()])
				{
					log += " - not loaded : already loaded";
					delete lib;
				}
				else
				{
					log += " - loaded from " + sourceDir.absolutePath();
					RzxComputer::localhost()->addCapabilities(pi->getFeatures());
					connect(pi, SIGNAL(send(const QString&)), RzxServerListener::object(), SLOT(sendProtocolMsg(const QString&)));
					connect(pi, SIGNAL(queryData(RzxPlugIn::Data, RzxPlugIn*)), this, SLOT(sendQuery(RzxPlugIn::Data, RzxPlugIn*)));
					connect(pi, SIGNAL(requestAction(RzxPlugIn::Action, const QString& )), this, SLOT(action(RzxPlugIn::Action, const QString& )));
					pi->getData(RzxPlugIn::DATA_PLUGINPATH, pipath);
					pi->getData(RzxPlugIn::DATA_USERDIR, userpath);
					pi->setSettings(RzxConfig::global()->settings);
					plugins << pi;
					pluginByName.insert(pi->getName(), pi);
					fileByName.insert(pi->getName(), lib);
					state.insert(pi, false);
				}
				qDebug(log.toAscii().constData());
			}
		}
		else
		{
			RzxMessageBox::information(NULL,
				tr("Unable to load a plug-in"),
				tr("A plug-in file has been found but qRezix can't extract any plug-in from it. Maybe the plug-in file is corrupted or not up-to-date.\n Try to install the last version of this plug-in (file %1).").arg(it));
			delete lib;
		}
	}
	delete pipath;
	delete userpath;
}

/// Fermeture et destruction des plug-ins
/**La fermeture des plugins doit se faire dans les règles parce que j'aime pas les erreurs de segmentation même à la fermeture du programme.*/
RzxPlugInLoader::~RzxPlugInLoader()
{
	Rzx::beginModuleClosing("Plugins");
	foreach(RzxPlugIn *pi, plugins)
	{
		if(state[pi]) pi->stop();
		delete pi;
	}
#ifndef WIN32
	qDeleteAll(fileByName);
#endif
	fileByName.clear();
	Rzx::endModuleClosing("Plugins");
}

/// lancement de l'exécution des plug-ins
/** Il est du devoir des programmeurs de plugin de faire des programmes qui utilisent au maximum la programmation asynchrone. En effet, il est inimaginable de concevoir un plug-in qui viendrait bloquer le programme à son lancement */
void RzxPlugInLoader::init()
{
	initialized = true;
	QStringList ignored = RzxConfig::ignoredPluginsList();
	foreach(RzxPlugIn *pi, plugins)
	{
		if(ignored.contains(pi->getName()))
			state[pi] = false;
		else
		{
			//le userDir sert à donner l'emplacement des données de configuration
			//en particulier, c'est comme ça que les plug-ins peuvent écrirent leurs données
			//dans le qrezixrc du rep .rezix
			if(!state[pi]) pi->init(RzxConfig::global()->settings);
			state[pi] = true;
		}
	}
}

/// Lancement de l'exécution d'un plug-in particulier
void RzxPlugInLoader::init(const QString& name)
{
	RzxPlugIn *pi = pluginByName[name];
	if(pi)
	{
		if(!state[pi]) pi->init(RzxConfig::global()->settings);
		state[pi] = true;
	}
}

/// Arrêt de l'exécution de tous les plug-ins
/** Bien que l'arrêt soit réalisé par le destructeur de ~RzxPlugInLoader cette méthode est là au cas où */
void RzxPlugInLoader::stop()
{
	foreach(RzxPlugIn *pi, plugins)
	{
		if(state[pi]) pi->stop();
		state[pi] = false;
	}
}

/// Arrêt de l'exécution d'un plug-in particulier
void RzxPlugInLoader::stop(const QString& name)
{
	RzxPlugIn *pi = pluginByName[name];
	if(pi)
	{
		if(state[pi]) pi->stop();
		state[pi] = false;
	}
}

/// Recharge le plug-in
void RzxPlugInLoader::reload()
{
}


///Changement de la classe setting
/** Pour permettre un flush des settings par qRezix */
void RzxPlugInLoader::setSettings()
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		pi->setSettings(RzxConfig::global()->settings);
}

/* mélange avec l'interface */
//Cette partie gère l'ajout des plug-ins à l'interface.
//Ainsi chaque plug-in peut donner un sous-menu pour le menu de la tray icon
//ou pour celui qui aparaît lors du clic droit sur un item du rezal

/// ajout des plug-ins au menu de la trayicon
void RzxPlugInLoader::menuTray(QMenu& menu)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
	{
		QMenu *piMenu = pi->getTrayMenu();
		if(piMenu && piMenu->actions().count() && state[pi])
		{
			QPixmap *icon = pi->getIcon();
			if(icon)
				menu.addAction(*icon, pi->getName(), piMenu, 0, 0);
			else
				menu.addAction(pi->getName(), piMenu, 0, 0);
		}
	}
	if(menu.actions().count()) menu.addSeparator();
}

/// ajout des plug-ins au menu des items du rezal
void RzxPlugInLoader::menuItem(QMenu& menu)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
	{
		QMenu *piMenu = pi->getItemMenu();
		if(piMenu && piMenu->actions().count() && state[pi])
		{
			QPixmap *icon = pi->getIcon();
			if(icon)
				menu.addAction(*icon, pi->getName(), piMenu, 0, 0);
			else
				menu.addAction(pi->getName(), piMenu, 0, 0);
		}
	}
}

/// Génération du menu plug-ins de la fenêtre principale
void RzxPlugInLoader::menuAction(QMenu& menu)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
	{
		QMenu *piMenu = pi->getActionMenu();
		if(piMenu && piMenu->actions().count() && state[pi])
		{
			QPixmap *icon = pi->getIcon();
			if(icon)
				menu.addAction(*icon, pi->getName(), piMenu, 0, 0);
			else
				menu.addAction(pi->getName(), piMenu, 0, 0);
		}
	}
}

///Mise à jour du menu plug-in de la fenêtre de chat
void RzxPlugInLoader::menuChat(QMenu& menu)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
	{
		QMenu *piMenu = pi->getChatMenu();
		if(piMenu && piMenu->actions().count() && state[pi])
		{
			QPixmap *icon = pi->getIcon();
			if(icon)
				menu.addAction(icon->scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
							   pi->getName(), piMenu, 0, 0);
			else
				menu.addAction(pi->getName(), piMenu, 0, 0);
		}
	}
}


/* Gestion de l'affichage des propriétés */
//Chaque plug-in peut avoir une fenêtre de réglage des propriétés
//pour faire des réglages propres aux plug-in.

/// Préparation de la liste des plug-ins
void RzxPlugInLoader::makePropListView(QTreeWidget *lv, QToolButton *btnProp, QToolButton *btnReload)
{
	if(!initialized) return;
	if(!lvItems.isEmpty()) return;
	pluginListView = lv;
	pluginGetProp = btnProp;

	//Non encore implémenté
	btnReload->setEnabled(false);
	
	//la fenêtre consiste en 1 - le nom du plug-in     2 - la description
	//les deux étant fournis par le plug-in
	foreach(RzxPlugIn *pi, plugins)
	{
		QTreeWidgetItem *lvi = new QTreeWidgetItem(lv);
		lvi->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
		lvi->setText(0, pi->getName());
		lvi->setText(1, pi->getDescription());
		lvi->setText(2, pi->getInternalVersion());
		lvi->setCheckState(0, state[pi]?Qt::Checked:Qt::Unchecked);
		QPixmap *icon = pi->getIcon();
		if(icon)
			lvi->setIcon(0, *icon);
	}
	btnProp->setEnabled(false);
	
	//gestion des actions
	connect(lv, SIGNAL(itemSelectionChanged()), this, SLOT(changePlugIn()));
	connect(btnProp, SIGNAL(clicked()), this, SLOT(dispProperties()));
	connect(btnReload, SIGNAL(clicked()), this, SLOT(reload()));
}

/// Mise à jour de l'état des plug-ins à partir du statut de la listview
void RzxPlugInLoader::validPropListView()
{
	QStringList ignored;
	foreach(RzxPlugIn *pi, plugins)
	{
		QTreeWidgetItem *lvi = lvItems[pi];
		if(lvi && lvi->checkState(0) && !state[pi])
			init(lvi->text(0));
		else if(lvi && !lvi->checkState(0) && state[pi])
		{
			stop(lvi->text(0));
			ignored.append(lvi->text(0));
		}
	}
	changePlugIn();
	RzxConfig::writeIgnoredPluginsList(ignored);
}

/// Mise à jour de l'état du bouton en fonction du plug-in sélectionné
void RzxPlugInLoader::changePlugIn()
{
	if(!initialized) return;
	QTreeWidgetItem *lvi = pluginListView->currentItem();
	selectedPlugin = lvItems.key((QTreeWidgetItem*)lvi);
	if(!selectedPlugin || !state[selectedPlugin])
	{
		pluginGetProp->setEnabled(false);
		return;
	}
	pluginGetProp->setEnabled(selectedPlugin->hasProp());
	selectedPlugInName = selectedPlugin->getName();
}

/// Lancement de la gestion des propriétés
/**La main est juste donnée au plug-in
 * <br>C'est encore une fois donné à la responsabilité du concepteur du plug-in pour que la fonction ne bloque pas l'exécution du programme */
void RzxPlugInLoader::dispProperties()
{
	if(!initialized) return;
	if(!selectedPlugin) return;
	selectedPlugin->properties();
}

/// Envoi de données aux plugins 
/** \param plugin si est NULL, les données seront envoyées à tous le plugins */
void RzxPlugInLoader::sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin)
{
	if(!initialized) return;
	//On prépare les données en fonction de ce qui nous a été demandé d'envoyer
	QVariant *value;
	switch((int)data)
	{
		case RzxPlugIn::DATA_SERVERFTP: value = new QVariant(RzxComputer::localhost()->servers() & RzxComputer::SERVER_FTP); break;
		case RzxPlugIn::DATA_SERVERHTTP: value = new QVariant(RzxComputer::localhost()->servers() & RzxComputer::SERVER_HTTP); break;
		case RzxPlugIn::DATA_SERVERNEWS: value = new QVariant(RzxComputer::localhost()->servers() & RzxComputer::SERVER_NEWS); break;
		case RzxPlugIn::DATA_SERVERSMB: value = new QVariant(RzxComputer::localhost()->servers() & RzxComputer::SERVER_SAMBA); break;
		case RzxPlugIn::DATA_DNSNAME: value = new QVariant(RzxComputer::localhost()->name()); break;
		case RzxPlugIn::DATA_NAME: value = new QVariant(RzxConfig::propLastName()); break;
		case RzxPlugIn::DATA_FIRSTNAME: value = new QVariant(RzxConfig::propName()); break;
		case RzxPlugIn::DATA_SURNAME: value = new QVariant(RzxConfig::propSurname()); break;
		case RzxPlugIn::DATA_IP: value = new QVariant(RzxComputer::localhost()->ip().toString()); break;
		case RzxPlugIn::DATA_WORKSPACE: value = new QVariant(RzxConfig::FTPPath()); break;
		case RzxPlugIn::DATA_DISPFTP: value = new QVariant(RzxComputer::localhost()->serverFlags() & RzxComputer::SERVER_FTP); break;
		case RzxPlugIn::DATA_DISPHTTP: value = new QVariant(RzxComputer::localhost()->serverFlags() & RzxComputer::SERVER_HTTP); break;
		case RzxPlugIn::DATA_DISPNEWS: value = new QVariant(RzxComputer::localhost()->serverFlags() & RzxComputer::SERVER_NEWS); break;
		case RzxPlugIn::DATA_DISPSMB: value = new QVariant(RzxComputer::localhost()->serverFlags() & RzxComputer::SERVER_SAMBA); break;
		case RzxPlugIn::DATA_LANGUAGE: value = new QVariant(tr("English")); break;
		case RzxPlugIn::DATA_THEME: value = new QVariant(RzxConfig::iconTheme()); break;
		case RzxPlugIn::DATA_AWAY: value = new QVariant(RzxConfig::autoResponder()); break;
		case RzxPlugIn::DATA_ICONFTP: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_FTP)); break;
		case RzxPlugIn::DATA_ICONHTTP: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_HTTP)); break;
		case RzxPlugIn::DATA_ICONNEWS: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NEWS)); break;
		case RzxPlugIn::DATA_ICONSAMBA: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_SAMBA)); break;
		case RzxPlugIn::DATA_ICONNOFTP: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NOFTP)); break;
		case RzxPlugIn::DATA_ICONNOHTTP: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NOHTTP)); break;
		case RzxPlugIn::DATA_ICONNONEWS: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NONEWS)); break;
		case RzxPlugIn::DATA_ICONNOSAMBA: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NOSAMBA)); break;
		case RzxPlugIn::DATA_ICONSAMEGATEWAY: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_SAMEGATEWAY)); break;
		case RzxPlugIn::DATA_ICONDIFFGATEWAY: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OTHERGATEWAY)); break;
		case RzxPlugIn::DATA_ICONJONE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_JONE)); break;
		case RzxPlugIn::DATA_ICONORANGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_ORANJE)); break;
		case RzxPlugIn::DATA_ICONROUJE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_ROUJE)); break;
		case RzxPlugIn::DATA_ICONUNKOS: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS0)); break;
		case RzxPlugIn::DATA_ICONUNKOSLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS0_LARGE)); break;
		case RzxPlugIn::DATA_ICONWIN: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS1)); break;
		case RzxPlugIn::DATA_ICONWINLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS1_LARGE)); break;
		case RzxPlugIn::DATA_ICONWINNT: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS2)); break;
		case RzxPlugIn::DATA_ICONWINNTLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS2_LARGE)); break;
		case RzxPlugIn::DATA_ICONLINUX: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS3)); break;
		case RzxPlugIn::DATA_ICONLINUXLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS3_LARGE)); break;
		case RzxPlugIn::DATA_ICONMAC: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS4)); break;
		case RzxPlugIn::DATA_ICONMACLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS4_LARGE)); break;
		case RzxPlugIn::DATA_ICONMACX: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS5)); break;
		case RzxPlugIn::DATA_ICONMACXLARGE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OS5_LARGE)); break;
		case RzxPlugIn::DATA_ICONSPEAKER: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_SOUNDON)); break;
		case RzxPlugIn::DATA_ICONNOSPEAKER: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_SOUNDOFF)); break;
		case RzxPlugIn::DATA_ICONPLUGIN: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_PLUGIN)); break;
		case RzxPlugIn::DATA_ICONHERE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_HERE)); break;
		case RzxPlugIn::DATA_ICONAWAY: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_AWAY)); break;
		case RzxPlugIn::DATA_ICONCOLUMNS: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_COLUMN)); break;
		case RzxPlugIn::DATA_ICONPREF: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_PREFERENCES)); break;
		case RzxPlugIn::DATA_ICONSEND: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_SEND)); break;
		case RzxPlugIn::DATA_ICONHIST: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_HISTORIQUE)); break;
		case RzxPlugIn::DATA_ICONPROP: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_PROPRIETES)); break;
		case RzxPlugIn::DATA_ICONCHAT: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_CHAT)); break;
		case RzxPlugIn::DATA_ICONSIZE: value = new QVariant(RzxConfig::menuIconSize()); break;
		case RzxPlugIn::DATA_ICONTEXT: value = new QVariant(RzxConfig::menuTextPosition()); break;
		case RzxPlugIn::DATA_ICONFAVORITE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_FAVORITE)); break;
		case RzxPlugIn::DATA_ICONNOTFAVORITE: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_NOTFAVORITE)); break;
		case RzxPlugIn::DATA_CONNECTEDLIST: value = new QVariant(RzxConnectionLister::global()->getIpList()); break;
		case RzxPlugIn::DATA_ICONOK: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_OK)); break;
		case RzxPlugIn::DATA_ICONAPPLY: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_APPLY)); break;
		case RzxPlugIn::DATA_ICONCANCEL: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_CANCEL)); break;
		case RzxPlugIn::DATA_ICONBAN: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_BAN)); break;
		case RzxPlugIn::DATA_ICONUNBAN: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_UNBAN)); break;
		case RzxPlugIn::DATA_ICONQUIT: value = new QVariant(RzxIconCollection::getPixmap(Rzx::ICON_QUIT)); break;
		case RzxPlugIn::DATA_FEATUREDLIST:
			if(plugin)
				value = new QVariant(RzxConnectionLister::global()->getIpList((Rzx::Capabilities)plugin->getFeatures()));
			else
				value = new QVariant(RzxConnectionLister::global()->getIpList());
			break;
				
		default: return;
	}
	
	//envoie des données
	//au plugin demandeur si il y en a un
	if(plugin) plugin->getData(data, value);
	
	//à tous les plugins si aucun plugin particulier n'a été précisé
	else
		foreach(RzxPlugIn *pi, plugins)
			if(state[pi]) pi->getData(data, value);

	delete value;
}

///Exécution des actions demandées par le plugin
/** Permet la demande d'action par le plug-in à qRez */
void RzxPlugInLoader::action(RzxPlugIn::Action action, const QString&)
{
	if(!initialized) return;
	switch((int)action)
	{
		case RzxPlugIn::ACTION_NONE:
			break;

		case RzxPlugIn::ACTION_CHAT:
			//RzxConnectionLister::global()->createChat(param);
			break;

		case RzxPlugIn::ACTION_CLOSE_CHAT:
			//RzxConnectionLister::global()->closeChat(param);
			break;
			
/*		case RzxPlugIn::ACTION_FTP:
			RzxUtilsLauncher::global()->ftp(param);
			break;
			
		case RzxPlugIn::ACTION_HTTP:
			RzxUtilsLauncher::global()->http(param);
			break;
			
		case RzxPlugIn::ACTION_NEWS:
			RzxUtilsLauncher::global()->news(param);
			break;
			
		case RzxPlugIn::ACTION_SMB:
			RzxUtilsLauncher::global()->samba(param);
			break;*/
			
		case RzxPlugIn::ACTION_MINIMIZE:
			break;
		
		case RzxPlugIn::ACTION_QUIT:
			break;
	}
}

/// Envoi des certifications de changement d'état du plug-in
/** On a changé de fenêtre de chat, on l'indique à tout les plug-ins */
void RzxPlugInLoader::chatChanged(QTextEdit *chat)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_CHAT, (QVariant*)chat);
}

/// On indique que le chat envoie le message
void RzxPlugInLoader::chatSending()
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_CHATEMIT, NULL);
}

/// On indique que le chat reçoit un message
void RzxPlugInLoader::chatReceived(QString *chat)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_CHATRECEIVE, (QVariant*)chat);
}

/// On indique que le chat a envoyé un plug-in
void RzxPlugInLoader::chatEmitted(QString *chat)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_CHATEMITTED, (QVariant*)chat);
}

///On indique qu'un nouvel item vient d'être sélectionné
void RzxPlugInLoader::itemChanged(QTreeWidgetItem *item)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_ITEMSELECTED, (QVariant*)item);
}

///On indique que l'item sélectionné chez les favoris à changé
void RzxPlugInLoader::favoriteChanged(QTreeWidgetItem *item)
{
	if(!initialized) return;
	foreach(RzxPlugIn *pi, plugins)
		if(state[pi]) pi->getData(RzxPlugIn::DATA_FAVORITESELECTED, (QVariant*)item);
}

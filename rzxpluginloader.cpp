/***************************************************************************
                      rzxpluginloader.cpp  -  description
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

#include <qpopupmenu.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qlistview.h>
#include <qlibrary.h>
#include <qtoolbutton.h>
#include <qobject.h>
#include <qdir.h>
#include <qvariant.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qiconset.h>

#include "rzxpluginloader.h"

#include "qrezix.h"
#include "rzxrezal.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxserverlistener.h"
#include "rzxhostaddress.h"
#include "rzxhostaddress.h"
#include "rzxmessagebox.h"
#include "rzxutilslauncher.h"
#include "rzxconnectionlister.h"

///L'object global... normalement seul objet de cette classe lors de l'exécution
RzxPlugInLoader *RzxPlugInLoader::object = NULL;

///Construction d'une interface de gestion des plug-ins
/** La construction du programme se fait simplement par recherche et chargement des plugins existant. Le chargement et le lancement de l'exécution des plug-ins se fait de manière distincte. On peut donc envisager la désactivation des plugs-ins par le programme principale */
RzxPlugInLoader::RzxPlugInLoader() : QObject(0, 0)
{
	pluginFlags = 0;
	initialized = false;
	object = this;
	
	//on charge les plug-ins dans les rep /usr/share/qrezix/plugins
	loadPlugIn(RzxConfig::globalConfig()->systemDir());
#ifndef WIN32
	//et $HOME/.rezix/plugins
	if (RzxConfig::globalConfig()->userDir() != RzxConfig::globalConfig()->systemDir())
		loadPlugIn(RzxConfig::globalConfig()->userDir());
	if (RzxConfig::globalConfig()->libDir() != RzxConfig::globalConfig()->systemDir())
		loadPlugIn(RzxConfig::globalConfig()->libDir());
#endif
}

/// Recherche des plugins et chargement dans un répertoire
/** Le chargement se fait en fait dans sourceDir/plugins */
void RzxPlugInLoader::loadPlugIn(QDir sourceDir)
{
	//vérification de l'existence du rep sourceDir/plugins
	if(!sourceDir.cd("plugins"))
	{
		qDebug(QString("Cannot cd to %1/plugins").arg(sourceDir.canonicalPath()));
		return;
	}
	else qDebug(QString("Exploring %1").arg(sourceDir.canonicalPath()));

	//les plugins doivent avoir un nom de fichier qui contient rzxpi
	//		par exemple librzxpixplo.so ou rzxpixplo.dll
#ifdef WIN32 //sous linux c'est librzxpi<nomduplugin>.so
	sourceDir.setNameFilter("rzxpi*");
#else //sous windows c'est rzxpi<nomduplugin>.dll
#ifdef Q_OS_MACX
	sourceDir.setNameFilter("librzxpi*.dylib");
#else
	sourceDir.setNameFilter("librzxpi*.so");
#endif
#endif
	QStringList trans=sourceDir.entryList(QDir::Files|QDir::Readable);
	qDebug(QString("Found %1 plugins in %2").arg(trans.count()).arg(sourceDir.canonicalPath()));

	//chargement des plugins dans les fichiers
	QVariant *pipath = new QVariant(sourceDir.canonicalPath());
	QVariant *userpath = new QVariant(RzxConfig::globalConfig()->userDir().canonicalPath());
	for(QStringList::Iterator it=trans.begin(); it!=trans.end(); ++it)
	{
		qDebug(QString("Loading plug-in file %1").arg(*it));
		
		//tout plug-in doit avoir une fonction RzxPlugIn *getPlugIn() qui renvoi un plugin
		//à partir duquel on peut traiter.
		QLibrary *lib = new QLibrary(sourceDir.filePath(*it));
		loadPlugInProc getPlugIn = (loadPlugInProc)(lib->resolve("getPlugIn"));
		if(getPlugIn)
		{
			RzxPlugIn *pi = getPlugIn();
			if(pi)
			{
				//chargement du plugin et connexion au programme
				qDebug("Plugin is " + pi->getName() + " (version: " + QString::number(pi->getVersion(), 16) + "/" + QString::number(PLUGIN_VERSION, 16) + ")");
				if(pi->getVersion() > PLUGIN_VERSION || (pi->getVersion() & 0xfffff000) != (PLUGIN_VERSION & 0xfffff000))
				{
					qDebug("Wrong plug-in version number");
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
					qDebug("A plug-in with the same name has already be loaded");
					delete lib;
				}
				else
				{
					pluginFlags |= pi->getFeatures();
					connect(pi, SIGNAL(send(const QString&)), RzxServerListener::object(), SLOT(sendProtocolMsg(const QString&)));
					connect(pi, SIGNAL(queryData(RzxPlugIn::Data, RzxPlugIn*)), this, SLOT(sendQuery(RzxPlugIn::Data, RzxPlugIn*)));
					connect(pi, SIGNAL(requestAction(RzxPlugIn::Action, const QString& )), this, SLOT(action(RzxPlugIn::Action, const QString& )));
					pi->getData(RzxPlugIn::DATA_PLUGINPATH, pipath);
					pi->getData(RzxPlugIn::DATA_USERDIR, userpath);
					pi->setSettings(RzxConfig::globalConfig()->settings);
					plugins.append(pi);
					pluginByName.insert(pi->getName(), pi);
					fileByName.insert(pi->getName(), lib);
					state.append(false);
				}
			}
		}
		else
		{
			RzxMessageBox::information(NULL,
				tr("Unable to load a plug-in"),
				tr("A plug-in file has been found but qRezix can't extract any plug-in from it. Maybe the plug-in file is corrupted or not up-to-date.\n Try to install the last version of this plug-in (file %1).").arg(*it));
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
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		if(state[i]) it->stop();
		delete it;
	}
	fileByName.setAutoDelete(true);
	fileByName.clear();
}

/// retour de l'objet global
/** on le construit s'il n'existe pas */
RzxPlugInLoader *RzxPlugInLoader::global()
{
	if(!object) return new RzxPlugInLoader();
	return object;
}

/// lancement de l'exécution des plug-ins
/** Il est du devoir des programmeurs de plugin de faire des programmes qui utilisent au maximum la programmation asynchrone. En effet, il est inimaginable de concevoir un plug-in qui viendrait bloquer le programme à son lancement */
void RzxPlugInLoader::init()
{
	RzxPlugIn *it;
	int i = 0;
	initialized = true;
	QStringList ignored = RzxConfig::ignoredPluginsList();
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		if(ignored.contains(it->getName()))
			state[i] = false;
		else
		{
			//le userDir sert à donner l'emplacement des données de configuration
			//en particulier, c'est comme ça que les plug-ins peuvent écrirent leurs données
			//dans le qrezixrc du rep .rezix
			if(!state[i]) it->init(RzxConfig::globalConfig()->settings, "127.0.0.1");
			state[i] = true;
		}
	}
}

/// Lancement de l'exécution d'un plug-in particulier
void RzxPlugInLoader::init(const QString& name)
{
	RzxPlugIn *pi = pluginByName[name];
	if(pi)
	{
		int pos = plugins.find(pi);
		if(!state[pos]) pi->init(RzxConfig::globalConfig()->settings, "127.0.0.1");
		if(pos > -1)
			state[pos] = true;
	}
}

/// Arrêt de l'exécution de tous les plug-ins
/** Bien que l'arrêt soit réalisé par le destructeur de ~RzxPlugInLoader cette méthode est là au cas où */
void RzxPlugInLoader::stop()
{
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		if(state[i]) it->stop();
		state[i] = false;
	}
}

/// Arrêt de l'exécution d'un plug-in particulier
void RzxPlugInLoader::stop(const QString& name)
{
	RzxPlugIn *pi = pluginByName[name];
	if(pi)
	{
		int pos = plugins.find(pi);
		if(state[pos]) pi->stop();
		if(pos > -1)
			state[pos] = false;
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
	RzxPlugIn *it;
	for(it = plugins.first() ; it ; it = plugins.next())
		it->setSettings(RzxConfig::globalConfig()->settings);
}

/* mélange avec l'interface */
//Cette partie gère l'ajout des plug-ins à l'interface.
//Ainsi chaque plug-in peut donner un sous-menu pour le menu de la tray icon
//ou pour celui qui aparaît lors du clic droit sur un item du rezal

/// ajout des plug-ins au menu de la trayicon
void RzxPlugInLoader::menuTray(QPopupMenu& menu)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		QPopupMenu *piMenu = it->getTrayMenu();
		if(piMenu && piMenu->count() && state[i])
		{
			QPixmap *icon = it->getIcon();
			if(icon)
			{
				QImage img = icon->convertToImage();
				img = img.smoothScale(16, 16);
				QPixmap tmp;
				tmp.convertFromImage(img);
				QIconSet icons(tmp);
				menu.insertItem(icons, it->getName(), piMenu, 0, 0);
			}
			else
				menu.insertItem(it->getName(), piMenu, 0, 0);
		}
	}
	if(menu.count()) menu.insertSeparator();
}

/// ajout des plug-ins au menu des items du rezal
void RzxPlugInLoader::menuItem(QPopupMenu& menu)
{
	if(!initialized) return;
	int i=0;
	int k=0;

	RzxPlugIn *it;
	for(it = plugins.first() ; it ; it = plugins.next(), k++)
	{
		QPopupMenu *piMenu = it->getItemMenu();
		if(piMenu && piMenu->count() && state[k])
		{
			if(!i) menu.insertSeparator();
			QPixmap *icon = it->getIcon();
			if(icon)
			{
				QImage img = icon->convertToImage();
				img = img.smoothScale(16, 16);
				QPixmap tmp;
				tmp.convertFromImage(img);
				QIconSet icons(tmp);
				menu.insertItem(icons, it->getName(), piMenu);
			}
			else
				menu.insertItem(it->getName(), piMenu);
			i++;
		}
	}
}

/// Génération du menu plug-ins de la fenêtre principale
void RzxPlugInLoader::menuAction(QPopupMenu& menu)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		QPopupMenu *piMenu = it->getActionMenu();
		if(piMenu && piMenu->count() && state[i])
		{
			QPixmap *icon = it->getIcon();
			if(icon)
			{
				QImage img = icon->convertToImage();
				img = img.smoothScale(16, 16);
				QPixmap tmp;
				tmp.convertFromImage(img);
				QIconSet icons(tmp);
				menu.insertItem(icons, it->getName(), piMenu);
			}
			else
				menu.insertItem(it->getName(), piMenu);
		}
	}
}

///Mise à jour du menu plug-in de la fenêtre de chat
void RzxPlugInLoader::menuChat(QPopupMenu& menu)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		QPopupMenu *piMenu = it->getChatMenu();
		if(piMenu && piMenu->count() && state[i])
		{
			QPixmap *icon = it->getIcon();
			if(icon)
			{
				QImage img = icon->convertToImage();
				img = img.smoothScale(16, 16);
				QPixmap tmp;
				tmp.convertFromImage(img);
				QIconSet icons(tmp);
				menu.insertItem(icons, it->getName(), piMenu);
			}
			else
				menu.insertItem(it->getName(), piMenu);
		}
	}
}


/* Gestion de l'affichage des propriétés */
//Chaque plug-in peut avoir une fenêtre de réglage des propriétés
//pour faire des réglages propres aux plug-in.

/// Préparation de la liste des plug-ins
void RzxPlugInLoader::makePropListView(QListView *lv, QToolButton *btnProp, QToolButton *btnReload)
{
	if(!initialized) return;
	if(!lvItems.isEmpty()) return;
	pluginListView = lv;
	pluginGetProp = btnProp;
	RzxPlugIn *it;
	int i = 0;
	
	//Non encore implémenté
	btnReload->setEnabled(false);
	
	//la fenêtre consiste en 1 - le nom du plug-in     2 - la description
	//les deux étant fournis par le plug-in
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		QCheckListItem *lvi = new QCheckListItem(lv, it->getName(), QCheckListItem::CheckBox);
		lvi->setText(1, it->getDescription());
		lvi->setText(2, it->getInternalVersion());
		lvi->setOn(state[i]);
		QPixmap *icon = it->getIcon();
		if(icon)
		{
			QImage img = icon->convertToImage();
			img = img.smoothScale(16, 16);
			QPixmap tmp;
			tmp.convertFromImage(img);
			lvi->setPixmap(0, tmp);
		}
		lvi->setVisible(true);
		lvItems.append(lvi);
	}
	btnProp->setEnabled(false);
	
	//gestion des actions
	connect(lv, SIGNAL(selectionChanged()), this, SLOT(changePlugIn()));
	connect(btnProp, SIGNAL(clicked()), this, SLOT(dispProperties()));
	connect(btnReload, SIGNAL(clicked()), this, SLOT(reload()));
}

/// Mise à jour de l'état des plug-ins à partir du statut de la listview
void RzxPlugInLoader::validPropListView()
{
	RzxPlugIn *it;
	QStringList ignored;
	QCheckListItem *lvi;
	int i = 0;

	for(it = plugins.first() ; it ; it = plugins.next(), i++)
	{
		lvi = (QCheckListItem*)lvItems.at(i);
		if(lvi && lvi->isOn() && !state[i])
			init(lvi->text(0));
		else if(lvi && !lvi->isOn() && state[i])
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
	QListViewItem *lvi = pluginListView->selectedItem();
	selectedPlugIn = lvItems.findRef(lvi);
	if(selectedPlugIn == -1 || !state[selectedPlugIn])
	{
		pluginGetProp->setEnabled(false);
		return;
	}
	pluginGetProp->setEnabled(plugins.at(selectedPlugIn)->hasProp());
	selectedPlugInName = plugins.at(selectedPlugIn)->getName();
}

/// Lancement de la gestion des propriétés
/**La main est juste donnée au plug-in
 * <br>C'est encore une fois donné à la responsabilité du concepteur du plug-in pour que la fonction ne bloque pas l'exécution du programme */
void RzxPlugInLoader::dispProperties()
{
	if(!initialized) return;
	if(selectedPlugIn == -1) return;
	plugins.at(selectedPlugIn)->properties();
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
		case RzxPlugIn::DATA_SERVERFTP: value = new QVariant(RzxConfig::localHost()->getServers() & RzxComputer::SERVER_FTP); break;
		case RzxPlugIn::DATA_SERVERHTTP: value = new QVariant(RzxConfig::localHost()->getServers() & RzxComputer::SERVER_HTTP); break;
		case RzxPlugIn::DATA_SERVERNEWS: value = new QVariant(RzxConfig::localHost()->getServers() & RzxComputer::SERVER_NEWS); break;
		case RzxPlugIn::DATA_SERVERSMB: value = new QVariant(RzxConfig::localHost()->getServers() & RzxComputer::SERVER_SAMBA); break;
		case RzxPlugIn::DATA_DNSNAME: value = new QVariant(RzxConfig::localHost()->getName()); break;
		case RzxPlugIn::DATA_NAME: value = new QVariant(RzxConfig::propLastName()); break;
		case RzxPlugIn::DATA_FIRSTNAME: value = new QVariant(RzxConfig::propName()); break;
		case RzxPlugIn::DATA_SURNAME: value = new QVariant(RzxConfig::propSurname()); break;
		case RzxPlugIn::DATA_IP: value = new QVariant(RzxServerListener::object()->getIP().toString()); break;
		case RzxPlugIn::DATA_WORKSPACE: value = new QVariant(RzxConfig::FTPPath()); break;
		case RzxPlugIn::DATA_DISPFTP: value = new QVariant(RzxConfig::localHost()->getServerFlags() & RzxComputer::FLAG_FTP); break;
		case RzxPlugIn::DATA_DISPHTTP: value = new QVariant(RzxConfig::localHost()->getServerFlags() & RzxComputer::FLAG_HTTP); break;
		case RzxPlugIn::DATA_DISPNEWS: value = new QVariant(RzxConfig::localHost()->getServerFlags() & RzxComputer::FLAG_NEWS); break;
		case RzxPlugIn::DATA_DISPSMB: value = new QVariant(RzxConfig::localHost()->getServerFlags() & RzxComputer::FLAG_SAMBA); break;
		case RzxPlugIn::DATA_LANGUAGE: value = new QVariant(tr("English")); break;
		case RzxPlugIn::DATA_THEME: value = new QVariant(RzxConfig::iconTheme()); break;
		case RzxPlugIn::DATA_AWAY: value = new QVariant(RzxConfig::autoResponder()); break;
		case RzxPlugIn::DATA_ICONFTP: value = new QVariant(*RzxConfig::themedIcon("ftp")); break;
		case RzxPlugIn::DATA_ICONHTTP: value = new QVariant(*RzxConfig::themedIcon("http")); break;
		case RzxPlugIn::DATA_ICONNEWS: value = new QVariant(*RzxConfig::themedIcon("news")); break;
		case RzxPlugIn::DATA_ICONSAMBA: value = new QVariant(*RzxConfig::themedIcon("samba")); break;
		case RzxPlugIn::DATA_ICONNOFTP: value = new QVariant(*RzxConfig::themedIcon("no_ftp")); break;
		case RzxPlugIn::DATA_ICONNOHTTP: value = new QVariant(*RzxConfig::themedIcon("no_http")); break;
		case RzxPlugIn::DATA_ICONNONEWS: value = new QVariant(*RzxConfig::themedIcon("no_news")); break;
		case RzxPlugIn::DATA_ICONNOSAMBA: value = new QVariant(*RzxConfig::themedIcon("no_samba")); break;
		case RzxPlugIn::DATA_ICONSAMEGATEWAY: value = new QVariant(*RzxConfig::themedIcon("same_gateway")); break;
		case RzxPlugIn::DATA_ICONDIFFGATEWAY: value = new QVariant(*RzxConfig::themedIcon("diff_gateway")); break;
		case RzxPlugIn::DATA_ICONJONE: value = new QVariant(*RzxConfig::themedIcon("jone")); break;
		case RzxPlugIn::DATA_ICONORANGE: value = new QVariant(*RzxConfig::themedIcon("orange")); break;
		case RzxPlugIn::DATA_ICONROUJE: value = new QVariant(*RzxConfig::themedIcon("rouje")); break;
		case RzxPlugIn::DATA_ICONUNKOS: value = new QVariant(*RzxConfig::themedIcon("os_0")); break;
		case RzxPlugIn::DATA_ICONUNKOSLARGE: value = new QVariant(*RzxConfig::themedIcon("os_0_large")); break;
		case RzxPlugIn::DATA_ICONWIN: value = new QVariant(*RzxConfig::themedIcon("os_1")); break;
		case RzxPlugIn::DATA_ICONWINLARGE: value = new QVariant(*RzxConfig::themedIcon("os_1_large")); break;
		case RzxPlugIn::DATA_ICONWINNT: value = new QVariant(*RzxConfig::themedIcon("os_2")); break;
		case RzxPlugIn::DATA_ICONWINNTLARGE: value = new QVariant(*RzxConfig::themedIcon("os_2_large")); break;
		case RzxPlugIn::DATA_ICONLINUX: value = new QVariant(*RzxConfig::themedIcon("os_3")); break;
		case RzxPlugIn::DATA_ICONLINUXLARGE: value = new QVariant(*RzxConfig::themedIcon("os_3_large")); break;
		case RzxPlugIn::DATA_ICONMAC: value = new QVariant(*RzxConfig::themedIcon("os_4")); break;
		case RzxPlugIn::DATA_ICONMACLARGE: value = new QVariant(*RzxConfig::themedIcon("os_4_large")); break;
		case RzxPlugIn::DATA_ICONMACX: value = new QVariant(*RzxConfig::themedIcon("os_5")); break;
		case RzxPlugIn::DATA_ICONMACXLARGE: value = new QVariant(*RzxConfig::themedIcon("os_5_large")); break;
		case RzxPlugIn::DATA_ICONSPEAKER: value = new QVariant(*RzxConfig::themedIcon("haut_parleur1")); break;
		case RzxPlugIn::DATA_ICONNOSPEAKER: value = new QVariant(*RzxConfig::themedIcon("haut_parleur2")); break;
		case RzxPlugIn::DATA_ICONPLUGIN: value = new QVariant(*RzxConfig::themedIcon("plugin")); break;
		case RzxPlugIn::DATA_ICONHERE: value = new QVariant(*RzxConfig::themedIcon("here")); break;
		case RzxPlugIn::DATA_ICONAWAY: value = new QVariant(*RzxConfig::themedIcon("away")); break;
		case RzxPlugIn::DATA_ICONCOLUMNS: value = new QVariant(*RzxConfig::themedIcon("column")); break;
		case RzxPlugIn::DATA_ICONPREF: value = new QVariant(*RzxConfig::themedIcon("pref")); break;
		case RzxPlugIn::DATA_ICONSEND: value = new QVariant(*RzxConfig::themedIcon("send")); break;
		case RzxPlugIn::DATA_ICONHIST: value = new QVariant(*RzxConfig::themedIcon("historique")); break;
		case RzxPlugIn::DATA_ICONPROP: value = new QVariant(*RzxConfig::themedIcon("prop")); break;
		case RzxPlugIn::DATA_ICONCHAT: value = new QVariant(*RzxConfig::themedIcon("chat")); break;
		case RzxPlugIn::DATA_ICONSIZE: value = new QVariant(RzxConfig::menuIconSize()); break;
		case RzxPlugIn::DATA_ICONTEXT: value = new QVariant(RzxConfig::menuTextPosition()); break;
		case RzxPlugIn::DATA_ICONFAVORITE: value = new QVariant(*RzxConfig::themedIcon("favorite")); break;
		case RzxPlugIn::DATA_ICONNOTFAVORITE: value = new QVariant(*RzxConfig::themedIcon("not_favorite")); break;
		case RzxPlugIn::DATA_CONNECTEDLIST: value = new QVariant(RzxConnectionLister::global()->getIpList()); break;
		case RzxPlugIn::DATA_ICONOK: value = new QVariant(*RzxConfig::themedIcon("ok")); break;
		case RzxPlugIn::DATA_ICONAPPLY: value = new QVariant(*RzxConfig::themedIcon("apply")); break;
		case RzxPlugIn::DATA_ICONCANCEL: value = new QVariant(*RzxConfig::themedIcon("cancel")); break;
		case RzxPlugIn::DATA_ICONBAN: value = new QVariant(*RzxConfig::themedIcon("ban")); break;
		case RzxPlugIn::DATA_ICONUNBAN: value = new QVariant(*RzxConfig::themedIcon("unban")); break;
		case RzxPlugIn::DATA_ICONQUIT: value = new QVariant(*RzxConfig::themedIcon("quit")); break;
		case RzxPlugIn::DATA_FEATUREDLIST:
			if(plugin)
				value = new QVariant(RzxConnectionLister::global()->getIpList(plugin->getFeatures()));
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
	{
		RzxPlugIn *it;
		int i=0;
		for(it = plugins.first() ; it ; it = plugins.next(), i++)
			if(state[i]) it->getData(data, value);
	}

	delete value;
}

///Exécution des actions demandées par le plugin
/** Permet la demande d'action par le plug-in à qRez */
void RzxPlugInLoader::action(RzxPlugIn::Action action, const QString& param)
{
	if(!initialized) return;
	switch((int)action)
	{
		case RzxPlugIn::ACTION_NONE:
			break;

		case RzxPlugIn::ACTION_CHAT:
			RzxConnectionLister::global()->chatCreate(param);
			break;

		case RzxPlugIn::ACTION_CLOSE_CHAT:
			RzxConnectionLister::global()->closeChat(param);
			break;
			
		case RzxPlugIn::ACTION_FTP:
			RzxUtilsLauncher::ftp(param);
			break;
			
		case RzxPlugIn::ACTION_HTTP:
			RzxUtilsLauncher::http(param);
			break;
			
		case RzxPlugIn::ACTION_NEWS:
			RzxUtilsLauncher::news(param);
			break;
			
		case RzxPlugIn::ACTION_SMB:
			RzxUtilsLauncher::samba(param);
			break;
			
		case RzxPlugIn::ACTION_MINIMIZE:
			break;
		
		case RzxPlugIn::ACTION_QUIT:
			QRezix::global()->closeByTray();
			break;
	}
}

/// Envoi des certifications de changement d'état du plug-in
/** On a changé de fenêtre de chat, on l'indique à tout les plug-ins */
void RzxPlugInLoader::chatChanged(QTextEdit *chat)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i=0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_CHAT, (QVariant*)chat);
}

/// On indique que le chat envoie le message
void RzxPlugInLoader::chatSending()
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i=0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_CHATEMIT, NULL);
}

/// On indique que le chat reçoit un message
void RzxPlugInLoader::chatReceived(QString *chat)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i=0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_CHATRECEIVE, (QVariant*)chat);
}

/// On indique que le chat a envoyé un plug-in
void RzxPlugInLoader::chatEmitted(QString *chat)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i=0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_CHATEMITTED, (QVariant*)chat);
}

///On indique qu'un nouvel item vient d'être sélectionné
void RzxPlugInLoader::itemChanged(QListViewItem *item)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_ITEMSELECTED, (QVariant*)item);
}

///On indique que l'item sélectionné chez les favoris à changé
void RzxPlugInLoader::favoriteChanged(QListViewItem *item)
{
	if(!initialized) return;
	RzxPlugIn *it;
	int i = 0;
	for(it = plugins.first() ; it ; it = plugins.next(), i++)
		if(state[i]) it->getData(RzxPlugIn::DATA_FAVORITESELECTED, (QVariant*)item);
}

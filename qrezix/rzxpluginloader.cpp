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
#include <qvaluelist.h>
#include <qlistview.h>
#include <qlibrary.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdir.h>
#include <qvariant.h>

#include "rzxplugin.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxserverlistener.h"
#include "rzxpluginloader.h"
#include "rzxhostaddress.h"

/** Cette classe contient l'ensemble des fonctions pour créer une interface entre
le programme principal et ses plugins. Les plug-ins communiquent avec le programme
par envoi de signaux interceptés par cette classe et alors traités ici.
Le programme communique avec les plug-ins par l'appel des fonctions adaptés de cette
classe dès que nécessaire. Ainsi, lorsque certaines données partagées doivent être
modifiées (comme le nom dns, l'état des servers...) il faut alors le notifier aux
plugins. Il advient donc aux programmeurs de faire appel à cette interface */

//l'object global... normalement seul objet de cette classe lors de l'exécution
RzxPlugInLoader *RzxPlugInLoader::object = NULL;

/* la construction du programme se fait simplement par recherche et chargement des
plugins existant. Le chargement et le lancement de l'exécution des plug-ins se fait de
manière distincte. On peut donc envisager la désactivation des plugs-ins par le programme
principale */
RzxPlugInLoader::RzxPlugInLoader() : QObject(0, 0)
{
	//on charge les plug-ins dans les rep /usr/share/qrezix/plugins
	loadPlugIn(RzxConfig::globalConfig()->systemDir());
	//et $HOME/.rezix/plugins
	loadPlugIn(RzxConfig::globalConfig()->userDir());
	
	object = this;
}

/** Recherche des plugins et chargement dans un répertoire */
//le chargement se fait en fait dans sourceDir/plugins
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
	sourceDir.setNameFilter("*rzxpi*");
	QStringList trans=sourceDir.entryList(QDir::Files|QDir::Readable|QDir::Executable);
	qDebug(QString("Found %1 plugins in %2").arg(trans.count()).arg(sourceDir.canonicalPath()));

	//chargement des plugins dans les fichiers
	for(QStringList::Iterator it=trans.begin(); it!=trans.end(); ++it)
	{
		qDebug(QString("Loading plug-in file %1").arg(*it));
		
		//tout plug-in doit avoir une fonction RzxPlugIn *getPlugIn() qui renvoi un plugin
		//à partir duquel on peut traiter.
		loadPlugInProc getPlugIn = (loadPlugInProc)QLibrary::resolve(sourceDir.filePath((*it)), "_Z9getPlugInv");
		if(getPlugIn)
		{
			RzxPlugIn *pi = getPlugIn();
			if(pi)
			{
				//chargement du plugin et connexion au programme
				qDebug("Plugin is " + pi->getName());
				connect(pi, SIGNAL(send(const QString&)), RzxServerListener::object(), SLOT(sendProtocolMsg(const QString&)));
				connect(pi, SIGNAL(queryData(RzxPlugIn::Data, RzxPlugIn*)), this, SLOT(sendQuery(RzxPlugIn::Data, RzxPlugIn*)));
				plugins << pi;
			}
		}
	}
	
}

/** Fermeture et destruction des plug-ins */
//la fermeture des plugins doit se faire dans les règles.
//en particulier parce que l'utilisation de QSettings nécessite une destruction
//propre pour permettre l'enregistrement des données
RzxPlugInLoader::~RzxPlugInLoader()
{
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
	{
		(*it)->stop();
		delete *it;
	}
}

/** retour de l'objet global */
//on le construit s'il n'existe pas
RzxPlugInLoader *RzxPlugInLoader::global()
{
	if(!object) return new RzxPlugInLoader();
	return object;
}

/** lancement de l'exécution des plug-ins */
//Il est du devoir des programmeurs de plugin de faire des programmes qui utilisent au maximum
//la programmation asynchrone. En effet, il est inimaginable de concevoir un plug-in qui
//viendrait bloquer le programme à son lancement
void RzxPlugInLoader::init()
{
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
		//le userDir sert à donner l'emplacement des données de configuration
		//en particulier, c'est comme ça que les plug-ins peuvent écrirent leurs données
		//dans le qrezixrc du rep .rezix
		(*it)->init(RzxConfig::globalConfig()->userDir().canonicalPath(), "127.0.0.1");
}

/** arrêt de l'exécution de tous les plug-ins */
//Bien que l'arrêt soit réalisé par le destructeur de ~RzxPlugInLoader
//cette méthode est là au cas où
void RzxPlugInLoader::stop()
{
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
		(*it)->stop();
}

/** mélange avec l'interface */
//Cette partie gère l'ajout des plug-ins à l'interface.
//Ainsi chaque plug-in peut donner un sous-menu pour le menu de la tray icon
//ou pour celui qui aparaît lors du clic droit sur un item du rezal

/* ajout des plug-ins au menu de la trayicon */
void RzxPlugInLoader::menuTray(QPopupMenu& menu)
{
	int i=0;

	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
	{
		QPopupMenu *piMenu = (*it)->getTrayMenu();
		if(piMenu)
		{
			if(!i) menu.insertSeparator(0);
			menu.insertItem((*it)->getName(), piMenu, 0, 0);
			i++;
		}
	}
}

/* ajout des plug-ins au menu des items du rezal */
void RzxPlugInLoader::menuItem(QPopupMenu& menu)
{
	int i=0;

	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
	{
		QPopupMenu *piMenu = (*it)->getItemMenu();
		if(piMenu)
		{
			if(!i) menu.insertSeparator();
			menu.insertItem((*it)->getName(), piMenu);
			i++;
		}
	}
}

//Cette fonctin n'a pas pour l'instant d'utilité
//elle sert à créer un menu de lancement de plug-ins
//qui permettrait de lancer des applications utilisateurs
//fournies par le plug-in
//comme par exemple une minimule pour l'xplo
QStringList RzxPlugInLoader::menuAction()
{
	QStringList actionMenu;
	
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
	{
		QStringList *piMenu = (*it)->getActionMenu();
		if(piMenu)
			actionMenu += *piMenu;
	}
	return actionMenu;
}

/** Gestion de l'affichage des propriétés */
//Chaque plug-in peut avoir une fenêtre de réglage des propriétés
//pour faire des réglages propres aux plug-in.

/** Préparation de la liste des plug-ins */
void RzxPlugInLoader::makePropListView(QListView *lv, QPushButton *btn)
{
	if(!lvItems.isEmpty()) return;
	pluginListView = lv;
	pluginGetProp = btn;
	QValueListIterator<RzxPlugIn*> it;
	
	//la fenêtre consiste en 1 - le nom du plug-in     2 - la description
	//les deux étant fournis par le plug-in
	for(it = plugins.begin() ; it != plugins.end() ; it++)
	{
		QListViewItem *lvi = new QListViewItem(lv, (*it)->getName(), (*it)->getDescription());
		lvi->setVisible(true);
		lvItems << lvi;
	}
	btn->setEnabled(false);
	
	//gestion des actions
	connect(lv, SIGNAL(selectionChanged()), this, SLOT(changePlugIn()));
	connect(btn, SIGNAL(clicked()), this, SLOT(dispProperties()));
}

/* Mise à jour de l'état du bouton en fonction
 du plug-in sélectionné */
void RzxPlugInLoader::changePlugIn()
{
	QListViewItem *lvi = pluginListView->selectedItem();
	selectedPlugIn = lvItems.findIndex(lvi);
	if(selectedPlugIn == -1)
	{
		pluginGetProp->setEnabled(false);
		return;
	}
	pluginGetProp->setEnabled((*(plugins.at(selectedPlugIn)))->hasProp());
}

/* Lancement de la gestion des propriétés */
//la main est juste donnée au plug-in
//C'est encore une fois donné à la responsabilité du concepteur
//du plug-in pour que la fonction ne bloque pas l'exécution du programme
void RzxPlugInLoader::dispProperties()
{
	if(selectedPlugIn == -1) return;
	(*(plugins.at(selectedPlugIn)))->properties();
}

/* Envoi de données aux plugins */
//si plugin est NULL, les données seront envoyées à tous le plugins
void RzxPlugInLoader::sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin)
{
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
		case RzxPlugIn::DATA_IP: value = new QVariant(RzxConfig::localHost()->getIP().toString()); break;
		default: return;
	}
	
	//envoie des données
	//au plugin demandeur si il y en a un
	if(plugin) plugin->getData(data, value);
	
	//à tous les plugins si aucun plugin particulier n'a été précisé
	else
	{
		QValueListIterator<RzxPlugIn*> it;
		for(it = plugins.begin() ; it != plugins.end() ; it++)
			(*it)->getData(data, value);
	}
}

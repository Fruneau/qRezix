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

/** Cette classe contient l'ensemble des fonctions pour cr�er une interface entre
le programme principal et ses plugins. Les plug-ins communiquent avec le programme
par envoi de signaux intercept�s par cette classe et alors trait�s ici.
Le programme communique avec les plug-ins par l'appel des fonctions adapt�s de cette
classe d�s que n�cessaire. Ainsi, lorsque certaines donn�es partag�es doivent �tre
modifi�es (comme le nom dns, l'�tat des servers...) il faut alors le notifier aux
plugins. Il advient donc aux programmeurs de faire appel � cette interface */

//l'object global... normalement seul objet de cette classe lors de l'ex�cution
RzxPlugInLoader *RzxPlugInLoader::object = NULL;

/* la construction du programme se fait simplement par recherche et chargement des
plugins existant. Le chargement et le lancement de l'ex�cution des plug-ins se fait de
mani�re distincte. On peut donc envisager la d�sactivation des plugs-ins par le programme
principale */
RzxPlugInLoader::RzxPlugInLoader() : QObject(0, 0)
{
	//on charge les plug-ins dans les rep /usr/share/qrezix/plugins
	loadPlugIn(RzxConfig::globalConfig()->systemDir());
	//et $HOME/.rezix/plugins
	loadPlugIn(RzxConfig::globalConfig()->userDir());
	
	object = this;
}

/** Recherche des plugins et chargement dans un r�pertoire */
//le chargement se fait en fait dans sourceDir/plugins
void RzxPlugInLoader::loadPlugIn(QDir sourceDir)
{
	//v�rification de l'existence du rep sourceDir/plugins
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
		//� partir duquel on peut traiter.
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
//la fermeture des plugins doit se faire dans les r�gles.
//en particulier parce que l'utilisation de QSettings n�cessite une destruction
//propre pour permettre l'enregistrement des donn�es
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

/** lancement de l'ex�cution des plug-ins */
//Il est du devoir des programmeurs de plugin de faire des programmes qui utilisent au maximum
//la programmation asynchrone. En effet, il est inimaginable de concevoir un plug-in qui
//viendrait bloquer le programme � son lancement
void RzxPlugInLoader::init()
{
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
		//le userDir sert � donner l'emplacement des donn�es de configuration
		//en particulier, c'est comme �a que les plug-ins peuvent �crirent leurs donn�es
		//dans le qrezixrc du rep .rezix
		(*it)->init(RzxConfig::globalConfig()->userDir().canonicalPath(), "127.0.0.1");
}

/** arr�t de l'ex�cution de tous les plug-ins */
//Bien que l'arr�t soit r�alis� par le destructeur de ~RzxPlugInLoader
//cette m�thode est l� au cas o�
void RzxPlugInLoader::stop()
{
	QValueListIterator<RzxPlugIn*> it;
	for(it = plugins.begin() ; it != plugins.end() ; it++)
		(*it)->stop();
}

/** m�lange avec l'interface */
//Cette partie g�re l'ajout des plug-ins � l'interface.
//Ainsi chaque plug-in peut donner un sous-menu pour le menu de la tray icon
//ou pour celui qui apara�t lors du clic droit sur un item du rezal

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

//Cette fonctin n'a pas pour l'instant d'utilit�
//elle sert � cr�er un menu de lancement de plug-ins
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

/** Gestion de l'affichage des propri�t�s */
//Chaque plug-in peut avoir une fen�tre de r�glage des propri�t�s
//pour faire des r�glages propres aux plug-in.

/** Pr�paration de la liste des plug-ins */
void RzxPlugInLoader::makePropListView(QListView *lv, QPushButton *btn)
{
	if(!lvItems.isEmpty()) return;
	pluginListView = lv;
	pluginGetProp = btn;
	QValueListIterator<RzxPlugIn*> it;
	
	//la fen�tre consiste en 1 - le nom du plug-in     2 - la description
	//les deux �tant fournis par le plug-in
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

/* Mise � jour de l'�tat du bouton en fonction
 du plug-in s�lectionn� */
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

/* Lancement de la gestion des propri�t�s */
//la main est juste donn�e au plug-in
//C'est encore une fois donn� � la responsabilit� du concepteur
//du plug-in pour que la fonction ne bloque pas l'ex�cution du programme
void RzxPlugInLoader::dispProperties()
{
	if(selectedPlugIn == -1) return;
	(*(plugins.at(selectedPlugIn)))->properties();
}

/* Envoi de donn�es aux plugins */
//si plugin est NULL, les donn�es seront envoy�es � tous le plugins
void RzxPlugInLoader::sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin)
{
	//On pr�pare les donn�es en fonction de ce qui nous a �t� demand� d'envoyer
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
	
	//envoie des donn�es
	//au plugin demandeur si il y en a un
	if(plugin) plugin->getData(data, value);
	
	//� tous les plugins si aucun plugin particulier n'a �t� pr�cis�
	else
	{
		QValueListIterator<RzxPlugIn*> it;
		for(it = plugins.begin() ; it != plugins.end() ; it++)
			(*it)->getData(data, value);
	}
}

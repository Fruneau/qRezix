/***************************************************************************
                       RzxPlugInloader.h  -  description
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

#ifndef RzxPlugInLOADER_H
#define RzxPlugInLOADER_H

#include <QMenu>
#include <QList>
#include <QHash>
#include <QObject>
#include <QDir>
#include <QString>

#include "RzxPlugIn.h"

class QTextEdit;
class QToolButton;
class QString;
class QLibrary;
class Q3ListView;
class Q3ListViewItem;
class Q3CheckListItem;

/**

@author Florent Bruneau
*/

typedef RzxPlugIn *(*loadPlugInProc)(void);

/// Gestion de l'interface entre les plug-ins et qRezix
/** Cette classe contient l'ensemble des fonctions pour cr�er une interface entre le programme principal et ses plugins. Les plug-ins communiquent avec le programme par envoi de signaux intercept�s par cette classe et alors trait�s ici. Le programme communique avec les plug-ins par l'appel des fonctions adapt�s de cette classe d�s que n�cessaire. Ainsi, lorsque certaines donn�es partag�es doivent �tre modifi�es (comme le nom dns, l'�tat des servers...) il faut alors le notifier aux plugins. Il advient donc aux programmeurs de faire appel � cette interface */
class RzxPlugInLoader : public QObject
{
	Q_OBJECT

	QHash<QString,QLibrary*> fileByName;
	QHash<QString,RzxPlugIn*> pluginByName;
	QList<RzxPlugIn*> plugins;
	QHash<RzxPlugIn*, Q3CheckListItem*> lvItems;
	QHash<RzxPlugIn*, bool> state;
	RzxPlugIn *selectedPlugin;
	QString selectedPlugInName;
	bool initialized;
	
	unsigned int pluginFlags;

	Q3ListView *pluginListView;
	QToolButton *pluginGetProp;

	static RzxPlugInLoader *object;
	void loadPlugIn(const QDir &sourceDir);

	RzxPlugInLoader();

	public:
		virtual ~RzxPlugInLoader();
		static RzxPlugInLoader *global();

		void init();
		void init(const QString&);
		void stop();
		void stop(const QString&);
		
		void menuTray(QMenu& menu);
		void menuItem(QMenu& menu);
		void menuAction(QMenu& menu);
		void menuChat(QMenu& menu);
		
		void setSettings();
		int getFeatures() const;
		
		void makePropListView(Q3ListView *lv, QToolButton *btnProp, QToolButton *btnReload);
		void validPropListView();

	public slots:
		void chatChanged(QTextEdit *chat);
		void chatSending();
		void chatReceived(QString *chat);
		void chatEmitted(QString *chat);
		void itemChanged(Q3ListViewItem *item);
		void favoriteChanged(Q3ListViewItem *item);
		void sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin);
		void action(RzxPlugIn::Action action, const QString& args);

	protected slots:
		void reload();
		void dispProperties();
		void changePlugIn();
};

///Indique le masque des fonctionnalit�s du plugin
inline int RzxPlugInLoader::getFeatures() const
{
	 return pluginFlags;
}

/// retour de l'objet global
/** on le construit s'il n'existe pas */
inline RzxPlugInLoader *RzxPlugInLoader::global()
{
	if(!object)
		object = new RzxPlugInLoader();
	return object;
}


#endif

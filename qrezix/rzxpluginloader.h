/***************************************************************************
                       rzxpluginloader.h  -  description
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

#ifndef RZXPLUGINLOADER_H
#define RZXPLUGINLOADER_H

#include <qpopupmenu.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qdir.h>
#include <qlistview.h>

#include "rzxplugin.h"


class QTextEdit;
class QToolButton;
class QString;

/**
@author Florent Bruneau
*/

class QTextEdit;

typedef RzxPlugIn *(*loadPlugInProc)(void);

/// Gestion de l'interface entre les plug-ins et qRezix
/** Cette classe contient l'ensemble des fonctions pour créer une interface entre le programme principal et ses plugins. Les plug-ins communiquent avec le programme par envoi de signaux interceptés par cette classe et alors traités ici. Le programme communique avec les plug-ins par l'appel des fonctions adaptés de cette classe dès que nécessaire. Ainsi, lorsque certaines données partagées doivent être modifiées (comme le nom dns, l'état des servers...) il faut alors le notifier aux plugins. Il advient donc aux programmeurs de faire appel à cette interface */
class RzxPlugInLoader : public QObject
{
	Q_OBJECT

	QPtrList<RzxPlugIn> plugins;
	QPtrList<QListViewItem> lvItems;
	int selectedPlugIn;
	bool initialized;
	
	unsigned int pluginFlags;

	QListView *pluginListView;
	QToolButton *pluginGetProp;

	static RzxPlugInLoader *object;
	void loadPlugIn(QDir sourceDir);

	public:
		RzxPlugInLoader();
		virtual ~RzxPlugInLoader();
		static RzxPlugInLoader *global();

		void init();
		void stop();

		void menuTray(QPopupMenu& menu);
		void menuItem(QPopupMenu& menu);
		void menuAction(QPopupMenu& menu);
		void menuChat(QPopupMenu& menu);
		
		void setSettings();
		inline int getFeatures() { return pluginFlags; };
		
		void makePropListView(QListView *lv, QToolButton *btn);

	public slots:
		void chatChanged(QTextEdit *chat);
		void chatSending();
		void chatReceived(QString *chat);
		void chatEmitted(QString *chat);
		void itemChanged(QListViewItem *item);
		void favoriteChanged(QListViewItem *item);
		void sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin);
		void action(RzxPlugIn::Action action, const QString& args);

	protected slots:
		void dispProperties();
		void changePlugIn();
};

#endif

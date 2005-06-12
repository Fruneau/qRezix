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

#include <q3popupmenu.h>
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <q3dict.h>
#include <qlibrary.h>
#include <qobject.h>
#include <qdir.h>
#include <q3listview.h>

#include "rzxplugin.h"


class QTextEdit;
class QToolButton;
class QString;

/**
@author Florent Bruneau
*/

class Q3TextEdit;

typedef RzxPlugIn *(*loadPlugInProc)(void);

/// Gestion de l'interface entre les plug-ins et qRezix
/** Cette classe contient l'ensemble des fonctions pour créer une interface entre le programme principal et ses plugins. Les plug-ins communiquent avec le programme par envoi de signaux interceptés par cette classe et alors traités ici. Le programme communique avec les plug-ins par l'appel des fonctions adaptés de cette classe dès que nécessaire. Ainsi, lorsque certaines données partagées doivent être modifiées (comme le nom dns, l'état des servers...) il faut alors le notifier aux plugins. Il advient donc aux programmeurs de faire appel à cette interface */
class RzxPlugInLoader : public QObject
{
	Q_OBJECT

	Q3Dict<QLibrary> fileByName;
	Q3Dict<RzxPlugIn> pluginByName;
	Q3PtrList<RzxPlugIn> plugins;
	Q3PtrList<Q3ListViewItem> lvItems;
	Q3ValueList<bool> state;
	int selectedPlugIn;
	QString selectedPlugInName;
	bool initialized;
	
	unsigned int pluginFlags;

	Q3ListView *pluginListView;
	QToolButton *pluginGetProp;

	static RzxPlugInLoader *object;
	void loadPlugIn(QDir sourceDir);

	public:
		RzxPlugInLoader();
		virtual ~RzxPlugInLoader();
		static RzxPlugInLoader *global();

		void init();
		void init(const QString&);
		void stop();
		void stop(const QString&);
		
		void menuTray(Q3PopupMenu& menu);
		void menuItem(Q3PopupMenu& menu);
		void menuAction(Q3PopupMenu& menu);
		void menuChat(Q3PopupMenu& menu);
		
		void setSettings();
		inline int getFeatures() { return pluginFlags; };
		
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

#endif

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
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qobject.h>

#include "rzxplugin.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxserverlistener.h"

 
typedef RzxPlugIn *(*loadPlugInProc)(void);

class RzxPlugInLoader : public QObject
{
	Q_OBJECT

	QValueList<RzxPlugIn*> plugins;
	QValueList<QListViewItem*> lvItems;
	int selectedPlugIn;

	QListView *pluginListView;
	QPushButton *pluginGetProp;

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
		QStringList menuAction();
		
		void makePropListView(QListView *lv, QPushButton *btn);

	public slots:
		void sendQuery(RzxPlugIn::Data data, RzxPlugIn *plugin);

	protected slots:
		void dispProperties();
		void changePlugIn();
};

#endif

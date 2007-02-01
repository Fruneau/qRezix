/***************************************************************************
                          rzxbob.h  -  description
                             -------------------
    begin                : Sat Jan 27 2007
    copyright            : (C) 2007 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXBOB_H
#define RZXBOB_H

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLBOB_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <QHttp>
#include <QAction>
#include <QAbstractItemView>
#include <Qtimer>

#include <RzxRezal>

// Module affichant l'état du bob dans par une icone dans la barre d'outils
class RzxBob : public QObject, public RzxRezal
{
	Q_OBJECT

	bool bobState;
	QHttp* http;
	QAction* action;
	QTimer timeout;
	int request;

public:
	RzxBob();
	~RzxBob();

	virtual QAbstractItemView *widget();
	virtual QDockWidget::DockWidgetFeatures features() const;
	virtual Qt::DockWidgetAreas allowedAreas() const;
	virtual Qt::DockWidgetArea area() const;
	virtual bool floating() const;
	virtual QAction* toolButton();

private slots:
	void checkState();
	void httpRequestFinished(int, bool);
};

#endif

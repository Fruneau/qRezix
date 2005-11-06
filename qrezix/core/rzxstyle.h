/***************************************************************************
                          rzxstyle  -  description
                             -------------------
    begin                : Sun Nov 6 2005
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
#ifndef RZXSTYLE_H
#define RZXSTYLE_H

#include <QObject>
#include <QList>
#include <QPointer>
#include <QWidget>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

///Gestion des styles
class RzxStyle:public QObject
{
	Q_OBJECT
	RZX_GLOBAL(RzxStyle)

	QList< QPointer<QWidget> > styledWidgets;
	void applyStyle();

	public:
		static void applyStyle(QWidget *);
		static void useStyleOnWindow(QWidget*);
		static bool macMetalStyle();
		static void setMacMetalStyle(bool);

	signals:
		void useMacMetalStyle(bool);
};

#endif

/***************************************************************************
                          rzxrezaldrag  -  description
                               -------------------
    begin                : Sun Dec 4 2005
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
#ifndef RZXREZALDRAG_H
#define RZXREZALDRAG_H

#include <QDrag>
#include <QModelIndex>

class QMouseEvent;
class QDropEvent;
class RzxComputer;

///Fournit un moyen simple de cr�er un QDrag associ� � un RzxComputer
class RzxRezalDrag: public QDrag
{
	Q_OBJECT

	public:
		RzxRezalDrag(QWidget*, RzxComputer*);
		~RzxRezalDrag();

		static RzxRezalDrag *mouseEvent(QWidget*, QMouseEvent*, RzxComputer* = NULL);
		static RzxComputer *dragEvent(QDropEvent*);
		static RzxComputer *dragEvent(QDropEvent*, const QModelIndex&);
		static void dropEvent(QDropEvent*, const QModelIndex&);
};

#endif

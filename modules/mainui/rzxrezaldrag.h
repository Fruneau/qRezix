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

#include "rzxmainuiglobal.h"

class QMouseEvent;
class QDropEvent;
class RzxComputer;

///Fournit un moyen simple de créer un QDrag associé à un RzxComputer
/** Le format utilisé pour le drag&drop ici est compatible avec celui utilisé
 * par RzxComputerListWidget
 */
class RZX_MAINUI_EXPORT RzxRezalDrag: public QDrag
{
	Q_OBJECT

	public:
		RzxRezalDrag(QWidget*, RzxComputer*);
		~RzxRezalDrag();

		static RzxRezalDrag *mouseEvent(QWidget*, QMouseEvent*, RzxComputer* = NULL);
		static QList<RzxComputer*> dragEvent(QDropEvent*);
		static QList<RzxComputer*> dragEvent(QDropEvent*, const QModelIndex&);
		static void dropEvent(QDropEvent*, const QModelIndex&);
};

#endif

/***************************************************************************
                          rzxrezalpopup  -  description
                             -------------------
    begin                : Wed Jul 20 2005
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
#ifndef RZXREZALPOPUP_H
#define RZXREZALPOPUP_H

#include <QMenu>
#include <QPoint>
#include <QModelIndex>

/**
@author Florent Bruneau
*/

class QKeyEvent;
class QMenuBar;
class RzxComputer;

///Popup simple qui intercepte le clavier
/** Pour réimplanter le clavier et la touche droite, ne mérite pas un .h/.cpp pour lui tt seul */
class Q_DECL_EXPORT RzxRezalPopup : public QMenu
{
	Q_OBJECT

	void init(RzxComputer*, const QPoint&);

	public:
		RzxRezalPopup(const QModelIndex&, QMenuBar*);
		RzxRezalPopup(RzxComputer*, const QPoint&, QWidget *parent = 0);
		RzxRezalPopup(const QModelIndex&, const QPoint&, QWidget *parent = 0);

		void change(const QModelIndex&);

	protected:
		void keyPressEvent(QKeyEvent *e);
};

#endif

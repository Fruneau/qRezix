/***************************************************************************
                         rzxrezalindex  -  description
                            -------------------
   begin                : Mon Aug 15 2005
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
#ifndef RZXREZALINDEX_H
#define RZXREZALINDEX_H

#include <QTreeView>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLINDEX_BUILTIN
#       define RZX_BUILTIN
#else
#       define RZX_PLUGIN
#endif

#include <RzxRezal>

/**
@author Florent Bruneau
*/
class RzxRezalIndex : public QTreeView, public RzxRezal
{
	Q_OBJECT

	public:
		RzxRezalIndex(QWidget *parent = NULL);
		~RzxRezalIndex();

		virtual QAbstractItemView *widget();
		virtual QDockWidget::DockWidgetFeatures features() const;
		virtual Qt::DockWidgetAreas allowedAreas() const;
		virtual Qt::DockWidgetArea area() const;
		virtual bool floating() const;

	protected:
		virtual void mousePressEvent(QMouseEvent*);
		virtual void mouseMoveEvent(QMouseEvent*);
		virtual void dragEnterEvent(QDragEnterEvent*);
		virtual void dragMoveEvent(QDragMoveEvent*);
		virtual void dropEvent(QDropEvent*);
};

#endif

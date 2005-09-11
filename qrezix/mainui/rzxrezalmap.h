/***************************************************************************
                          rzxrezalmap  -  description
                               -------------------
    begin                : Sat Sep 11 2005
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
#ifndef RZXREZALMAP_H
#define RZXREZALMAP_H

#include <QAbstractItemView>

#include "rzxrezal.h"

/**
 @author Florent Bruneau
 */

///Afficheur des items sous forme de plan
/**
 * Ce Rezal offre l'affichage des items sous d'un plan qui permet de situer les
 * personnes sélectionnées.
 */
class RzxRezalMap:public QAbstractItemView, public RzxRezal
{
	Q_OBJECT
	
	public:
		RzxRezalMap(QWidget *parent = 0);
		~RzxRezalMap();
	
		virtual QAbstractItemView *widget();
		virtual QDockWidget::DockWidgetFeatures features() const;
		virtual Qt::DockWidgetAreas allowedAreas() const;
		virtual Qt::DockWidgetArea area() const;
		virtual bool floating() const;
		virtual void updateLayout();
		
		virtual QModelIndex indexAt(const QPoint&) const;
		virtual void scrollTo(const QModelIndex&, ScrollHint hint = EnsureVisible);
		virtual QRect visualRect(const QModelIndex&) const;
		
	protected:
		virtual int horizontalOffset() const;
		virtual int verticalOffset() const;
		virtual bool isIndexHidden(const QModelIndex&) const;
		virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
		virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
		virtual QRegion visualRegionForSelection(const QItemSelection&) const;
};

#endif

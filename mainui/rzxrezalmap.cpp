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
#include "rzxrezalmap.h"

#include "rzxrezalmodel.h"

RZX_REZAL_EXPORT(RzxRezalMap)

///Construction de l'object
RzxRezalMap::RzxRezalMap(QWidget *widget)
:QAbstractItemView(widget), RzxRezal("Platal 1.7.0-svn", "Show an interactive map of the campus")
{
	beginLoading();
	setModel(RzxRezalModel::global());
	endLoading();
}

///Destruction
RzxRezalMap::~RzxRezalMap()
{
	beginClosing();
	endClosing();
}

///Retourne une fenêtre utilisable pour l'affichage.
QAbstractItemView *RzxRezalMap::widget()
{
	return this;
}

///Retourne les caractéristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalMap::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autorisées du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalMap::allowedAreas() const
{
	return Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea;
}

///Retourne la position par défaut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalMap::area() const
{
	return Qt::BottomDockWidgetArea;
}

///Retourne l'état par défaut du rezal
bool RzxRezalMap::floating() const
{
	return false;
}

/** \reimp */
void RzxRezalMap::updateLayout()
{
}

QModelIndex RzxRezalMap::indexAt(const QPoint&) const
{
	return QModelIndex();
}

void RzxRezalMap::scrollTo(const QModelIndex&, ScrollHint)
{
}

QRect RzxRezalMap::visualRect(const QModelIndex&) const
{
	return QRect();
}

int RzxRezalMap::horizontalOffset() const
{
	return 0;
}

int RzxRezalMap::verticalOffset() const
{
	return 0;
}

bool RzxRezalMap::isIndexHidden(const QModelIndex& index) const
{
	return index.isValid();
}

QModelIndex RzxRezalMap::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
	return QModelIndex();
}

void RzxRezalMap::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
}

QRegion RzxRezalMap::visualRegionForSelection(const QItemSelection&) const
{
	return QRegion();
}

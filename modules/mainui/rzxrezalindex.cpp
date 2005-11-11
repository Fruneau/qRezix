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
#include "rzxrezalindex.h"

#include <RzxRezalModel>

RZX_REZAL_EXPORT(RzxRezalIndex)

RzxRezalIndex::RzxRezalIndex(QWidget *parent)
 : QTreeView(parent), RzxRezal("Index 1.7.0-svn", "Index view for easy navigation")
{
	beginLoading();
	setType(TYP_ALL);
	setType(TYP_INDEX);
	setModel(RzxRezalModel::global());
	for(int i = 1 ; i < RzxRezalModel::numColonnes ; i++)
		hideColumn(i);
	resizeColumnToContents(0);
	endLoading();
}

RzxRezalIndex::~RzxRezalIndex()
{
	beginClosing();
	endClosing();
}

///Retourne une fenêtre utilisable pour l'affichage.
QAbstractItemView *RzxRezalIndex::widget()
{
	return this;
}

///Retourne les caractéristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalIndex::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autorisées du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalIndex::allowedAreas() const
{
	return Qt::AllDockWidgetAreas;
}

///Retourne la position par défaut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalIndex::area() const
{
	return Qt::LeftDockWidgetArea;
}

///Retourne l'état par défaut du rezal
bool RzxRezalIndex::floating() const
{
	return false;
}

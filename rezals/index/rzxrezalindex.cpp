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
#define RZX_MODULE_NAME "Index"
#define RZX_MODULE_DESCRIPTION "Index view for easy navigation"
#define RZX_MODULE_ICON RzxThemedIcon("rzlindex")

#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <RzxApplication>
#include <RzxComputer>

#include "rzxrezalindex.h"

#include <RzxRezalModel>
#include <RzxRezalDrag>
#include <RzxRezalPopup>
#include <RzxRezalAction>

RZX_REZAL_EXPORT(RzxRezalIndex)

///Construction
RzxRezalIndex::RzxRezalIndex(QWidget *parent)
 : QTreeView(parent), RzxRezal(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Index view for easy navigation"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(TYP_ALL);
	setType(TYP_INDEX);
	setIcon(RZX_MODULE_ICON);
	setModel(RzxRezalModel::global());
	firstChange = true;
	for(int i = 1 ; i < RzxRezalModel::numColonnes ; i++)
		hideColumn(i);
	resizeColumnToContents(0);
	setAcceptDrops(true);
	setMinimumWidth(150);
	endLoading();
}

///Destruction
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
	return Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea;
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

///Pour que le changement de sélection entraînte toujours la mise à jour de l'index
void RzxRezalIndex::currentChanged(const QModelIndex& current, const QModelIndex&)
{
	if(firstChange)
	{
		emit activated(current);
		firstChange = false;
	}
	if(!RzxRezalModel::global()->isIndex(current) && !RzxRezalModel::global()->isComputer(current))
		expand(current.parent());
}

///Pour affichier le menu contextuel et le drag&drop
/** Ce menu fournit les actions de bases accessibles depuis l'item */
void RzxRezalIndex::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::RightButton)
		new RzxRezalPopup(indexAt(e->pos()), e->globalPos(), this);
	else
	{
		QModelIndex model = indexAt(e->pos());
		if(!model.isValid()) return;
		RzxComputer *computer = model.model()->data(model, Qt::UserRole).value<RzxComputer*>();
		RzxRezalDrag::mouseEvent(this, e, computer);

		QTreeView::mousePressEvent(e);
	}
}

///Pour lancer le client qui va bien en fonction de la position du clic
void RzxRezalIndex::mouseDoubleClickEvent(QMouseEvent *e)
{
	QModelIndex model = indexAt(e->pos());
	if(!model.isValid()) return;
	RzxComputer *computer = model.model()->data(model, Qt::UserRole).value<RzxComputer*>();
	if(!computer) return;

	switch(model.column())
	{
		case RzxRezalModel::ColFTP: computer->ftp(); break;
		case RzxRezalModel::ColHTTP: computer->http(); break;
		case RzxRezalModel::ColSamba: computer->samba(); break;
		case RzxRezalModel::ColNews: computer->news(); break;
		default: RzxRezalAction::run(computer);
	}
}

///Pour le drag du drag&drop
void RzxRezalIndex::mouseMoveEvent(QMouseEvent *e)
{
	RzxRezalDrag::mouseEvent(this, e);
	QTreeView::mouseMoveEvent(e);
}

///Pour le drop du drag&drop
void RzxRezalIndex::dragEnterEvent(QDragEnterEvent *e)
{
	QTreeView::dragEnterEvent(e);
	if(RzxRezalDrag::dragEvent(e))
		e->accept();
}

///Pour le drop du drag&drop
void RzxRezalIndex::dragMoveEvent(QDragMoveEvent *e)
{
	QTreeView::dragMoveEvent(e);
	const QModelIndex model = indexAt(e->pos());
	const QRect rect = visualRect(model);
	if(RzxRezalDrag::dragEvent(e, model))
		e->accept(rect);
	else
		e->ignore(rect);
}

///Pour le drop :)
void RzxRezalIndex::dropEvent(QDropEvent *e)
{
	RzxRezalDrag::dropEvent(e, indexAt(e->pos()));
}

///Pour éviter que les branches soient ouvertes lors de la suppression d'une ligne...
void RzxRezalIndex::rowsRemoved(const QModelIndex& parent, int, int)
{
	if(isExpanded(parent))
	{
		setExpanded(parent, false);
		setExpanded(parent, true);
	}
}

///Pour éviter que les branches soient ouvertes lors de la suppression d'une ligne...
void RzxRezalIndex::rowsAboutToBeRemoved(const QModelIndex&, int, int)
{
}

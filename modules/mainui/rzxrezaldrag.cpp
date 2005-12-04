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
#include <QHash>
#include <QPointer>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>

#include <RzxComputer>
#include <RzxConnectionLister>
#include <RzxRezalModel>

#include <RzxRezalDrag>

///Construction de l'objet
RzxRezalDrag::RzxRezalDrag(QWidget *source, RzxComputer *object)
	:QDrag(source)
{
	if(!object) return;

	QMimeData *mimeData = new QMimeData();
	mimeData->setText(object->name());
	setMimeData(mimeData);
	setPixmap(object->icon());
}

///Destruction...
RzxRezalDrag::~RzxRezalDrag()
{
}

///Génération d'un objet à partir des informations d'un QMouseEvent
/** Permet de mettre en place une méthode générique de création de
 * RzxRezalDrag qui affranchi le dévelopeur de RzxRezal de devoir
 * recréer un code qui est de toute façon toujours quasiment identique
 */
RzxRezalDrag *RzxRezalDrag::mouseEvent(QWidget *source, QMouseEvent *e, RzxComputer *computer)
{
	static QHash<QWidget*, QPoint> beginPoints;
	static QHash<QWidget*, bool> started;
	static QHash<QWidget*, QPointer<RzxComputer> > computers;
	RzxRezalDrag *drag = NULL;
	
	if(e->type() == QEvent::MouseButtonPress && e->button() == Qt::LeftButton)
	{
		started.insert(source, false);
		beginPoints.insert(source, e->pos());
		computers.insert(source, computer);
	}
	else if(e->type() == QEvent::MouseMove && (e->buttons() & Qt::LeftButton) && !started[source])
	{
		if((e->pos() - beginPoints[source]).manhattanLength() < QApplication::startDragDistance())
			return NULL;

		RzxComputer *computer = computers[source];
		if(!computer)
			return NULL;
		drag = new RzxRezalDrag(source, computer);
		drag->start();
	}
	return drag;
}

///Retrouve le RzxComputer draggé si possible
/** Ou retourne NULL en cas d'impossibilité de retrouver l'objet
 */
RzxComputer *RzxRezalDrag::dragEvent(QDropEvent *e)
{
	const QMimeData *mimedata = e->mimeData();
	if(!mimedata->hasText())
		return NULL;
	return RzxConnectionLister::global()->getComputerByName(mimedata->text());
}

///Retrouve le RzxComputer draggé si possible
/** Surcharge de la fonction précédente qui vérifie en même temps
 * que la zône actuelle est accessible pour un drop
 */
RzxComputer *RzxRezalDrag::dragEvent(QDropEvent *e, const QModelIndex& index)
{
	const QModelIndex model = index.parent();

	if(model == RzxRezalModel::global()->favoriteIndex ||
		model == RzxRezalModel::global()->ignoredIndex ||
		model == RzxRezalModel::global()->neutralIndex ||
		model == RzxRezalModel::global()->favoritesGroup)
		return RzxRezalDrag::dragEvent(e);
	return NULL;
}

///Applique le drop de RzxComputer
void RzxRezalDrag::dropEvent(QDropEvent *e, const QModelIndex& index)
{
	const QModelIndex parent = index.parent();
	const QModelIndex favorite = RzxRezalModel::global()->favoriteIndex;
	const QModelIndex neutral = RzxRezalModel::global()->neutralIndex;
	const QModelIndex ignored = RzxRezalModel::global()->ignoredIndex;
	RzxComputer *computer = dragEvent(e);

	if(!computer) return;
	if(parent == favorite || index == favorite)
	{
		e->acceptProposedAction();
		computer->unban();
		computer->addToFavorites();
	}
	if(parent == ignored || index == ignored)
	{
		e->acceptProposedAction();
		computer->removeFromFavorites();
		computer->ban();
	}
	if(parent == neutral || index == neutral)
	{
		e->acceptProposedAction();
		computer->unban();
		computer->removeFromFavorites();
	}
}

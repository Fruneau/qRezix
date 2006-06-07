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
#include <RzxUserGroup>

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
QList<RzxComputer*> RzxRezalDrag::dragEvent(QDropEvent *e)
{
	const QMimeData *mimedata = e->mimeData();
	QList<RzxComputer*> computers;
	if(!mimedata->hasText())
		return computers;
	const QStringList names = mimedata->text().split('|');
	foreach(QString name, names)
	{
		RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
		if(!computer)
			computer = RzxConnectionLister::global()->getComputerByIP(name);
		if(computer)
			computers << computer;
	}
	return computers;
}

///Retrouve le RzxComputer draggé si possible
/** Surcharge de la fonction précédente qui vérifie en même temps
 * que la zône actuelle est accessible pour un drop
 */
QList<RzxComputer*> RzxRezalDrag::dragEvent(QDropEvent *e, const QModelIndex& index)
{
	const QModelIndex model = index.parent();

	if(model.parent() == RzxRezalModel::global()->favoritesGroup[0] ||
		model == RzxRezalModel::global()->favoritesGroup[0] || 
		model == RzxRezalModel::global()->everybodyGroup[0] ||
		index == RzxRezalModel::global()->everybodyGroup[0])
		return RzxRezalDrag::dragEvent(e);
	return QList<RzxComputer*>();
}

///Applique le drop de RzxComputer
void RzxRezalDrag::dropEvent(QDropEvent *e, const QModelIndex& index)
{
	const QModelIndex parent = index.parent();
	const QModelIndex favorite = RzxRezalModel::global()->favoriteIndex[0];
	const QModelIndex ignored = RzxRezalModel::global()->ignoredIndex[0];
	const QModelIndex everybody = RzxRezalModel::global()->everybodyGroup[0];
	const QModelIndex favoritesGroup = RzxRezalModel::global()->favoritesGroup[0];
	QList<RzxComputer*> computers = dragEvent(e);

	foreach(RzxComputer *computer, computers)
	{
		if(!computer) return;
		if(parent == favorite || index == favorite)
		{
			e->acceptProposedAction();
			computer->unban();
			computer->addToFavorites();
		}
		else if(parent == ignored || index == ignored)
		{
			e->acceptProposedAction();
			computer->removeFromFavorites();
			computer->ban();
		}
		else if(parent == everybody || index == everybody)
		{
			e->acceptProposedAction();
			computer->unban();
			computer->removeFromFavorites();
		}
		else if(parent == favoritesGroup || parent.parent() == favoritesGroup)
		{
			const int groupId = RzxRezalModel::global()->groupId(index);
			if(groupId != -1)
			{
				RzxUserGroup::group(groupId)->add(computer);
				computer->emitUpdate();
			}
		}
	}
}

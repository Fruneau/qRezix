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
#include <QKeyEvent>
#include <QString>
#include <QPixmap>
#include <QMenuBar>

#include <RzxComputer>
#include <RzxConfig>
#include <RzxIconCollection>
#include <RzxUserGroup>

#include "rzxrezalpopup.h"

QHash<QAction*, int> RzxRezalPopup::groupIds = QHash<QAction*, int>();
QPointer<RzxComputer> RzxRezalPopup::computer = NULL;

///Construit un RezalPopup attaché à une barre de menu
/** Cette fonction est surtout utilise sous Mac OS
 */
RzxRezalPopup::RzxRezalPopup(const QModelIndex& index, QMenuBar *menu)
{
	if(!index.isValid())
	{
		deleteLater();
		return;
	}
	QVariant value = index.model()->data(index, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
	{
		deleteLater();
		return;
	}
	init(value.value<RzxComputer*>(), QPoint());
	menu->addMenu(this);
}

///Constructeur on ne peut plus simple...
RzxRezalPopup::RzxRezalPopup(RzxComputer *computer, const QPoint& point, QWidget *parent)
	:QMenu(parent)
{
	init(computer, point);
}

///Constructeur à partir d'un ModelIndex
/** La complexité de ce constructeur provient de la conversion du QModelIndex en RzxComputer */
RzxRezalPopup::RzxRezalPopup(const QModelIndex& index, const QPoint& point, QWidget *parent)
	:QMenu(parent)
{
	if(!index.isValid())
	{
		deleteLater();
		return;
	}
	QVariant value = index.model()->data(index, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
	{
		deleteLater();
		return;
	}
	init(value.value<RzxComputer*>(), point);
}

///Initalisation du Popup
/** Le popup contient toutes les interactions envisageables avec l'ordinateur indiqué. */
void RzxRezalPopup::init(RzxComputer *m_computer, const QPoint& point)
{
	computer = m_computer;
	if(!computer)
	{
		deleteLater();
		return;
	}
	if(!point.isNull()) setAttribute(Qt::WA_DeleteOnClose);

	setTitle(computer->name());
	
#define newItem(name, trad, receiver, slot) addAction(RzxIconCollection::getIcon(name), trad, receiver, slot)
	if(computer->isIgnored())
	{
		if(computer->hasSambaServer()) newItem(Rzx::ICON_SAMBA, tr("Samba connect"), computer, SLOT(samba()));
		if(computer->hasFtpServer()) newItem(Rzx::ICON_FTP, tr("FTP connect"), computer, SLOT(ftp()));
		if(computer->hasHttpServer()) newItem(Rzx::ICON_HTTP, tr("browse Web"), computer, SLOT(http()));
		if(computer->hasNewsServer()) newItem(Rzx::ICON_NEWS, tr("read News"), computer, SLOT(news()));
		if(computer->hasEmailAddress()) newItem(Rzx::ICON_MAIL, tr("send a Mail"), computer, SLOT(mail()));
		addSeparator();
		newItem(Rzx::ICON_UNBAN, tr("Remove from ignore list"), computer, SLOT(unban()));
	}
	else
	{
		if(computer->canBeChatted()) newItem(Rzx::ICON_CHAT, tr("begin &Chat"), computer, SLOT(chat()));
		if(computer->hasSambaServer()) newItem(Rzx::ICON_SAMBA, tr("Samba connect"), computer, SLOT(samba()));
		if(computer->hasFtpServer()) newItem(Rzx::ICON_FTP, tr("FTP connect"), computer, SLOT(ftp()));
		if(computer->hasHttpServer()) newItem(Rzx::ICON_HTTP, tr("browse Web"), computer, SLOT(http()));
		if(computer->hasNewsServer()) newItem(Rzx::ICON_NEWS, tr("read News"), computer, SLOT(news()));
		if(computer->hasEmailAddress()) newItem(Rzx::ICON_MAIL, tr("send a Mail"), computer, SLOT(mail()));
		if(!computer->isLocalhost())
		{
			addSeparator();
			newItem(Rzx::ICON_HISTORIQUE, tr("History"), computer, SLOT(history()));
		}
		if(computer->canBeChecked())
			newItem(Rzx::ICON_PROPRIETES, tr("Properties"), computer, SLOT(checkProperties()));
		addSeparator();
		if(computer->isFavorite())
			newItem(Rzx::ICON_NOTFAVORITE, tr("Remove from favorites"), computer, SLOT(removeFromFavorites()));
		else
		{
			newItem(Rzx::ICON_FAVORITE, tr("Add to favorites"), computer, SLOT(addToFavorites()));
			newItem(Rzx::ICON_BAN, tr("Add to ignore list"), computer, SLOT(ban()));
		}
	}

	QMenu *addGroups = new QMenu(tr("Add to a group"));
	QMenu *delGroups = new QMenu(tr("Remove from group"));
	addGroups->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
	delGroups->setIcon(RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE));
	const int groupNumber = RzxUserGroup::groupNumber();
	int add = 0;
	int del = 0;
	for(int i = 0 ; i < groupNumber ; i++)
	{
		const RzxUserGroup *group = RzxUserGroup::group(i);
		QAction *action;
		if(group->contains(computer))
		{
			del++;
			action = delGroups->addAction(group->icon(), group->name());
		}
		else
		{
			add++;
			action = addGroups->addAction(group->icon(), group->name());
		}
		groupIds.insert(action, i);
	}
	connect(addGroups, SIGNAL(triggered(QAction*)), this, SLOT(addToGroup(QAction*)));
	connect(delGroups, SIGNAL(triggered(QAction*)), this, SLOT(removeFromGroup(QAction*)));
	if(add)
		addMenu(addGroups);
	else
		delete addGroups;
	if(del)
		addMenu(delGroups);
	else
		delete delGroups;
	

	if(!point.isNull())
	{
		addSeparator();
		newItem(Rzx::ICON_CANCEL, tr("Cancel"), this, SLOT(close()));
		popup(point);
	}
#undef newItem
}

///Change l'ordinateur concerné
void RzxRezalPopup::change(const QModelIndex& index)
{
	clear();
	if(!index.isValid())
	{
		setTitle("");
		return;
	}
	QVariant value = index.model()->data(index, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
	{
		setTitle("");
		return;
	}
	init(value.value<RzxComputer*>(), QPoint());
}

///Interception des entrées clavier
/** La flèche de gauche sert à fermer le popup */
void RzxRezalPopup::keyPressEvent(QKeyEvent *e)
{
	if(e->key() != Qt::Key_Left)
	{
		QMenu::keyPressEvent(e);
		return;
	}
	close();
}

///Ajout à un groupe
void RzxRezalPopup::addToGroup(QAction *action)
{
	if(!groupIds.keys().contains(action) || !computer) return;

	const int groupId = groupIds[action];
	if(groupId >= RzxUserGroup::groupNumber() || groupId < 0) return;
	RzxUserGroup::group(groupId)->add(computer);
	computer->emitUpdate();
}

///Suppression d'un groupe
void RzxRezalPopup::removeFromGroup(QAction *action)
{
	if(!groupIds.keys().contains(action) || !computer) return;

	const int groupId = groupIds[action];
	if(groupId >= RzxUserGroup::groupNumber() || groupId < 0) return;
	RzxUserGroup::group(groupId)->remove(computer);
	computer->emitUpdate();
}

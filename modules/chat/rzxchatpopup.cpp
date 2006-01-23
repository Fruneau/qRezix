/***************************************************************************
                       rzxchatpopup  -  description
                             -------------------
    begin                : Sat Nov 19 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include <QAbstractButton>
#include <QDesktopWidget>
#include <QApplication>

#include "rzxchatpopup.h"

///Construit un popup
RzxChatPopup::RzxChatPopup(QAbstractButton *btn, QWidget *parent)
#ifdef Q_OS_MAC
	:QFrame(parent, Qt::Drawer)
#else
	:QFrame(parent, Qt::Window | Qt::FramelessWindowHint)
#endif
{
	button = btn;
	setAttribute(Qt::WA_QuitOnClose,false);
	setAttribute(Qt::WA_DeleteOnClose);
	if(!button)
	{
		close();
		return;
	}
	move();
}

///Place le popup sous le bouton qui lui est assigné
void RzxChatPopup::move()
{
#ifndef Q_OS_MAC
	if(!button) return;

	const QSize resolution = QApplication::desktop()->size();
	QPoint point = button->mapToGlobal(button->rect().bottomLeft());
	const bool limitX = point.x() + width() > resolution.width();
	const bool limitY = point.y() + height() > resolution.height();

	if(limitX && !limitY)
		point.setX(resolution.width() - size().width());
	else if(limitY && !limitX)
	{
		point.setX(button->mapToGlobal(button->rect().bottomRight()).x());
		point.setY(resolution.height() - size().height());
	}
	else if(limitY && limitX)
	{
		point.setX(point.x() - width());
		point.setY(point.y() - height());
	}
	QFrame::move(point);
#endif
}

///Corrige le placement suite au changement de la taille
void RzxChatPopup::resizeEvent(QResizeEvent*)
{
	move();
}

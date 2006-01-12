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
	QFrame::move(button->mapToGlobal(button->rect().bottomLeft()));
#endif
}

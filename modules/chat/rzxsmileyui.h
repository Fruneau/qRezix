/***************************************************************************
                          rzxsmileyui.h  -  description
                             -------------------
    begin                : mer nov 16 2005
    copyright            : (C) 2005 by Guillaume Porcher
    email                : pico@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXSMILEYUI_H
#define RZXSMILEYUI_H

#include <QPushButton>
#include <QtDebug>

#include "rzxchatpopup.h"

/**
@author Pico
*/
class RzxSmileyButton : public QPushButton
{
	Q_OBJECT
	public:
		RzxSmileyButton(const QString& test, const QIcon & icon, QWidget * parent = 0 );
	private:
		QString msg;
	signals:
		void clicked(const QString&);
	private slots:
		void wantAdd(){emit clicked(msg);}
};


class RzxSmileyUi : public RzxChatPopup
{
	Q_OBJECT
	public:
		RzxSmileyUi(QAbstractButton *btn, QWidget *parent = 0);
		~RzxSmileyUi();

	signals:
		void clickedSmiley(const QString& msg);
};

#endif

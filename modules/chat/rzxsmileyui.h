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

#include <QFrame>
#include <QPushButton>
#include <QtDebug>

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
		void clicked(QString);
	private slots:
		void wantAdd(){emit clicked(msg);}
};


class RzxSmileyUi : public QFrame
{
	Q_OBJECT
	public:
		RzxSmileyUi(QWidget *parent = 0);
		~RzxSmileyUi();

	signals:
		void clickedSmiley(QString msg);
};

#endif

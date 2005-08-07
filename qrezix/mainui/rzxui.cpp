/***************************************************************************
                                 rzxui.cpp
          Interface du module pour l'linterface principale de qRezix
                             -------------------
    begin                : Sat Aug 6 2005
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../core/rzxglobal.h"

#include "rzxui.h"

#include "qrezix.h"

/** \reimp */
RzxUi::RzxUi()
	:RzxModule("Main UI 1.7.0-svn", QT_TR_NOOP("Main UI for qRezix"))
{
	beginLoading();
	setType(MOD_MAINUI);
	qrezix = QRezix::global();
	endLoading();
}

/** \reimp */
RzxUi::~RzxUi()
{
	beginClosing();
	delete qrezix;
	endClosing();
}

/** \reimp */
bool RzxUi::isInitialised() const
{
	return qrezix->isInitialised();
}

/** \reimp */
void RzxUi::show()
{
	qrezix->show();
}

/** \reimp */
void RzxUi::hide()
{
	qrezix->hide();
}

/** \reimp */
void RzxUi::toggleVisible()
{
	qrezix->toggleVisible();
}

/** \reimp */
QWidget *RzxUi::mainWindow() const
{
	return qrezix;
}

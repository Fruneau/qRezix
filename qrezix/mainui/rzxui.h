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

#ifndef RZXUI_H
#define RZXUI_H

#include "../core/rzxmodule.h"

/**
 @author Florent Bruneau
 */

class QRezix;

///Interface de module pour l'interface principale de qRezix
/** Cette classe existe parce que le double h�ritage par QObjet
 * pose quelques probl�me...
 */
class RzxUi:public RzxModule
{
	QRezix *qrezix;

	public:
		RzxUi();
		~RzxUi();

		virtual bool isInitialised() const;

	public slots:
		virtual void show();
		virtual void hide();
		virtual void toggleVisible();

	public:
		QWidget *mainWindow() const;
};

#endif

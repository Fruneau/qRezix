/***************************************************************************
                               rzxtraywindow.h
         Gestion des fenêtres popups de notification de la trayicon
                             -------------------
    begin                : Tue Nov 16 2004
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

#include <QFrame>
#include <QTimer>

class RzxComputer;
class QEvent;

class RzxTrayWindow: public QFrame
{
	Q_OBJECT
	
	QTimer timer;
	
	public:
		RzxTrayWindow(RzxComputer *computer, bool connected = true, unsigned int time = 5);
		~RzxTrayWindow();
};

/***************************************************************************
                     rzxnotifierconfig.h  -  description
                             -------------------
    begin                : Sat Aug 13 2005
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
#ifndef RZXNOTIFIERCONFIG_H
#define RZXNOTIFIERCONFIG_H

#include <RzxAbstractConfig>

#include "rzxtraywindow.h"

#ifndef Q_WS_X11
#	define WINDOWSTYLE_DEFAULT RzxTrayWindow::Nice
#else
#	define WINDOWSTYLE_DEFAULT RzxTrayWindow::Old
#endif

/**
 @author Florent Bruneau
 */

///Données de configuration du module de notification
class RzxNotifierConfig:public RzxAbstractConfig
{
	RZX_CONFIG(RzxNotifierConfig)

	public:
		RZX_BOOLPROP("showConnection", showConnection, setShowConnection, true)
		RZX_BOOLPROP("beepConnection", beepConnection, setBeepConnection, false)
		RZX_STRINGPROP("beepSound", beepSound, setBeepSound, QString())

		RZX_BOOLPROP("notifyHere", notifyHere, setNotifyHere, true)
		RZX_BOOLPROP("notifyDisconnection", notifyDisconnection, setNotifyDisconnection, true)
		RZX_BOOLPROP("notifyAway", notifyAway, setNotifyAway, true)

		RZX_INTPROP("windowStyle", windowStyle, setWindowStyle, WINDOWSTYLE_DEFAULT)

		RZX_BOOLPROP("whenIAmAway", notNotifyWhenILeave, setNotNotifyWhenILeave, false)
};

#endif

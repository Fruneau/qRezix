/***************************************************************************
                       rzxtrayconfig.h  -  description
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
#ifndef RZXTRAYCONFIG_H
#define RZXTRAYCONFIG_H

#include <RzxAbstractConfig>

/**
 @author Florent Bruneau
 */

///Gestion de données de configuration de la trayicon
class RzxTrayConfig:public RzxAbstractConfig
{
	RZX_CONFIG(RzxTrayConfig)
	Q_ENUMS(QuickAction)
	Q_FLAGS(QuickActions)

	enum QuickAction {
		None = 0,
		Chat = 1,
		Ftp = 2,
		Http = 4,
		Samba = 8,
		News = 16,
		Mail = 32,
		ChatFav = 64
	};
	Q_DECLARE_FLAGS(QuickActions, QuickAction)

	public:
		RZX_INTPROP("traysize", traySize, setTraySize, 22)
		RZX_BOOLPROP("autoscale", autoScale, setAutoScale, true)
		RZX_INTPROP("quickAction", quickActions, setQuickActions, Ftp | Http | Mail)
};

#endif

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

class RzxTrayConfig:public RzxAbstractConfig
{
	RZX_CONFIG(RzxTrayConfig)

	public:
		RZX_INTPROP("traysize", traySize, setTraySize, 22)
};

#endif

/***************************************************************************
                          rzxxnetconfig  -  description
                             -------------------
    begin                : Sun Aug 28 2005
    copyright            : (C) 2005 by Florent Bruneau
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
#ifndef RZXXNETCONFIG_H
#define RZXXNETCONFIG_H

#include <RzxAbstractConfig>

#define DEFAULT_SERVER "xnetserver.eleves.polytechnique.fr"
#define DEFAULT_PORT 5053
#define DEFAULT_RECONNECTION 60000
#define DEFAULT_TIMEOUT 120000

/**
@author Florent Bruneau
*/

///Données de configuration du protocole xNet
class RzxXNetConfig : public RzxAbstractConfig
{
	RZX_CONFIG(RzxXNetConfig)

	public:
		RZX_UINTPROP("reconnection", reconnection, setReconnection, DEFAULT_RECONNECTION)
		RZX_UINTPROP("ping_timeout", pingTimeout, setPingTimeout, DEFAULT_TIMEOUT)
		RZX_INTPROP("server_port", serverPort, setServerPort, DEFAULT_PORT)
		RZX_STRINGPROP("server_name", serverName, setServerName, DEFAULT_SERVER)
		RZX_PROP_DECLARE(QString, pass, setPass, QString::null)
		RZX_PROP_DECLARE(QString, oldPass, setOldPass, QString::null)
};

#endif

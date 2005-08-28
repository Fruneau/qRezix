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
#include "rzxxnetconfig.h"

#include "rzxserverlistener.h"

RZX_CONFIG_INIT(RzxXNetConfig)

///Change le mot de passe de connexion actuel
void RzxXNetConfig::setPass(const QString& passcode)
{
	global() -> setValue(((RzxServerListener*)global()->module)->getServerIP().toString() + "/pass", passcode);
	global() -> setValue("pass", passcode);
	global() -> flush();
}

///Renvoie le password xnet
QString RzxXNetConfig::pass()
{
	QString i = global() -> value(((RzxServerListener*)global()->module)->getServerIP().toString() + "/pass").toString();
	if(i.isNull()) //Pour la compatibilité avec les anciennes formes de stockage sous nux
		i = global() -> value("pass").toString();
	return i;
}

///Change l'ancien mot de passe de connexion
void RzxXNetConfig::setOldPass(const QString& oldPass)
{
	global() -> setValue(((RzxServerListener*)global()->module)->getServerIP().toString() + "/oldpass", oldPass);
	global() -> flush();
}

///Renvoie l'ancien mot de passe xnet
QString RzxXNetConfig::oldPass()
{
	QString i = global() -> value(((RzxServerListener*)global()->module)->getServerIP().toString() + "/oldpass").toString();
	if(i.isEmpty()) i = QString::null;
	return i;
}

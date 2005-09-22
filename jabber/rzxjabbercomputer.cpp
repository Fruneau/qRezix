/***************************************************************************
                          rzxjabbercomputer  -  description
                             -------------------
    begin                : mar sep 20 2005
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
#include "rzxjabbercomputer.h"


RzxJabberComputer::RzxJabberComputer(QString jid, QString name, uint id)
{
	jid_name = jid;
	name_name = name.isEmpty() ? jid : name;
	ip_addr = new QHostAddress("0.0." + QString::number(id / 255) + "." + QString::number(id % 255));
	nbClients = 1;
}




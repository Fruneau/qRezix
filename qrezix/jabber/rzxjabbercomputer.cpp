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


RzxJabberComputer::RzxJabberComputer(QString name, uint id)
{
	QStringList list = name.split("/");
	jid_name = list[0];
	if(list.count() > 0){
	resource_list << list[1];
	}
	uint subnet = id / 255;
	uint nb = id % 255;
	ip_addr = new QHostAddress("0.0." + QString::number(subnet) + "." + QString::number(nb));
	nbClients = 1;
}




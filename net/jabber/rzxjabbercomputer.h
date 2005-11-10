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
#ifndef RZXJABBERCOMPUTER_H
#define RZXJABBERCOMPUTER_H

#include <QHostAddress>
#include <QString>
#include <QStringList>
#include <RzxComputer>

#include "rzxjabberproperty.h"

class RzxJabberComputer
{
	public:
		RzxJabberComputer(){ jid_name = ""; name_name =""; };
		RzxJabberComputer(QString jid, QString name, uint id);
		const QHostAddress &ip() const{return ip_addr;}
		void setIp(QHostAddress ip){ ip_addr = ip; }
		const QString &jid(){return jid_name;}
		void setJid(QString jid){ jid_name=jid;}
		const QString name(){return name_name;}
		void setName(QString name){ name_name=name;}
		RzxJabberProperty* props(){return properties;}
		uint nbClients;

	private:
		QHostAddress ip_addr;
		QString jid_name, name_name;
		RzxJabberProperty* properties;
};

#endif

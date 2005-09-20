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

class RzxJabberComputer{
	public:
		RzxJabberComputer(QString name, uint id);
		QHostAddress ip(){return *ip_addr;}
		uint nbClients;
		bool operator==(const RzxJabberComputer& a){ return a.jid == jid; }
	
	private:
		QHostAddress *ip_addr;
		QString jid;
		
};

#endif

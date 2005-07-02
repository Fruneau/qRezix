/***************************************************************************
                          rzxhostaddress.h  -  description
                             -------------------
    begin                : Sat Jan 26 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXHOSTADDRESS_H
#define RZXHOSTADDRESS_H

#include <qhostaddress.h>

/**
  *@author Sylvain Joyeux
  */

class RzxHostAddress : public QHostAddress  {
public: 
	RzxHostAddress();
	RzxHostAddress(const QHostAddress& host);
	RzxHostAddress(quint32 ip);
	~RzxHostAddress();
	static RzxHostAddress fromRezix(quint32 ip);
	quint32 toRezix() const;

	bool sameGateway(const RzxHostAddress& peer) const;
	operator quint32() const;
};

#endif

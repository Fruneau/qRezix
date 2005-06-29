/***************************************************************************
                          rzxhostaddress.cpp  -  description
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
#include "rzxhostaddress.h"

RzxHostAddress::RzxHostAddress(){
}

RzxHostAddress::RzxHostAddress(quint32 ip)
	: QHostAddress(ip) {
	
}

RzxHostAddress::RzxHostAddress(const QHostAddress& host)
	: QHostAddress(host)
{ }
	
RzxHostAddress::~RzxHostAddress(){
}

/** No descriptions */
RzxHostAddress RzxHostAddress::fromRezix(quint32 tempIP) {
	RzxHostAddress ret;
	quint32 ip;
	ip = 0;
	ip |= (tempIP >> 24) & 0xFF;
	ip |= ((tempIP >> 16) & 0xFF) << 8;
	ip |= ((tempIP >> 8) & 0xFF) << 16;
	ip |= (tempIP & 0xFF) << 24;
	ret.setAddress(ip);
	return ret;
}

bool RzxHostAddress::sameGateway(const RzxHostAddress& peer) const {
	quint32 local4 = ip4Addr(), peer4 = peer.ip4Addr();
	bool retval=((local4 & 0xFFFFFF00) == (peer4 & 0xFFFFFF00));
	//qDebug(QString("Comparison between %2 and %3 returned %1").arg(retval).arg(local4).arg(peer4));
	return retval;
}

/** No descriptions */
quint32 RzxHostAddress::toRezix() const{
	quint32 tempIP = ip4Addr();
	quint32 ip;
	ip = 0;
	ip |= (tempIP >> 24) & 0xFF;
	ip |= ((tempIP >> 16) & 0xFF) << 8;
	ip |= ((tempIP >> 8) & 0xFF) << 16;
	ip |= (tempIP & 0xFF) << 24;
	return ip;
}


/***************************************************************************
                          rzxclientlistener.h  -  description
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

#ifndef RZXCLIENTLISTENER_H
#define RZXCLIENTLISTENER_H

#include <qobject.h>
#include <qsocketdevice.h>
#include "rzxhostaddress.h"

/**
  *@author Sylvain Joyeux
  */

class RzxClientListener : public QObject  {
	Q_OBJECT

	RzxClientListener();
	static RzxClientListener * globalObject;
	
public:
	static RzxClientListener * object();
	
	~RzxClientListener();
	bool listenOnPort(Q_UINT32 port);
	bool isValid( void ) const;
	
	static const char * DCCFormat[];
	enum DCCCommands {
		DCC_PROPQUERY=0,
		DCC_PROPANSWER=1,
		DCC_CHAT=2,
		DCC_ROOMREQUEST=3,
		DCC_ROOMREFUSE=4,
		DCC_ROOMJOIN=5,
		DCC_ROOMJOINED=6,
		DCC_ROOMQUIT=7,
		DCC_ROOMQUITTED=8,
		DCC_ROOMCHAT=9
	};
	
	enum DCCParams {
		DCC_MSGSIZE=516
	};
	
	void sendPropAnswer(const RzxHostAddress& host, const QString& msg);
	void sendPropQuery(const RzxHostAddress& host);
	void close();

public slots:
	void sendChat(const RzxHostAddress& ip, const QString& msg);
	void sendResponder(const RzxHostAddress& ip, const QString& msg);
	void sendProperties(const RzxHostAddress &peer);
	
protected: // Protected attributes
	char * buffer;
	unsigned long bufferSize;
	bool WaitingForProperties;
	QSocketDevice udpSocket;
	bool valid;

protected slots: // Protected slots
	void socketRead(int socket);

protected: // Protected methods
	void parse(const QString& msg);
	void enforceBufferSize( unsigned long size );
	void sendDccChat(const RzxHostAddress& ip, const QString& msg);

signals: // Signals
	void propQuery(const RzxHostAddress& host);
	void propAnswer(const RzxHostAddress& host, const QString& msg);
	void chat(const RzxHostAddress& peer, const QString& msg);
	void chatSent();
};

#endif

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
#include <qsocket.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qstring.h>
#include "rzxhostaddress.h"

/**
  *@author Sylvain Joyeux
  */
  
class RzxChat;
class RzxChatSocket : public QSocket
{
	Q_OBJECT
	friend class RzxChat;
	friend class RzxClientListener;

	RzxChat *chatWindow;
	QTimer timeOut;
	QTime pongTime;
	RzxHostAddress host;
	bool alone;
	QString tmpChat;
	
	static QString DCCFormat[];
	
	protected:
	enum DCCCommands {
		DCC_PROPQUERY=0,
		DCC_PROPANSWER=1,
		DCC_CHAT=2,
		DCC_PING=3,
		DCC_PONG=4,
		DCC_ROOMREQUEST=5,
		DCC_ROOMREFUSE=6,
		DCC_ROOMJOIN=7,
		DCC_ROOMJOINED=8,
		DCC_ROOMQUIT=9,
		DCC_ROOMQUITTED=10,
		DCC_ROOMCHAT=11
	};
	
	public:
		RzxChatSocket();
		RzxChatSocket(const RzxHostAddress& s_host, RzxChat *parent = NULL);
		RzxChatSocket(const RzxHostAddress& s_host, bool s_alone);
		~RzxChatSocket();
		
		void setParent(RzxChat *parent);
		void connectToHost();
		void setSocket(int socket);
		void close();

	public slots:
		void sendPropQuery();
		void sendChat(const QString& msg);
		void sendResponder(const QString& msg);
		void sendProperties();
		void sendPing();
		void sendPong();
		
	protected slots:
		void chatConnexionEtablished();
		void chatConnexionClosed();
		void chatConnexionError(int Error);
		void chatConnexionTimeout();
		int readSocket();

	protected: // Protected attributes
		int parse(const QString& msg);
		void send(const QString& msg);
		void sendDccChat(const QString& msg);

	signals: // Signals
		void propQuery();
		void propAnswer(const RzxHostAddress& host, const QString& msg);
		
		void chat(QSocket *sock, const QString& msg);
		void chat(const QString& msg);
		
		void chatSent();
		void propertiesSent(const RzxHostAddress& host);
		void pongReceived(int time);
		void info(const QString& msg);
		void notify(const QString& msg, bool withHostname = false);
};


class RzxClientListener : public QObject  {
	Q_OBJECT

	QSocketDevice listenSocket;
	bool valid;
	
	static RzxClientListener * globalObject;
	
	public:
		RzxClientListener();
		~RzxClientListener();
	
		static RzxClientListener * object();
	
		bool listenOnPort(Q_UINT32 port);
		bool isValid( void ) const;
		void close();
		void attach(RzxChatSocket* socket);
		
	public slots:
		void checkProperty(const RzxHostAddress& host);

	protected slots: // Protected slots
		void socketRead(int socket);
		
	signals:
		void propAnswer(const RzxHostAddress& host, const QString& msg);
		void propertiesSent(const RzxHostAddress& host);
		void chat(QSocket *sock, const QString& msg);
		void chatSent();
};

#endif

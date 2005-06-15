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

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QString>

#include "rzxhostaddress.h"

/**
  *@author Sylvain Joyeux
  */

class QPoint;
class RzxChat;
class QRezix;

class RzxChatSocket : public QTcpSocket
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
		DCC_TYPING=5,
		DCC_NONE=6
	};
	
	public:
		RzxChatSocket();
		RzxChatSocket(const RzxHostAddress& s_host, RzxChat *parent = NULL);
		RzxChatSocket(const RzxHostAddress& s_host, bool s_alone);
		~RzxChatSocket();
		
		void setParent(RzxChat *parent);
		void connectToHost();
		virtual void setSocketDescriptor(int socket);
		void close();
		virtual bool operator==(const RzxChatSocket *socket) const { return socket && socketDescriptor() == socket->socketDescriptor(); }
		virtual bool operator==(const RzxChatSocket &socket) const { return socketDescriptor() == socket.socketDescriptor(); }
		virtual bool operator!=(const RzxChatSocket *socket) const { return !socket || socketDescriptor() != socket->socketDescriptor(); }
		virtual bool operator!=(const RzxChatSocket &socket) const { return socketDescriptor() != socket.socketDescriptor(); }
		virtual bool isConnected() const { return state() == QAbstractSocket::ConnectedState; }
		
	public slots:
		void sendPropQuery();
		void sendChat(const QString& msg);
		void sendResponder(const QString& msg);
		void sendProperties();
		void sendPing();
		void sendPong();
		void sendTyping(bool state = false);
		
	protected slots:
		void chatConnexionEtablished();
		void chatConnexionClosed();
		void chatConnexionError(SocketError);
		void chatConnexionTimeout();
		int readSocket();

	protected: // Protected attributes
		int parse(const QString& msg);
		void send(const QString& msg);
		void sendDccChat(const QString& msg);
		
	public:
		static QWidget *showProperties(const RzxHostAddress& peer, const QString& msg, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL );
		static QWidget *showHistorique( unsigned long ip, const QString &hostname, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL );


	signals: // Signals
		void propQuery();
		
		void chat(const QString& msg);
		
		void typing(bool state);
		
		void chatSent();
		void propertiesSent(const RzxHostAddress& host);
		void pongReceived(int time);
		void info(const QString& msg);
		void notify(const QString& msg, bool withHostname = false);
};


class RzxClientListener : public QTcpServer  {
	Q_OBJECT

	static RzxClientListener * object;
	
	public:
		RzxClientListener();
		~RzxClientListener() { }
	
		static RzxClientListener *global() {
			if(!object) new RzxClientListener;
			return object;
		}
		bool listen(quint16 port) { return QTcpServer::listen(QHostAddress::Any, port); }
		void attach(RzxChatSocket* socket);
		
	public slots:
		void checkProperty(const RzxHostAddress& host);

	protected slots: // Protected slots
		void socketRead();
		void info(const QString&);
		
	signals:
		void propertiesSent(const RzxHostAddress& host);
		void chatSent();
};

#endif

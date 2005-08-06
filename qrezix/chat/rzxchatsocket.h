/***************************************************************************
                          rzxchatsocket  -  description
                             -------------------
    begin                : Sun Jul 24 2005
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
#ifndef RZXCHATSOCKET_H
#define RZXCHATSOCKET_H

#include <QTcpSocket>
#include <QString>
#include <QTimer>
#include <QTime>

#include "../core/rzxhostaddress.h"

class QPoint;
class RzxComputer;
class RzxChat;
class RzxClientListener;

/**
@author Florent Bruneau
*/

///Gestion d'un socket utilisant le protocole DCC du xNet
/** Gère les communications et s'interface de manière autonome sur un RzxChat
 * en cas de besoin.
 *
 * Cette classe fournit également des utilitaires qui ne sont pas nécessairement
 * bien placés... mais qui en tout cas on un rapport avec les communications.
 */
class RzxChatSocket : public QTcpSocket
{
	Q_OBJECT
	Q_PROPERTY(RzxChat* chat READ chat WRITE setChat)

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
		RzxChatSocket(const RzxHostAddress& shost, RzxChat *parent = NULL);
		RzxChatSocket(const RzxHostAddress& shost, bool salone);
		~RzxChatSocket();
		
		void close();
		void connectToHost();
		RzxChat *chat() const;

		virtual void setSocketDescriptor(int socket);
		virtual bool operator==(const RzxChatSocket *socket) const;
		virtual bool operator==(const RzxChatSocket &socket) const;
		virtual bool operator!=(const RzxChatSocket *socket) const;
		virtual bool operator!=(const RzxChatSocket &socket) const;
		virtual bool isConnected() const;
		
	public slots:
		void sendPropQuery();
		void sendChat(const QString& msg);
		void sendResponder(const QString& msg);
		void sendProperties();
		void sendPing();
		void sendPong();
		void sendTyping(bool state = false);
		void setChat(RzxChat *parent);
		
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

///Retourne la fenêtre de chat associée au socket
inline RzxChat* RzxChatSocket::chat() const
{ return chatWindow; }

///Test l'égalité à un autre socket
inline bool RzxChatSocket::operator==(const RzxChatSocket *socket) const
{ return socket && socketDescriptor() == socket->socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxChatSocket::operator==(const RzxChatSocket &socket) const
{ return socketDescriptor() == socket.socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxChatSocket::operator!=(const RzxChatSocket *socket) const
{ return !socket || socketDescriptor() != socket->socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxChatSocket::operator!=(const RzxChatSocket &socket) const
{ return socketDescriptor() != socket.socketDescriptor(); }

///Indique si la connexion est établie
inline bool RzxChatSocket::isConnected() const
{ return state() == QAbstractSocket::ConnectedState; }

#endif

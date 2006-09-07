/***************************************************************************
                          rzxfilesocket  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 2006 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXFILESOCKET_H
#define RZXFILESOCKET_H

#include <QTcpSocket>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QFile>

#include <RzxHostAddress>
#include "rzxfilewidget.h"

class QPoint; //inutile je pense
class RzxChat;
class RzxComputer;
class RzxClientListener; //voir tout ca
class RzxFileListener;  

/**
@author Guillaume Bandet
*/

/// Socket gérant le transfert de fichier.
/** Gère les transferts de fichier en s'interfaçant dans un RzxChat
 *
 */
class RzxFileSocket : public QTcpSocket
{
	Q_OBJECT

	friend class RzxFileListener;

	enum State {
		STATE_NONE=0,
		STATE_CHECKING=1,
		STATE_CHECKED=2,
		STATE_PROPOSING=3,
		STATE_SENDING=4,
		STATE_RECEIVING=5
	};

	QTimer timeOut;
	QTimer checkTimeOut;
	RzxHostAddress host;
	bool envoi;	// indique s'il s'agit une connexion d'envoi ou de réception
	State fileState;
	bool modeBinaire;
	QFile *file;
	QString nom;
	qint64 taille;
	qint64 octetsEcrits;
	int octetsALire;
	QString tmpMsg;

	static QString fileFormat[];

	protected:
	enum FileCommands {
		FILE_CANCEL=0,
		FILE_CHECK=1,
		FILE_OK=2,
		FILE_NOK=3,
		FILE_PROPOSE=4,
		FILE_ACCEPT=5,
		FILE_REJECT=6,
		FILE_SEND=7,
		FILE_SENDOK=8,
		FILE_END=9
	};
	
	public:
		RzxFileWidget *widget;
		RzxChat *chatWindow;

		RzxFileSocket();
		RzxFileSocket(RzxComputer*);
		~RzxFileSocket();
		
		void close();
		void connectToHost();
		void sendFile(QString);

		virtual void setSocketDescriptor(int socket);
		virtual bool operator==(const RzxFileSocket *socket) const;
		virtual bool operator==(const RzxFileSocket &socket) const;
		virtual bool operator!=(const RzxFileSocket *socket) const;
		virtual bool operator!=(const RzxFileSocket &socket) const;
		virtual bool isConnected() const;
		
		
	protected slots:
		void fileConnexionEtablished();
		void fileConnexionClosed();
		void fileConnexionError(QAbstractSocket::SocketError);
		void fileConnexionTimeout();
		void checkTimeout();
		void readSocket();
		void btnCancel();
		void btnAccept();
		void btnReject();

	protected: // Protected attributes
		void parse(const QString& msg);
		void send(const QString& msg);
		void sendCheck();
		void sendCancel();
		void sendOk();
		void sendNok();
		void sendPropose(QString nom, quint64 taille);
		void sendAccept();
		void sendReject();
		void sendBinary();
		void sendBinaryOk();
		void sendEnd();
		
	public:
		RzxHostAddress peer() const;

	signals: // Signals
		void fileSent();
		void addWidget(RzxFileWidget *widget);
		void removeWidget(RzxFileWidget *widget);

		void info(const QString& msg);
		void notify(const QString& msg, bool withHostname = false);

};

///Test l'égalité à un autre socket
inline bool RzxFileSocket::operator==(const RzxFileSocket *socket) const
{ return socket && socketDescriptor() == socket->socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxFileSocket::operator==(const RzxFileSocket &socket) const
{ return socketDescriptor() == socket.socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxFileSocket::operator!=(const RzxFileSocket *socket) const
{ return !socket || socketDescriptor() != socket->socketDescriptor(); }

///Test l'égalité à un autre socket
inline bool RzxFileSocket::operator!=(const RzxFileSocket &socket) const
{ return socketDescriptor() != socket.socketDescriptor(); }

///Indique si la connexion est établie
inline bool RzxFileSocket::isConnected() const
{ return state() == QAbstractSocket::ConnectedState; }

///Retourne l'adresse de l'host distant
inline RzxHostAddress RzxFileSocket::peer() const
{ return host; }

#endif

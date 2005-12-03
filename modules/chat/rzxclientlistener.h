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

#include <QTcpServer>
#include <QString>
#include <QPointer>
#include <QHash>

#include <RzxHostAddress>

#include "rzxchatsocket.h"

/**
  *@author Sylvain Joyeux
  */

///Ecoute réseau du chat DCC xNet
/** Cette classe du module chat a pour but d'implémenter l'écoute
 * du réseau pour le chat DCC du protocole xNet
 *
 * La partie communication est gérée par \ref RzxChatSocket
 */
class RzxClientListener : public QTcpServer
{
	Q_OBJECT

	QHash< RzxHostAddress, QPointer<RzxChatSocket> > sockets;

	public:
		RzxClientListener();
		~RzxClientListener();

		bool listen(quint16 port);
		void attach(RzxChatSocket* socket);

	public slots:
		void checkProperty(const RzxHostAddress&);
		void sendChatMessage(RzxComputer *, Rzx::ChatMessageType, const QString& = QString());

	protected slots: // Protected slots
		virtual void incomingConnection(int);
		void info(const QString&);
		
	signals:
		void propertiesSent(const RzxHostAddress&);
		void chatSent();
		void haveProperties(RzxComputer*);
};

///Surcharge pour simplifier l'appel
inline bool RzxClientListener::listen(quint16 port)
{
	return QTcpServer::listen(QHostAddress::Any, port);
}

#endif

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

#include <RzxHostAddress>

class RzxChatSocket;

/**
  *@author Sylvain Joyeux
  */

class RzxClientListener : public QTcpServer  {
	Q_OBJECT
	RZX_GLOBAL(RzxClientListener)

	RzxClientListener();
	public:
		~RzxClientListener();

		bool listen(quint16 port);
		void attach(RzxChatSocket* socket);
		
	public slots:
		void checkProperty(const RzxHostAddress&);

	protected slots: // Protected slots
		virtual void incomingConnection(int);
		void info(const QString&);
		
	signals:
		void propertiesSent(const RzxHostAddress&);
		void chatSent();
};

///Surcharge pour simplifier l'appel
inline bool RzxClientListener::listen(quint16 port)
{
	return QTcpServer::listen(QHostAddress::Any, port);
}

#endif

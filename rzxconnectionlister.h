/***************************************************************************
                     rzxconnectionlister.h  -  description
                             -------------------
    begin                : Sat Sep 11 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXCONNECTIONLISTER_H
#define RZXCONNECTIONLISTER_H

#include <qobject.h>
#include <qdict.h>
#include <qstring.h>


class RzxComputer;
class RzxChat;
class RzxServerListener;
class RzxClientListener;
class RzxHostAddress;
class QSocket;

class RzxConnectionLister : public QObject
{
	Q_OBJECT
	
	static RzxConnectionLister *object;
	
	public:
		QDict<RzxComputer> iplist;
		RzxServerListener * server;
		RzxClientListener * client;
		QDict<RzxChat> chats;

		RzxConnectionLister(QObject *parent = NULL, const char *name = NULL);
		~RzxConnectionLister();
		inline static RzxConnectionLister *global() { if(!object) new RzxConnectionLister(NULL, "Lister"); return object; }
		void initConnection();
		void closeSocket();
		
	public slots:
		void login(const QString& ordi);
		void logout(const RzxHostAddress& ip);
		bool isSocketClosed() const;
		void sysmsg(const QString& msg);
		void fatal(const QString& msg);
		void warnProperties(const RzxHostAddress& peer);
	
		void chat(QSocket* socket, const QString& msg);
		RzxChat *chatCreate( const RzxHostAddress& peer );
		void chatDelete(const RzxHostAddress& peerAddress);
		void closeChats();
		
	protected slots:
		void recvIcon(QImage*Icon, const RzxHostAddress&);
		void serverDisconnected();
		void serverConnected();
		
	signals:
		void needIcon(const RzxHostAddress&);
		void login(RzxComputer* computer);
		void logout(const QString& ip);
		void status(const QString& msg, bool fatal);
		void countChange(const QString& newCount);
		void socketClosed();
		void connectionEtablished();
};


#endif

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
#include <qtimer.h>
#include <qstringlist.h>


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
	QTimer delayDisplay;
	QStringList displayWaiter;
	
	public:
		QDict<RzxComputer> iplist;
		QDict<RzxComputer> computerByLogin;
		RzxServerListener * server;
		RzxClientListener * client;
		QDict<RzxChat> chats;
		QDict<RzxChat> chatsByLogin;

		RzxConnectionLister(QObject *parent = NULL, const char *name = NULL);
		~RzxConnectionLister();
		inline static RzxConnectionLister *global() { if(!object) new RzxConnectionLister(NULL, "Lister"); return object; }
		void initConnection();
		void closeSocket();
		
	public slots:
		void login(const QString& ordi = QString::null);
		void logout(const RzxHostAddress& ip);
		QStringList getIpList();
		
		bool isSocketClosed() const;
		
		void sysmsg(const QString& msg);
		void fatal(const QString& msg);
		
		void warnProperties(const RzxHostAddress& peer);
	
		RzxChat *chatCreate( const RzxHostAddress& peer );
		RzxChat *chatCreate( const QString& login);
		void closeChat(const QString& login);
		void chatDelete(const RzxHostAddress& peerAddress);
		void closeChats();
		
	protected slots:
		void recvIcon(QImage*Icon, const RzxHostAddress&);
		void serverDisconnected();
		void serverConnected();
		RzxChat *createChat( RzxComputer *computer);
		
	signals:
		void needIcon(const RzxHostAddress&);
		void login(RzxComputer* computer);
		void logout(const QString& ip);
		void status(const QString& msg, bool fatal);
		void countChange(const QString& newCount);
		void socketClosed();
		void connectionEtablished();
		void loginEnd();
};


#endif

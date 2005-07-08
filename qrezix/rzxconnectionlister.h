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

#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>
#include <QStringList>
#include <QList>

#include "rzxhostaddress.h"

class RzxComputer;
class RzxChat;
class RzxServerListener;
class RzxClientListener;
class QImage;

///RzxConnection lister est la classe centrale de l'architecture du programme
/** Elle est en effet le pivot entre l'architecture r�seau et l'interface graphique.
 * RzxRezal est son pendant graphique.
 */
class RzxConnectionLister : public QObject
{
	Q_OBJECT
	bool initialized;
	
	static RzxConnectionLister *object;

	// Pour le traitement asynchrone (buffered)
	QTimer delayDisplay;
	QList<RzxComputer*> displayWaiter;
	
	// Sockets actifs
	RzxServerListener * server;
	RzxClientListener * client;

	// Index des machines connect�es
	QHash<RzxHostAddress, RzxComputer*> computerByIP;
	QHash<QString, RzxComputer*> computerByLogin;
	QHash<RzxHostAddress, RzxChat*> chatByIP;
	QHash<QString, RzxChat*> chatByLogin;


	public:
		RzxConnectionLister(QObject *parent = NULL);
		~RzxConnectionLister();
		bool isInitialized() const;
		
		static RzxConnectionLister *global();
		
		void initConnection();
		void closeSocket();
		
		// R�cup�ration des donn�es des annuaires
		RzxComputer *getComputerByName(const QString&) const;
		RzxComputer *getComputerByIP(const RzxHostAddress&) const;
		RzxChat *getChatByName(const QString&) const;
		RzxChat *getChatByIP(const RzxHostAddress&) const;
		
	public slots:
		void login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
		void login();
		void logout(const RzxHostAddress& ip);
		QStringList getIpList(unsigned int features = 0);
		
		bool isSocketClosed() const;
		
		void sysmsg(const QString& msg);
		void fatal(const QString& msg);
		
		void warnProperties(const RzxHostAddress&);
	
		RzxChat *chatCreate(const RzxHostAddress&);
		RzxChat *chatCreate(const QString&);
		void closeChat(const QString& login);
		void chatDelete(const RzxHostAddress&);
		void closeChats();
		
	protected slots:
		void recvIcon(QImage*, const RzxHostAddress&);
		void serverDisconnected();
		void serverConnected();
		RzxChat *createChat( RzxComputer *computer);
		
	signals:
		void needIcon(const RzxHostAddress&);
		void login(RzxComputer*);
		void logout(RzxComputer*);
		void status(const QString& msg, bool fatal);
		void countChange(const QString& newCount);
		void socketClosed();
		void connectionEtablished();
		void loginEnd();
};

///Indique si l'object est correctement initialis�
inline bool RzxConnectionLister::isInitialized() const {
	return initialized;
}

///Renvoie l'objet global
/** L'objet est cr�� si n�cessaire */
inline RzxConnectionLister *RzxConnectionLister::global()
{
	if(!object)
		new RzxConnectionLister(NULL);
	return object;
}

///Renvoie l'ordinateur associ� � name
inline RzxComputer *RzxConnectionLister::getComputerByName(const QString& name) const
{
	return computerByLogin[name];
}

///Renvoie l'ordinateur associ� � l'IP
inline RzxComputer *RzxConnectionLister::getComputerByIP(const RzxHostAddress& ip) const
{
	return computerByIP[ip];
}

///Renvoie la fen�tre de chat associ�e � name
inline RzxChat *RzxConnectionLister::getChatByName(const QString& name) const
{
	return chatByLogin[name];
}

///Renvoie la fen�tre de chat associ�e � l'IP
inline RzxChat *RzxConnectionLister::getChatByIP(const RzxHostAddress& ip) const
{
	return chatByIP[ip];
}

#endif

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
#include <QTimer>
#include <QHash>
#include <QString>
#include <QStringList>

#include <RzxGlobal>

#include <RzxHostAddress>

class RzxComputer;
class RzxChat;
class RzxServerListener;
class RzxClientListener;
class QImage;

///RzxConnection lister est la classe centrale de l'architecture du programme
/** Elle est en effet le pivot entre l'architecture réseau et l'interface graphique.
 * RzxRezal est son pendant graphique.
 */
class RzxConnectionLister : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool initialized READ isInitialized)

	bool initialized;
	
	static RzxConnectionLister *object;

	// Pour le traitement asynchrone (buffered)
	QTimer delayDisplay;
	QList<RzxComputer*> displayWaiter;
	
	// Sockets actifs
	RzxServerListener * server;

	// Index des machines connectées
	QHash<RzxHostAddress, RzxComputer*> computerByIP;
	QHash<QString, RzxComputer*> computerByLogin;

	public:
		RzxConnectionLister(QObject *parent = NULL);
		~RzxConnectionLister();
		bool isInitialized() const;
		
		static RzxConnectionLister *global();
		
		void initConnection();
		void closeSocket();
		
		// Récupération des données des annuaires
		RzxComputer *getComputerByName(const QString&) const;
		RzxComputer *getComputerByIP(const RzxHostAddress&) const;
		
	public slots:
		void login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
		void login();
		void logout(const RzxHostAddress& ip);
		QStringList getIpList(Rzx::Capabilities features = Rzx::CAP_NONE);
		
		bool isSocketClosed() const;
		
		void sysmsg(const QString& msg);
		void fatal(const QString& msg);
	
	protected slots:
		void recvIcon(QImage*, const RzxHostAddress&);
		void serverDisconnected();
		void serverConnected();
		
	signals:
		void needIcon(const RzxHostAddress&);
		void login(RzxComputer*);
		void logout(RzxComputer*);
		void update(RzxComputer*);
		void clear();
		void status(const QString& msg, bool fatal);
		void countChange(const QString& newCount);
		void socketClosed();
		void connectionEtablished();
		void loginEnd();

		void wantChat(RzxComputer*);
		void wantProperties(RzxComputer*);
		void wantHistorique(RzxComputer*);
};

///Indique si l'object est correctement initialisé
inline bool RzxConnectionLister::isInitialized() const {
	return initialized;
}

///Renvoie l'objet global
/** L'objet est créé si nécessaire */
inline RzxConnectionLister *RzxConnectionLister::global()
{
	if(!object)
		new RzxConnectionLister(NULL);
	return object;
}

///Renvoie l'ordinateur associé à name
inline RzxComputer *RzxConnectionLister::getComputerByName(const QString& name) const
{
	return computerByLogin[name];
}

///Renvoie l'ordinateur associé à l'IP
inline RzxComputer *RzxConnectionLister::getComputerByIP(const RzxHostAddress& ip) const
{
	return computerByIP[ip];
}

#endif

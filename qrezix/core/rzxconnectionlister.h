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
#include <QPointer>

#include <RzxGlobal>

#include <RzxHostAddress>
#include <RzxBaseLoader>
#include <RzxNetwork>

class RzxComputer;
class QImage;

///RzxConnection lister est la classe centrale de l'architecture du programme
/** Elle est en effet le pivot entre l'architecture r�eau et l'interface graphique.
 * RzxRezal est son pendant graphique.
 */
class RzxConnectionLister : public QObject, public RzxBaseLoader<RzxNetwork>
{
	Q_OBJECT
	Q_PROPERTY(bool initialized READ isInitialized)
	RZX_GLOBAL(RzxConnectionLister)

	bool initialized;
	int connectionNumber;

	// Pour le traitement asynchrone (buffered)
	QTimer delayDisplay;
	QList< QPointer<RzxComputer> > displayWaiter;
	
	// Index des machines connect�s
	QHash<RzxHostAddress, RzxComputer*> computerByIP;
	QHash<QString, RzxComputer*> computerByLogin;

	public:
		RzxConnectionLister(QObject *parent = NULL);
		~RzxConnectionLister();

		bool isInitialized() const;
		bool isDisconnected() const;
		bool isConnected() const;
		
		// R�up�ation des donn�s des annuaires
		RzxComputer *getComputerByName(const QString&) const;
		RzxComputer *getComputerByIP(const RzxHostAddress&) const;
		
	public slots:
		void login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
		void login();
		void logout(const RzxHostAddress& ip);
		QStringList getIpList(Rzx::Capabilities features = Rzx::CAP_NONE);
		
		void start();
		void stop();
		void refresh();
		
		void info(const QString& msg);
		void warning(const QString& msg);
		void fatal(const QString& msg);

	protected:
		virtual void loadBuiltins();
		virtual bool installModule(RzxNetwork*);
	
	protected slots:
		void statusChanged(const QString&);
		void receivedIcon(QImage*, const RzxHostAddress&);
		void newDisconnection(RzxNetwork*);
		void newConnection(RzxNetwork*);
		
	signals:
		void login(RzxComputer*);
		void update(RzxComputer*);
		void logout(RzxComputer*);
		void countChange(const QString& newCount);

		void clear();
		void loginEnd();
		void connectionClosed(RzxNetwork*);
		void connectionEstablished(RzxNetwork*);
		void status(const QString& msg, bool fatal);

		void wantIcon(const RzxHostAddress&);

		void wantChat(RzxComputer*);
		void wantProperties(RzxComputer*);
		void wantHistorique(RzxComputer*);
};

///Indique si on a fini d'enregistrer tous les connect� d'un serveur
/** Lorsqu'on se connecte �un nouveau serveur, on peut dans certains
 * cas recevoir un grand nombre de nouvelle connexion. Celle-ci sont
 * buffer�s, et lorsque toutes ces connexions sont 'assimil�' on 
 * consid�e que les connexions sont en �at d'�re trait� comme des
 * connexions normales et non plus comme un paquet �absorber.
 *
 * Lorsque toutes ces connexions sont assimil�, on consid�e que
 * le RzxConnectionLister est initialized...
 */
inline bool RzxConnectionLister::isInitialized() const
{
	return initialized;
}

///Renvoie l'ordinateur associ��name
inline RzxComputer *RzxConnectionLister::getComputerByName(const QString& name) const
{
	return computerByLogin[name];
}

///Renvoie l'ordinateur associ��l'IP
inline RzxComputer *RzxConnectionLister::getComputerByIP(const RzxHostAddress& ip) const
{
	return computerByIP[ip];
}

#endif

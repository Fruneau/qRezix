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
/** Elle est en effet le pivot entre l'architecture r�seau et l'interface graphique.
 * RzxRezal est son pendant graphique.
 */
class RZX_CORE_EXPORT RzxConnectionLister : public QObject, public RzxBaseLoader<RzxNetwork>
{
	Q_OBJECT
	Q_PROPERTY(bool initialized READ isInitialized)
	Q_PROPERTY(bool connected READ isConnected)
	Q_PROPERTY(bool disconnected READ isDisconnected)
	Q_PROPERTY(int computerNumber READ computerNumber)
	Q_PROPERTY(QList<RzxComputer*> computerList READ computerList)
	RZX_GLOBAL(RzxConnectionLister)

	bool initialized;
	int connectionNumber;

	// Pour le traitement asynchrone (buffered)
	QTimer delayDisplay;
	QList< QPointer<RzxComputer> > displayWaiter;
	
	// Index des machines connect�es
	QHash<RzxHostAddress, RzxComputer*> computerByIP;
	QHash<QString, RzxComputer*> computerByLogin;

	public:
		RzxConnectionLister(QObject *parent = NULL);
		~RzxConnectionLister();

		bool isInitialized() const;
		bool isDisconnected() const;
		bool isConnected() const;
		
		// R�cup�ration des donn�es des annuaires
		RzxComputer *getComputerByName(const QString&) const;
		RzxComputer *getComputerByIP(const RzxHostAddress&) const;
		
		QStringList ipList(Rzx::Capabilities features = Rzx::CAP_NONE) const;
		QList<RzxComputer*> computerList(Rzx::Capabilities features = Rzx::CAP_NONE) const;
		int computerNumber() const;

	public slots:
		void login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
		void login();
		void logout(const RzxHostAddress& ip);
		
		void start();
		void stop();
		void refresh();
		
		void informationMessage(RzxNetwork*, const QString& msg);
		void warningMessage(RzxNetwork*, const QString& msg);
		void fatalMessage(RzxNetwork*, const QString& msg);

	protected:
		virtual void loadBuiltins();
		virtual bool installModule(RzxNetwork*);
		virtual void unloadModule(RzxNetwork*);
		virtual void relinkModules(RzxNetwork* = NULL, RzxNetwork* = NULL);

		void clearComputerFromNetwork(RzxNetwork *);
		void emitCountChange();
	
	protected slots:
		void statusChanged(RzxNetwork*, const QString&);
		void receivedIcon(QImage*, const RzxHostAddress&);
		void receivedIcon(const QPixmap&, const RzxHostAddress&);
		void newDisconnection(RzxNetwork*);
		void newConnection(RzxNetwork*);
		
	signals:
		///Informe qu'un nouvel ordinateur s'est connect�
		void login(RzxComputer*);
		///Informe qu'un ordinateur � �t� mis � jour
		void update(RzxComputer*);
		///Informe qu'un ordinateur s'est d�connect�
		/** C'est � dire que son protocole r�seau atach� ne le g�re plus et non pas
		 * que son �tat passe en Rzx::STATE_DISCONNECTED
		 */
		void logout(RzxComputer*);
		///Informe que le nombre de connect�s � chang�
		/** Sous la forme d'un texte "xxx connect�s" */
		void countChange(const QString& newCount);
		///Informe que le nombre de connect�s � chang�
		void countChange(int);

		///Indique si on est en phase d'initialisation de connexions
		/** Lorsqu'on est en phase d'initialisation, il est probable qu'on
		 * re�oive un grand nombre de connexions en peu de temps...
		 *
		 * La connexion � ce signal permet d'ignorer ce qui ce passe dans une
		 * p�riode de grand nombre de connexions.
		 */
		void initialLoging(bool);
		///Indique qu'une connexion est ferm�e
		void connectionClosed(RzxNetwork*);
		///Indique qu'une nouvelle connexion est enregistr�e		
		void connectionEstablished(RzxNetwork*);
		///Indique que le statut a chang�
		/** Le statut est donn� sous la forme d'un bool�en qui donne l'�tat
		 * global de connexion, et d'un message indiquant les derni�res
		 * information en provenance des RzxNetwork
		 */
		void status(const QString& msg, bool fatal);
		///Indique qu'on a obtenu l'ip locale
		void receiveAddress(const RzxHostAddress&);

		///Ce signal est �mis lorsqu'un ordinateur recherche l'ic�ne associ�e
		void wantIcon(const RzxHostAddress&);

		///Ce signal est �mis lorsqu'on demande un chat avec un ordinateur
		void wantChat(RzxComputer*);
		///Ce signal est �mis lorsquon demande les propri�t�s d'un ordinateur
		void wantProperties(RzxComputer*);
		///Ce signal est �mis lorsqu'on demande l'affichage de l'historique des discussions avec un ordinateur
		void wantHistorique(RzxComputer*);
};

#endif

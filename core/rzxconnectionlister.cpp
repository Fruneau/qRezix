/***************************************************************************
                 rzxconnectionlister.cpp  -  description
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
#include <QImage>
#include <QPixmap>

#include <RzxConnectionLister>

#include <RzxMessageBox>
#include <RzxIconCollection>
#include <RzxHostAddress>
#include <RzxComputer>
#include <RzxConfig>
#include <RzxApplication>

#ifdef RZX_XNET_BUILTIN
#include "../net/xnet/rzxserverlistener.h"
#endif
#ifdef RZX_JABBER_BUILTIN
#include "../net/jabber/rzxjabberprotocole.h"
#endif

RZX_GLOBAL_INIT(RzxConnectionLister)

///Construction du gestionnaire de réseau
/** La construction comprend en particulier le chargement des différents modules
 * réseaux disponibles.
 */
RzxConnectionLister::RzxConnectionLister( QObject *parent)
		: QObject(parent)
{
	Rzx::beginModuleLoading("Connection lister");
	object = this;
	connectionNumber = 0;

	delayDisplay.setSingleShot(true);

	loadModules("net", "rzxnet*", "getNetwork");	
	connect(&delayDisplay, SIGNAL(timeout()), this, SLOT(login()));

	Rzx::endModuleLoading("Connection lister");
}

///Destruction du gestionnaire de réseau
/** Ferme toutes les connexions
 */
RzxConnectionLister::~RzxConnectionLister()
{
	Rzx::beginModuleClosing("Connection lister");
	closeModules();
	qDeleteAll(computerByIP);
	Rzx::endModuleClosing("Connection lister");
	RZX_GLOBAL_CLOSE
}

///Chargement des builtins
void RzxConnectionLister::loadBuiltins()
{
#ifdef RZX_XNET_BUILTIN
	installModule(new RzxServerListener);
#endif
#ifdef RZX_JABBER_BUILTIN
	installModule(new RzxJabberProtocole);
#endif
}

///Installation des modules
bool RzxConnectionLister::installModule(RzxNetwork *network)
{
	if(RzxBaseLoader<RzxNetwork>::installModule(network))
	{
		connect(network, SIGNAL(login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)),
				this, SLOT(login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)) );
		connect(network, SIGNAL( logout( const RzxHostAddress& ) ), this, SLOT( logout( const RzxHostAddress& ) ) );
		connect(network, SIGNAL( receivedIcon( QImage*, const RzxHostAddress& ) ),
				this, SLOT( receivedIcon( QImage*, const RzxHostAddress& )));
		connect(network, SIGNAL( disconnected(RzxNetwork*) ), this, SLOT( newDisconnection() ) );
		connect(network, SIGNAL(status(const QString&)), this, SLOT(statusChanged(const QString&)));
		connect(network, SIGNAL( connected(RzxNetwork*) ), this, SLOT( newConnection(RzxNetwork*) ) );
		connect(network, SIGNAL( connected(RzxNetwork*) ), this, SIGNAL( connectionEstablished(RzxNetwork*)));
		connect(network, SIGNAL( disconnected(RzxNetwork*) ), this, SIGNAL( connectionClosed(RzxNetwork*) ) );
		connect(network, SIGNAL( receiveAddress(const RzxHostAddress& )), RzxComputer::localhost(), SLOT(setIP(const RzxHostAddress& )));
	
		connect(network, SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
		connect(network, SIGNAL(warning(const QString&)), this, SLOT(warning(const QString&)));
		connect(network, SIGNAL(fatal(const QString&)), this, SLOT(fatal(const QString&)));
		connect( this, SIGNAL(wantIcon(const RzxHostAddress&)), network, SLOT(getIcon(const RzxHostAddress&)));
		connect(network, SIGNAL(haveProperties(RzxComputer*)), RzxApplication::instance(), SLOT(relayProperties(RzxComputer*)));

		connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), network, SLOT(refresh()));
		return true;
	}
	return false;
}

///Enregistrement de l'arrivée d'un nouveau client
void RzxConnectionLister::login(RzxNetwork* network, const RzxHostAddress& ip, const QString& name, quint32 options, quint32 version, quint32 stamp, quint32 flags, const QString& comment)
{
	RzxComputer *computer = getComputerByIP(ip);
	if(!computer)
	{
		computer = new RzxComputer(network, ip, name, options, version, stamp, flags, comment);
		computerByIP.insert(ip, computer);
		displayWaiter << computer;
	
		if(!delayDisplay.isActive())
			delayDisplay.start(1);
	}
	else
	{
		computerByLogin.remove(computer->name());
		computer->update(name, options, stamp, flags, comment);
		computerByLogin.insert(name, computer);
		emit update(computer);
	}
}

/** Sert aussi au raffraichissement des données*/
void RzxConnectionLister::login()
{
	if(displayWaiter.isEmpty()) return;
	RzxComputer *computer = displayWaiter.takeFirst();

	//Le RzxComputer peut être nul si la machine s'est déconnecté entre l'enregistrement de sa connexion
	//et le traitement de cette connexion.
	if(computer)
	{
		connect(computer, SIGNAL(needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
		connect(computer, SIGNAL(wantChat(RzxComputer*)), this, SIGNAL(wantChat(RzxComputer* )));
		connect(computer, SIGNAL(wantProperties(RzxComputer*)), this, SIGNAL(wantProperties(RzxComputer* )));
		connect(computer, SIGNAL(wantHistorique(RzxComputer*)), this, SIGNAL(wantHistorique(RzxComputer* )));

		// Recherche si cet ordinateur était déjà présent (refresh ou login)
		QString tempIP = computer->ip().toString();
		computer->login();
		emit login(computer);
	
		//Ajout du nouvel ordinateur dans les qdict
		computerByLogin.insert(computer->name(), computer);
		emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	}
	
	//Pour passer aux suivants
	if(!displayWaiter.isEmpty()) delayDisplay.start(5);
	else
	{
		emit loginEnd();
		initialized = true;
	}
}

///Enregistre la déconnexion d'un client
void RzxConnectionLister::logout( const RzxHostAddress& ip )
{
	RzxComputer *computer = getComputerByIP(ip);
	if(computer)
	{
		computer->logout();
		emit logout(computer);
		computerByIP.remove(ip);
		computerByLogin.remove(computer->name());
		computer->deleteLater();
		emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	}
}


///Retourne la liste des IP des gens connectés
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connectées */
QStringList RzxConnectionLister::getIpList(Rzx::Capabilities features)
{
	QStringList ips;
	foreach(RzxHostAddress key, computerByIP.keys())
		if(!features || computerByIP[key]->can(features))
			ips << key.toString();
	return ips;
}

///Gère la déconnexion d'un RzxNetwork
/** \sa newConnection
 */
void RzxConnectionLister::newDisconnection(RzxNetwork*)
{
	if(connectionNumber > 0)
		connectionNumber--;
	else
		qDebug("Invalid disconnection");
}

///Gère la connexion d'un RzxNetwork
/** Lorsqu'un RzxNetwork établie une connexion avec son serveur
 * (ou tout autre façon assimilable d'être considérable comme
 * actif), il l'indique et est alors noté comme connecté par le
 * RzxConnectionLister
 *
 * \sa newDisconnection
 */
void RzxConnectionLister::newConnection(RzxNetwork* network)
{
	foreach(RzxComputer *computer, displayWaiter)
	{
		if(!computer || (computer->network() == network))
			displayWaiter.removeAll(computer);
	}

	foreach(QString key, computerByLogin.keys())
	{
		RzxComputer *computer = computerByLogin[key];
		if(!computer || computer->network()==network)
			computerByLogin.remove(key);
	}
	
	foreach(RzxHostAddress key, computerByIP.keys())
	{
		RzxComputer *computer = computerByIP[key];
		if(computer || computer->network()==network)
		{
			if(computer) delete computer;
			computerByIP.remove(key);
		}
	}

	if(connectionNumber < moduleList().count())
	{
		connectionNumber++;
		initialized = false;
	}
	else
		qDebug("Invalid connection");
	
	if(connectionNumber == 1)
		emit clear();
}

///Indique si on a au moins une connexion avec un serveur
bool RzxConnectionLister::isConnected() const
{
	bool connected = false;
	foreach(RzxNetwork *network, moduleList())
		connected |= network->isStarted();
	return connected;
}

///Indique si toutes les connections avec les serveurs sont coupées
bool RzxConnectionLister::isDisconnected() const
{
	return !isConnected();
}

///Lance toutes les connexions
void RzxConnectionLister::start()
{
	foreach(RzxNetwork *network, moduleList())
		network->start();
}

///Arrête toutes les connexions
void RzxConnectionLister::stop()
{
	foreach(RzxNetwork *network, moduleList())
	{
		disconnect(network, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
		network->stop();
	}
}

///Indique à tous les modules qu'il faut rafraichir les données
void RzxConnectionLister::refresh()
{
	foreach(RzxNetwork *network, moduleList())
		network->refresh();
}

///Affiche un message d'information
/** Ce message relai ceux des différents modules réseau
 *
 * \sa warning, fatal
 */
void RzxConnectionLister::info( const QString& msg )
{
	// Boîte de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information(NULL, tr( "XNet Server message:" ), msg );
}

///Affiche un avertissement
/** Ce message relai ceux des différents modules réseau
 *
 * \sa info, fatal
 */
void RzxConnectionLister::warning( const QString& msg )
{
	// Boîte de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::warning(NULL, tr( "XNet Server message:" ), msg );
}

///Affiche un message d'erreur
/** Ce message relai ceux des différents modules réseau
 *
 * \sa info, warning
 */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Boîte de dialogue modale
	RzxMessageBox::critical(NULL, tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
}

///Emet un changement de status en mettant à jour le flag qvb
void RzxConnectionLister::statusChanged(const QString& msg)
{
	emit status(msg, isConnected());
}

///Gère le réception d'une icône
/** Enregistre l'icône associée à l'adresse IP indiquée
 */
void RzxConnectionLister::receivedIcon(QImage* icon, const RzxHostAddress& ip)
{
	RzxComputer *computer = getComputerByIP(ip);
	computer->setIcon(RzxIconCollection::global()->setHashedIcon(computer->stamp(), *icon));
}

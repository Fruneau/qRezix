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

///Construction du gestionnaire de r�seau
/** La construction comprend en particulier le chargement des diff�rents modules
 * r�seaux disponibles.
 */
RzxConnectionLister::RzxConnectionLister( QObject *parent)
	: QObject(parent), RzxBaseLoader<RzxNetwork>("net", "rzxnet*", "getNetwork", "getNetworkName", "getNetworkVersion")
{
	Rzx::beginModuleLoading("Connection lister");
	object = this;
	connectionNumber = 0;
	initialized = true;

	delayDisplay.setSingleShot(true);
	setSettings(RzxConfig::global());

	loadModules();	
	connect(&delayDisplay, SIGNAL(timeout()), this, SLOT(login()));

	Rzx::endModuleLoading("Connection lister");
}

///Destruction du gestionnaire de r�seau
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

///Reconstruit les liens entre les modules
/** Dans le cas pr�sent, ceci ne consiste qu'� d�marrer les modules charg�s
 * � chaud
 */
void RzxConnectionLister::relinkModules(RzxNetwork *newNetwork, RzxNetwork *)
{
	if(newNetwork)
		newNetwork->start();
}

///D�charge un module r�seau
void RzxConnectionLister::unloadModule(RzxNetwork *network)
{
	if(!network) return;
	network->stop();
	clearComputerFromNetwork(network);
	RzxBaseLoader<RzxNetwork>::unloadModule(network);
}

///Indique si on a fini d'enregistrer tous les connect�s d'un serveur
/** Lorsqu'on se connecte � un nouveau serveur, on peut dans certains
 * cas recevoir un grand nombre de nouvelle connexion. Celle-ci sont
 * buffer�es, et lorsque toutes ces connexions sont 'assimil�es' on 
 * consid�re que les connexions sont en �tat d'�tre trait�es comme des
 * connexions normales et non plus comme un paquet � absorber.
 *
 * Lorsque toutes ces connexions sont assimil�es, on consid�re que
 * le RzxConnectionLister est initialized...
 */
bool RzxConnectionLister::isInitialized() const
{
	return initialized;
}

///Renvoie l'ordinateur associ� � name
RzxComputer *RzxConnectionLister::getComputerByName(const QString& name) const
{
	return computerByLogin[name];
}

///Renvoie l'ordinateur associ� � l'IP
RzxComputer *RzxConnectionLister::getComputerByIP(const RzxHostAddress& ip) const
{
	return computerByIP[ip];
}

///Enregistrement de l'arriv�e d'un nouveau client
void RzxConnectionLister::login(RzxNetwork* network, const RzxHostAddress& ip, const QString& name, quint32 options, quint32 version, quint32 stamp, quint32 flags, const QString& comment)
{
	RzxComputer *computer = getComputerByIP(ip);
	if(!computer)
	{
		computer = new RzxComputer(network, ip, name, options, version, stamp, flags, comment);
		computerByIP.insert(ip, computer);
		displayWaiter << computer;

		if(displayWaiter.size() > 5 && initialized)
		{
			initialized = false;
			emit initialLoging(true);
		}
	
		if(!delayDisplay.isActive())
			delayDisplay.start(1);
	}
	else
	{
		if(getComputerByName(name))
			computerByLogin.remove(computer->name());
		computer->update(name, options, stamp, flags, comment);
		if(getComputerByName(name))
		{
			computerByLogin.insert(name, computer);
			emit update(computer);
		}
	}
}

/** Sert aussi au raffraichissement des donn�es*/
void RzxConnectionLister::login()
{
	if(displayWaiter.isEmpty()) return;
	RzxComputer *computer = displayWaiter.takeFirst();

	//Le RzxComputer peut �tre nul si la machine s'est d�connect� entre l'enregistrement de sa connexion
	//et le traitement de cette connexion.
	if(computer)
	{
		connect(computer, SIGNAL(needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
		connect(computer, SIGNAL(wantChat(RzxComputer*)), this, SIGNAL(wantChat(RzxComputer* )));
		connect(computer, SIGNAL(wantProperties(RzxComputer*)), this, SIGNAL(wantProperties(RzxComputer* )));
		connect(computer, SIGNAL(wantHistorique(RzxComputer*)), this, SIGNAL(wantHistorique(RzxComputer* )));

		// Recherche si cet ordinateur �tait d�j� pr�sent (refresh ou login)
		QString tempIP = computer->ip().toString();
		computer->login();
		emit login(computer);
	
		//Ajout du nouvel ordinateur dans les qdict
		computerByLogin.insert(computer->name(), computer);
		emitCountChange();
	}
	
	//Pour passer aux suivants
	if(!displayWaiter.isEmpty()) delayDisplay.start(5);
	else
	{
		emit initialLoging(false);
		initialized = true;
	}
}

///Enregistre la d�connexion d'un client
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
		emitCountChange();
	}
}


///Retourne la liste des IP des gens connect�s
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connect�es */
QStringList RzxConnectionLister::ipList(Rzx::Capabilities features) const
{
	QStringList ips;
	foreach(RzxHostAddress key, computerByIP.keys())
		if(!features || computerByIP[key]->can(features))
			ips << key.toString();
	return ips;
}

///Retourne la liste des machines
QList<RzxComputer*> RzxConnectionLister::computerList(Rzx::Capabilities features) const
{
	QList<RzxComputer*> computers;
	foreach(RzxComputer *computer, computerByIP)
		if(computer && (!features || computer->can(features)))
			computers << computer;
	return computers;
}

///Retourne le nombre de machines connect�es
int RzxConnectionLister::computerNumber() const
{
	return computerByIP.count();
}

///G�re la d�connexion d'un RzxNetwork
/** \sa newConnection
 */
void RzxConnectionLister::newDisconnection(RzxNetwork*)
{
	if(connectionNumber > 0)
		connectionNumber--;
	else
		qDebug("Invalid disconnection");
}

///G�re la connexion d'un RzxNetwork
/** Lorsqu'un RzxNetwork �tablie une connexion avec son serveur
 * (ou tout autre fa�on assimilable d'�tre consid�rable comme
 * actif), il l'indique et est alors not� comme connect� par le
 * RzxConnectionLister
 *
 * \sa newDisconnection
 */
void RzxConnectionLister::newConnection(RzxNetwork* network)
{
	clearComputerFromNetwork(network);
	if(connectionNumber < moduleList().count())
		connectionNumber++;
	else
		qDebug("Invalid connection");
	
	if(connectionNumber == 1)
		emit clear();
}

///Retire de la liste des ordinateurs ceux associ� au module r�seau donn�
void RzxConnectionLister::clearComputerFromNetwork(RzxNetwork *network)
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
		if(computer || computer->network() == network)
		{
			emit logout(computer);
			delete computer;
			computerByIP.remove(key);
		}
	}

	emitCountChange();	
}

///Indique si on a au moins une connexion avec un serveur
bool RzxConnectionLister::isConnected() const
{
	bool connected = false;
	foreach(RzxNetwork *network, moduleList())
		connected |= network->isStarted();
	return connected;
}

///Indique si toutes les connections avec les serveurs sont coup�es
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

///Arr�te toutes les connexions
void RzxConnectionLister::stop()
{
	foreach(RzxNetwork *network, moduleList())
	{
		disconnect(network, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
		network->stop();
	}
}

///Indique � tous les modules qu'il faut rafraichir les donn�es
void RzxConnectionLister::refresh()
{
	foreach(RzxNetwork *network, moduleList())
		network->refresh();
}

///Affiche un message d'information
/** Ce message relai ceux des diff�rents modules r�seau
 *
 * \sa warning, fatal
 */
void RzxConnectionLister::info( const QString& msg )
{
	// Bo�te de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information(NULL, tr( "XNet Server message:" ), msg );
}

///Affiche un avertissement
/** Ce message relai ceux des diff�rents modules r�seau
 *
 * \sa info, fatal
 */
void RzxConnectionLister::warning( const QString& msg )
{
	// Bo�te de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::warning(NULL, tr( "XNet Server message:" ), msg );
}

///Affiche un message d'erreur
/** Ce message relai ceux des diff�rents modules r�seau
 *
 * \sa info, warning
 */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Bo�te de dialogue modale
	RzxMessageBox::critical(NULL, tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
}

///Emet un changement de status en mettant � jour le flag qvb
void RzxConnectionLister::statusChanged(const QString& msg)
{
	emit status(msg, isConnected());
}

///Emet des signaux indiquant le changement du nombre de connect�s
void RzxConnectionLister::emitCountChange()
{
	const int cn = computerNumber();
	emit countChange(tr("%1 clients connected").arg(cn));
	emit countChange(cn);
}

///G�re le r�ception d'une ic�ne
/** Enregistre l'ic�ne associ�e � l'adresse IP indiqu�e
 */
void RzxConnectionLister::receivedIcon(QImage* icon, const RzxHostAddress& ip)
{
	RzxComputer *computer = getComputerByIP(ip);
	computer->setIcon(RzxIconCollection::global()->setHashedIcon(computer->stamp(), *icon));
}

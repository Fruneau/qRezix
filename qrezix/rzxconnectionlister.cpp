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

#include "rzxconnectionlister.h"

#include "rzxmessagebox.h"
#include "rzxserverlistener.h"
#include "rzxiconcollection.h"
#include "rzxhostaddress.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"

#define USER_HASH_TABLE_LENGTH 1663


RzxConnectionLister *RzxConnectionLister::object = NULL;

RzxConnectionLister::RzxConnectionLister( QObject *parent)
		: QObject(parent)
{
	Rzx::beginModuleLoading("Connection lister");
	object = this;

	server = RzxServerListener::object();
	delayDisplay.setSingleShot(true);

	connect( server, SIGNAL(login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)),
			 this, SLOT(login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)) );
	connect( server, SIGNAL( logout( const RzxHostAddress& ) ), this, SLOT( logout( const RzxHostAddress& ) ) );
	connect( server, SIGNAL( rcvIcon( QImage*, const RzxHostAddress& ) ), this, SLOT( recvIcon( QImage*, const RzxHostAddress& ) ) );
	connect( server, SIGNAL( disconnected() ), this, SLOT( serverDisconnected() ) );
	connect( server, SIGNAL( status( const QString&, bool ) ), this, SIGNAL( status( const QString&, bool ) ) );
	connect( server, SIGNAL( connected() ), this, SLOT( serverConnected() ) );
	connect( server, SIGNAL( connected() ), this, SIGNAL( connectionEtablished()));
	connect( server, SIGNAL( disconnected() ), this, SIGNAL( socketClosed() ) );
	connect( server, SIGNAL(receiveAddress(const RzxHostAddress& )), RzxComputer::localhost(), SLOT(setIP(const RzxHostAddress& )));
	
	connect(server, SIGNAL(sysmsg(const QString&)), this, SLOT(sysmsg(const QString&)));
	connect(server, SIGNAL(fatal(const QString&)), this, SLOT(fatal(const QString&)));
	connect( this, SIGNAL(needIcon(const RzxHostAddress&)), server, SLOT(getIcon(const RzxHostAddress&)));
	
	connect(&delayDisplay, SIGNAL(timeout()), this, SLOT(login()));

	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), server, SLOT(sendRefresh()));
	Rzx::endModuleLoading("Connection lister");
}

RzxConnectionLister::~RzxConnectionLister()
{
	qDeleteAll(computerByIP);
	if(server)
		server->deleteLater();
	object = NULL;
}

///Enregistrement de l'arrivée d'un nouveau client
void RzxConnectionLister::login(const RzxHostAddress& ip, const QString& name, quint32 options, quint32 version, quint32 stamp, quint32 flags, const QString& comment)
{
	RzxComputer *computer = getComputerByIP(ip);
	if(!computer)
	{
		computer = new RzxComputer(ip, name, options, version, stamp, flags, comment);
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
	connect(computer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
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


void RzxConnectionLister::initConnection()
{
	server -> setupConnection();
}

/** No descriptions */
void RzxConnectionLister::serverDisconnected()
{
}

/** No descriptions */
void RzxConnectionLister::serverConnected()
{
	qDeleteAll(computerByIP);
	computerByIP.clear();
	computerByLogin.clear();
	displayWaiter.clear();
	
	emit clear();
	initialized = false;
}

/** No descriptions */
bool RzxConnectionLister::isSocketClosed() const
{
	return server->isSocketClosed();
}

/** No descriptions */
void RzxConnectionLister::closeSocket()
{
	disconnect(server, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
	if(server) server->close();
}

/** No descriptions */
void RzxConnectionLister::sysmsg( const QString& msg )
{
	// Boîte de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information(NULL, tr( "XNet Server message:" ), msg );
}
/** No descriptions */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Boîte de dialogue modale
	RzxMessageBox::critical(NULL, tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
}

/** No descriptions */
void RzxConnectionLister::recvIcon(QImage* icon, const RzxHostAddress& ip)
{
	RzxComputer *computer = getComputerByIP(ip);
	computer->setIcon(RzxIconCollection::global()->setHashedIcon(computer->stamp(), *icon));
}

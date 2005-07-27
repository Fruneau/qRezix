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
#include "rzxclientlistener.h"
#include "rzxserverlistener.h"
#include "rzxiconcollection.h"
#include "rzxhostaddress.h"
#include "rzxcomputer.h"
#include "rzxchat.h"
#include "rzxconfig.h"

#define USER_HASH_TABLE_LENGTH 1663


RzxConnectionLister *RzxConnectionLister::object = NULL;

RzxConnectionLister::RzxConnectionLister( QObject *parent)
		: QObject(parent)
{
	object = this;

	server = RzxServerListener::object();
	client = RzxClientListener::global();

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
	
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), this, SLOT(warnProperties(const RzxHostAddress&)));
	
	connect(&delayDisplay, SIGNAL(timeout()), this, SLOT(login()));
}

RzxConnectionLister::~RzxConnectionLister()
{
	qDeleteAll(computerByIP);
	qDeleteAll(chatByIP);
	if(server)
		server->deleteLater();
	if(client)
		client->deleteLater();
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
		RzxChat *chat = chatByLogin.take(computer->name());
		computerByLogin.remove(computer->name());
		computer->update(name, options, stamp, flags, comment);
		computerByLogin.insert(name, computer);
		if(chat)
		{
			chatByLogin.insert(name, chat);
			chat->setHostname(name);
		}
		emit update(computer);
	}
}

/** Sert aussi au raffraichissement des données*/
void RzxConnectionLister::login()
{
	if(displayWaiter.isEmpty()) return;
	RzxComputer *computer = displayWaiter.takeFirst();
	connect(computer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );

	// Recherche si cet ordinateur était déjà présent (refresh ou login)
	QString tempIP = computer->ip().toString();

	//mise à jour des données concernant le chat
	RzxChat *chat = chatByLogin.take(computer->name());
	if(chat)
	{
		//Indication au chat de la reconnexion
		if (!computer)
			chat->info( tr("reconnected") );
		chatByLogin.insert(computer->name(), chat);
		chat->setHostname(computer->name());
	}

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

	RzxChat *chat = getChatByIP(ip);
	if ( chat )
		chat->info( tr("disconnected") );
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


void RzxConnectionLister::initConnection() {
	server -> setupConnection();
	if( ! client -> listen(RzxConfig::chatPort()) )
		RzxMessageBox::warning( (QWidget *) parent(), "qRezix",
		tr("Cannot create peer to peer socket !\n\nChat and properties browsing are disabled") );
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
	client->close();
	qDeleteAll(chatByIP);
	chatByIP.clear();
	chatByLogin.clear();
	if(server) server->close();
}

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir du login de l'utilisateur */
RzxChat * RzxConnectionLister::createChat( const QString& login)
{
	RzxChat *chat = getChatByName(login);
	if(!chat)
		chat = createChat(getComputerByName(login));
	if(!chat)
	{
		qWarning("Received a chat request from %s", login.toAscii().constData());
		qWarning("%s is NOT logged", login.toAscii().constData());
		return NULL;
	}
	chat->show();

	return chat;
}

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir de l'adresse de l'utilisateur */
RzxChat * RzxConnectionLister::createChat( const RzxHostAddress& peer )
{
	RzxChat *chat = getChatByIP(peer);
	if(!chat)
		chat = createChat(getComputerByIP(peer));
	if(!chat)
	{
		qWarning("Received a chat request from %s", peer.toString().toAscii().constData());
		qWarning("%s is NOT logged", peer.toString().toAscii().constData());
		return NULL;
	}
	chat->show();
	return chat;
}

///Création d'une fenêtre de chat associée à l'ordinateur
RzxChat *RzxConnectionLister::createChat(RzxComputer *computer)
{
	if (!computer)
		return NULL;

	RzxHostAddress peer = computer->ip();
	
	RzxChat *chat = new RzxChat(peer);
	chat->setHostname( computer->name() );

	connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( deleteChat( const RzxHostAddress& ) ) );
	connect( RzxConfig::global(), SIGNAL( themeChanged() ), chat, SLOT( changeTheme() ) );
	connect( RzxConfig::global(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
	chatByIP.insert(peer, chat);
	chatByLogin.insert(computer->name(), chat);
	return chat;
}


///Fermeture du chat (si il existe) associé au login
void RzxConnectionLister::closeChat( const QString& login )
{
	RzxChat *chat = getChatByName(login);
	if(chat)
		chat->close();
}


/** No descriptions */
void RzxConnectionLister::deleteChat( const RzxHostAddress& peerAddress )
{
	RzxChat *chat = chatByIP.take(peerAddress);
	chatByLogin.remove(chat->hostname());
	delete chat;
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
void RzxConnectionLister::closeChats()
{
	foreach(RzxChat *chat, chatByIP.values())
		chat->close();
}

///Demande le check des proiétés de \a peer
void RzxConnectionLister::proprietes(const RzxHostAddress& peer)
{
	RzxChat *object = getChatByIP(peer);
	if(!object)
		client->checkProperty(peer);
	else
	{
		if(object->socket())
			object->socket()->sendPropQuery();
		else
			client->checkProperty(peer);
	}
}

///Indique que les propriétés ont été checkées par \a peer
void RzxConnectionLister::warnProperties( const RzxHostAddress& peer )
{
	RzxChat *chat = getChatByIP(peer);
	RzxComputer *computer = getComputerByIP(peer);
	if (!computer)
		return ;
	QTime cur = QTime::currentTime();
	QString heure;
	heure.sprintf( "%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if (!chat)
	{
		if (RzxConfig::global()->warnCheckingProperties()== 0)
			return ;
		RzxMessageBox::information(NULL, tr("Properties sent to %1").arg(computer->name()),
			tr("name : <i>%1</i><br>"
				"address : <i>%2</i><br>"
				"client : <i>%3</i><br>"
				"time : <i>%4</i>")
				.arg(computer->name()).arg(peer.toString()).arg(computer->client()).arg(heure));
		return ;
	}
	chat->notify( tr( "has checked your properties" ), true );
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

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
#include <QBitmap>
#include <QObject>

#include "rzxconnectionlister.h"

#include "rzxmessagebox.h"
#include "rzxclientlistener.h"
#include "rzxserverlistener.h"
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

	connect( server, SIGNAL(login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)),
			 this, SLOT(login(const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&)) );
	connect( server, SIGNAL( logout( const RzxHostAddress& ) ), this, SLOT( logout( const RzxHostAddress& ) ) );
	connect( server, SIGNAL( rcvIcon( QImage*, const RzxHostAddress& ) ), this, SLOT( recvIcon( QImage*, const RzxHostAddress& ) ) );
	connect( server, SIGNAL( disconnected() ), this, SLOT( serverDisconnected() ) );
	connect( server, SIGNAL( status( const QString&, bool ) ), this, SIGNAL( status( const QString&, bool ) ) );
	connect( server, SIGNAL( connected() ), this, SLOT( serverConnected() ) );
	connect( server, SIGNAL( connected() ), this, SIGNAL( connectionEtablished()));
	connect( server, SIGNAL( disconnected() ), this, SIGNAL( socketClosed() ) );
	
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

///Enregistrement de l'arriv�e d'un nouveau client
void RzxConnectionLister::login(const RzxHostAddress& ip, const QString& name, quint32 options, quint32 version, quint32 stamp, quint32 flags, const QString& comment)
{
	RzxComputer *computer = getComputerByIP(ip);
	if(!computer)
	{
		computer = new RzxComputer(ip, name, options, version, stamp, flags, comment);
		computerByIP.insert(ip, computer);
		displayWaiter << computer;
	
		if(!delayDisplay.isActive())
			delayDisplay.start(1, true);
	}
	else
	{
		RzxChat *chat = chatByLogin.take(computer->getName());
		computerByLogin.remove(computer->getName());
		computer->update(name, options, stamp, flags, comment);
		computerByLogin.insert(name, computer);
		if(chat)
		{
			chatByLogin.insert(name, chat);
			chat->setHostname(name);
		}
		emit login(computer);
	}
}

/** Sert aussi au raffraichissement des donn�es*/
void RzxConnectionLister::login()
{
	if(displayWaiter.isEmpty()) return;
	RzxComputer *computer = displayWaiter.takeFirst();
	connect(computer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );

	// Recherche si cet ordinateur �tait d�j� pr�sent (refresh ou login)
	QString tempIP = computer->getIP().toString();

	//mise � jour des donn�es concernant le chat
	RzxChat *chat = chatByLogin.take(computer->getName());
	if(chat)
	{
		//Indication au chat de la reconnexion
		if (!computer)
			chat->info( tr("reconnected") );
		chatByLogin.insert(computer->getName(), chat);
		chat->setHostname(computer->getName());
	}

	emit login(computer);
	
	//Ajout du nouvel ordinateur dans les qdict
	computerByLogin.insert(computer->getName(), computer);
	emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	
	if(!displayWaiter.isEmpty()) delayDisplay.start(5, true);
	else
	{
		emit loginEnd();
		initialized = true;
	}
}

///Enregistre la d�connexion d'un client
void RzxConnectionLister::logout( const RzxHostAddress& ip )
{
	RzxComputer *computer = getComputerByIP(ip);
	if(computer)
	{
		emit logout(computer);
		computerByIP.remove(ip);
		computerByLogin.remove(computer->getName());
		computer->deleteLater();
		emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	}

	RzxChat *chat = getChatByIP(ip);
	if ( chat )
		chat->info( tr("disconnected") );
}


///Retourne la liste des IP des gens connect�s
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connect�es */
QStringList RzxConnectionLister::getIpList(unsigned int features)
{
	QHashIterator<RzxHostAddress,RzxComputer*> it(computerByIP);
	QStringList ips;
	while(it.hasNext())
	{
		it.next();
		if(!features || it.value()->can(features))
			ips << it.key().toString();
	}
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
	displayWaiter.clear();
}

/** No descriptions */
void RzxConnectionLister::serverConnected()
{
	qDeleteAll(computerByIP);
	computerByIP.clear();
	computerByLogin.clear();
	displayWaiter.clear();
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

///Cr�ation d'une fen�tre de chat et r�f�rencement de celle-ci
/** Cr�ation � partir du login de l'utilisateur */
RzxChat * RzxConnectionLister::chatCreate( const QString& login)
{
	RzxChat *chat = getChatByName(login);
	if(!chat)
		chat = createChat(getComputerByName(login));
	if(!chat)
	{
		qWarning( tr("Received a chat request from %1").arg(login) );
		qWarning( tr("%1 is NOT logged").arg(login) );
		return NULL;
	}
	chat->show();

	return chat;
}

///Cr�ation d'une fen�tre de chat et r�f�rencement de celle-ci
/** Cr�ation � partir de l'adresse de l'utilisateur */
RzxChat * RzxConnectionLister::chatCreate( const RzxHostAddress& peer )
{
	RzxChat *chat = getChatByIP(peer);
	if(!chat)
		chat = createChat(getComputerByIP(peer));
	if(!chat)
	{
		qWarning( tr( "Received a chat request from %1" ).arg( peer.toString() ) );
		qWarning( tr( "%1 is NOT logged" ).arg( peer.toString() ) );
		return NULL;
	}
	chat->show();
	return chat;
}

RzxChat *RzxConnectionLister::createChat(RzxComputer *computer)
{
	if (!computer)
		return NULL;

	RzxHostAddress peer = computer->getIP();
	
	RzxChat *chat = new RzxChat(peer);
	chat->setHostname( computer->getName() );

	connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( chatDelete( const RzxHostAddress& ) ) );
	connect( RzxConfig::globalConfig(), SIGNAL( themeChanged() ), chat, SLOT( changeTheme() ) );
	connect( RzxConfig::globalConfig(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
	chatByIP.insert(peer, chat);
	chatByLogin.insert(computer->getName(), chat);
	return chat;
}


///Fermeture du chat (si il existe) associ� au login
void RzxConnectionLister::closeChat( const QString& login )
{
	RzxChat *chat = getChatByName(login);
	if(chat)
		chat->close();
}


/** No descriptions */
void RzxConnectionLister::chatDelete( const RzxHostAddress& peerAddress )
{
	RzxChat *chat = chatByIP.take(peerAddress);
	chatByLogin.remove(chat->getHostName());
	delete chat;
}

///Fermeture des chats en cours
/** Cette m�thode � pour but de fermer tous les chats en cours pour lib�rer en particulier le port 5050. Normalement l'appel de cette m�thode � la fermeture de qRezix doit corriger le probl�me de r�ouverture de l'�coute qui intervient � certains moments lors du d�marrage de qRezix */
void RzxConnectionLister::closeChats()
{
	QHashIterator<RzxHostAddress, RzxChat*> it(chatByIP);
	while(it.hasNext())
	{
		it.next();
		it.value()->close();
	}
}

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
		if (RzxConfig::globalConfig()->warnCheckingProperties()== 0)
			return ;
		RzxMessageBox::information(NULL, tr("Properties sent to %1").arg(computer->getName()),
			tr("name : <i>%1</i><br>"
				"address : <i>%2</i><br>"
				"client : <i>%3</i><br>"
				"time : <i>%4</i>")
				.arg(computer->getName()).arg(peer.toString()).arg(computer->getClient()).arg(heure));
		return ;
	}
	chat->notify( tr( "has checked your properties" ), true );
}

/** No descriptions */
void RzxConnectionLister::sysmsg( const QString& msg )
{
	// Bo�te de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information(NULL, tr( "XNet Server message:" ), msg );
}
/** No descriptions */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Bo�te de dialogue modale
	RzxMessageBox::critical(NULL, tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
}

/** No descriptions */
void RzxConnectionLister::recvIcon(QImage* icon, const RzxHostAddress& ip)
{
	RzxComputer * computer = getComputerByIP(ip);
	if (computer)
	{
		icon->save(RzxConfig::computerIconsDir().absFilePath(computer->getFilename()), "PNG");
		QPixmap pix;
		pix.convertFromImage(*icon);
		computer->setIcon(pix);
	}
}

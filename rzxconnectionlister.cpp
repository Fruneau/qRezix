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

#include "q.xpm"

#define USER_HASH_TABLE_LENGTH 1663


RzxConnectionLister *RzxConnectionLister::object = NULL;

RzxConnectionLister::RzxConnectionLister( QObject *parent)
		: QObject(parent)
{
	object = this;

	server = RzxServerListener::object();
	client = RzxClientListener::global();

	connect( server, SIGNAL( login( const QString& ) ), this, SLOT( login( const QString& ) ) );
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

///Enregistrement de l'arrivée d'un nouveau client
/** Sert aussi au raffraichissement des données*/
void RzxConnectionLister::login( const QString& newOrdi )
{
	QString ordi = newOrdi;
	
	if(!ordi.isNull())
	{
		displayWaiter << newOrdi;
		if(!delayDisplay.isActive())
			delayDisplay.start(1, true);
		return;
	}
	
	if(displayWaiter.isEmpty()) return;
	ordi = displayWaiter[0];
	displayWaiter.remove(ordi);

	RzxComputer * newComputer = new RzxComputer();
	connect( newComputer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
	if( newComputer -> parse( ordi ) )
	{
		delete newComputer;
		return ;
	}

	// Recherche si cet ordinateur était déjà présent (refresh ou login)
	QString tempIP = newComputer -> getIP().toString();
	RzxComputer *computer = getComputerByIP(newComputer->getIP());
	if(computer)
	{
		//suppression de l'ancien computer du qdict par nom
		computerByLogin.remove(computer->getName());
		
		//transfert des fenêtres fille vers le nouveau computer
		if(!computer->children().isEmpty())
		{
			QObjectList list = computer->children();
			for(QList<QObject *>::iterator item = list.begin() ; item != list.end() ; item++)
			{
				computer->removeChild(*item);
				newComputer->insertChild(*item);
			}
		}
		disconnect(computer, 0, 0, 0);
	}

	//mise à jour des données concernant le chat
	RzxChat *chat;
	if(computer)
		chat = chatByLogin.take(computer->getName());
	else
		chat = chatByLogin.take(newComputer->getName());

	if(chat)
	{
		//Indication au chat de la reconnexion
		if (!computer)
			chat->info( tr("reconnected") );

		chatByLogin.insert(newComputer->getName(), chat);
		chat->setHostname(newComputer->getName());
	}
		
	emit login( newComputer);

	//Placé après le emit login, pour que les signaux emits par la destruction n'interfèrent pas avec l'affichage
	if(computer) computer->deleteLater();
	
	//Ajout du nouvel ordinateur dans les qdict
	computerByLogin.insert(newComputer->getName(), newComputer);
	computerByIP.insert(newComputer->getIP(), newComputer );

	emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	
	if(!displayWaiter.isEmpty()) delayDisplay.start(5, true);
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
	if (computer)
	{
		computerByLogin.remove(computer->getName());
		emit countChange( tr("%1 clients connected").arg(computerByIP.count()) );
	}
	emit logout(computer);
	computer->deleteLater();

	RzxChat *chat = getChatByIP(ip);
	if ( chat )
		chat->info( tr("disconnected") );
}


///Retourne la liste des IP des gens connectés
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connectées */
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

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir du login de l'utilisateur */
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

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir de l'adresse de l'utilisateur */
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

	QPixmap iconeProg( ( const char ** ) q );
	iconeProg.setMask( iconeProg.createHeuristicMask() );
	
	chat->setIcon( iconeProg );
	chat->setHostname( computer->getName() );

	connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( chatDelete( const RzxHostAddress& ) ) );
	connect( RzxConfig::globalConfig(), SIGNAL( themeChanged() ), chat, SLOT( changeTheme() ) );
	connect( RzxConfig::globalConfig(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
	chatByIP.insert(peer, chat);
	chatByLogin.insert(computer->getName(), chat);
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
void RzxConnectionLister::chatDelete( const RzxHostAddress& peerAddress )
{
	RzxChat *chat = chatByIP.take(peerAddress);
	chatByLogin.remove(chat->getHostName());
	delete chat;
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
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
	RzxComputer * computer = getComputerByIP(ip);
	if (computer)
	{
		icon->save(RzxConfig::computerIconsDir().absFilePath(computer->getFilename()), "PNG");
		QPixmap pix;
		pix.convertFromImage(*icon);
		computer->setIcon(pix);
	}
}

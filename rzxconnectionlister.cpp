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
#include <qsocket.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qobjectlist.h>

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

RzxConnectionLister::RzxConnectionLister( QObject *parent, const char *name)
		: QObject( parent, name ), displayWaiter(), iplist( USER_HASH_TABLE_LENGTH ), computerByLogin( USER_HASH_TABLE_LENGTH )
{
	object = this;

	iplist.setAutoDelete( true );
	chats.setAutoDelete( true );
	chatsByLogin.setAutoDelete( false ); //surtout pas true, sinon on détruit 2 fois les mêmes objets
	computerByLogin.setAutoDelete(false);

	server = RzxServerListener::object();
	client = RzxClientListener::object();

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
	iplist.clear();
	chats.clear();
	if(server)
		server->deleteLater();
	object = NULL;
}

///Enregistrement de l'arrivée d'un nouveau client
/** Sert aussi au raffraichissement des données*/
void RzxConnectionLister::login( const QString& newOrdi )
{
	QString ordi = newOrdi;
	
	if(ordi)
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
	RzxComputer *computer = iplist.take( tempIP );
	if(computer)
	{
		//suppression de l'ancien computer du qdict par nom
		computerByLogin.remove(computer->getName());
		
		//transfert des fenêtres fille vers le nouveau computer
		if(computer->children())
		{
			QObjectList list = *(computer->children());
			for(QObject *item = list.first() ; item ; item = list.next())
			{
				computer->removeChild(item);
				newComputer->insertChild(item);
			}
		}
		disconnect(computer, 0, 0, 0);
	}

	//mise à jour des données concernant le chat
	RzxChat *chat;
	if(computer)
		chat = chatsByLogin.take(computer->getName());
	else
		chat = chatsByLogin.take(newComputer->getName());

	if(chat)
	{
		//Indication au chat de la reconnexion
		if (!computer)
			chat->info( tr( "reconnected" ) );

		chatsByLogin.insert(newComputer->getName(), chat);
		chat->setHostname(newComputer->getName());
	}
		
	emit login( newComputer);

	//Placé après le emit login, pour que les signaux emits par la destruction n'interfèrent pas avec l'affichage
	if(computer) computer->deleteLater();
	
	//Ajout du nouvel ordinateur dans les qdict
	computerByLogin.insert(newComputer->getName(), newComputer);
	iplist.insert( newComputer -> getIP().toString(), newComputer );

	emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
	
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
	QString key = ip.toString();
	RzxComputer *computer = iplist.take( key );
	if (computer)
	{
		computerByLogin.remove(computer->getName());
		emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
	}
	computer->deleteLater();
	
	emit logout(key);

	RzxChat *chat = chats.find(key);
	if ( chat )
		chat->info( tr( "disconnected" ) );
}


///Retourne la liste des IP des gens connectés
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connectées */
QStringList RzxConnectionLister::getIpList(unsigned int features)
{
	QDictIterator<RzxComputer> it(iplist);
	QStringList ips;
	for( ; it.current() ; ++it)
	{
		if(!features || it.current()->can(features))
			ips << (it.currentKey());
	}
	return ips;
}


void RzxConnectionLister::initConnection() {
	server -> setupConnection();
	if( ! client -> listenOnPort(RzxConfig::chatPort()) )
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
	iplist.clear();
	computerByLogin.clear();
	displayWaiter.clear();
	initialized = false;
}

/** No descriptions */
bool RzxConnectionLister::isSocketClosed() const
{
	return server -> isSocketClosed();
}

/** No descriptions */
void RzxConnectionLister::closeSocket()
{
	disconnect( server, SIGNAL( disconnected() ), this, SLOT( serverDisconnected() ) );
	client -> close();
	chats.clear();
	if ( server ) server -> close();
}

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir du login de l'utilisateur */
RzxChat * RzxConnectionLister::chatCreate( const QString& login)
{
	RzxChat *chat = chatsByLogin.find(login);
	if(!chat)
		chat = createChat(computerByLogin.find(login));
	if(!chat)
	{
		qWarning( tr( "Received a chat request from %1" ).arg(login) );
		qWarning( tr( "%1 is NOT logged" ).arg(login) );
		return NULL;
	}
	chat->show();

	return chat;
}

///Création d'une fenêtre de chat et référencement de celle-ci
/** Création à partir de l'adresse de l'utilisateur */
RzxChat * RzxConnectionLister::chatCreate( const RzxHostAddress& peer )
{
	RzxChat *chat = chats.find( peer.toString());
	if(!chat)
		chat = createChat(iplist.find(peer.toString()));
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
	if ( !computer )
		return NULL;
		
	RzxHostAddress peer = computer->getIP();
	
	RzxChat *chat = new RzxChat(peer);

	QPixmap iconeProg( ( const char ** ) q );
	iconeProg.setMask( iconeProg.createHeuristicMask() );
	
	chat->setIcon( iconeProg );
	chat->setHostname( computer->getName() );
	chat->edMsg->setFocus();

	connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( chatDelete( const RzxHostAddress& ) ) );
	connect( RzxConfig::globalConfig(), SIGNAL( themeChanged() ), chat, SLOT( changeTheme() ) );
	connect( RzxConfig::globalConfig(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
	chats.insert( peer.toString(), chat );
	chatsByLogin.insert(computer->getName(), chat);
	return chat;
}


///Fermeture du chat (si il existe) associé au login
void RzxConnectionLister::closeChat( const QString& login )
{
	RzxChat *chat = chatsByLogin.find(login);
	if(chat)
		chat->close();
}


/** No descriptions */
void RzxConnectionLister::chatDelete( const RzxHostAddress& peerAddress )
{
	RzxChat *chat = chats.take( peerAddress.toString() );
	chatsByLogin.remove(chat->getHostName());
	delete chat;
}

///Fermeture des chats en cours
/** Cette méthode à pour but de fermer tous les chats en cours pour libérer en particulier le port 5050. Normalement l'appel de cette méthode à la fermeture de qRezix doit corriger le problème de réouverture de l'écoute qui intervient à certains moments lors du démarrage de qRezix */
void RzxConnectionLister::closeChats()
{
	QDictIterator<RzxChat> it( chats );
	for ( ; it.current() ; ++it )
	{
		it.current() ->close();
	}
}

void RzxConnectionLister::warnProperties( const RzxHostAddress& peer )
{
	RzxChat *chat = chats.find( peer.toString() );
	RzxComputer *computer = iplist.find( peer.toString() );
	if ( !computer )
		return ;
	QTime cur = QTime::currentTime();
	QString heure;
	heure.sprintf( "%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if ( !chat )
	{
		if ( RzxConfig::globalConfig() ->warnCheckingProperties() == 0 )
			return ;
		RzxMessageBox::information(NULL, tr("Properties sent to %1").arg( computer->getName() ),
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
void RzxConnectionLister::recvIcon(QImage* icon, const RzxHostAddress& ip){
	RzxComputer * computer = iplist.find(ip.toString());
	if (computer)
	{
		icon -> save(RzxConfig::computerIconsDir().absFilePath(computer -> getFilename()), "PNG");
		QPixmap pix;
		pix.convertFromImage(*icon);
		computer->setIcon(pix);
	}
}

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
		: QObject( parent, name ), iplist( USER_HASH_TABLE_LENGTH )
{
	object = this;

	iplist.setAutoDelete( true );
	chats.setAutoDelete( true );

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
	
	connect(client, SIGNAL(chat(QSocket*, const QString& )), this, SLOT(chat(QSocket*, const QString& )));
	connect(client, SIGNAL(propertiesSent(const RzxHostAddress&)), this, SLOT(warnProperties(const RzxHostAddress&)));
}

RzxConnectionLister::~RzxConnectionLister()
{
	iplist.clear();
	chats.clear();
}

///Enregistrement de l'arrivée d'un nouveau client
/** Sert aussi au raffraichissement des données*/
void RzxConnectionLister::login( const QString& ordi )
{
	RzxComputer * newComputer = new RzxComputer;
	connect( newComputer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
	if( newComputer -> parse( ordi ) )
	{
		delete newComputer;
		return ;
	}

	// Recherche si cet ordinateur était déjà présent
	QString tempIP = newComputer -> getIP().toString();
	RzxComputer *computer = iplist.take( tempIP );
	if(computer && computer->children())
	{
		QObjectList list = *(computer->children());
		for(QObject *item = list.first() ; item ; item = list.next())
		{
			if(item->inherits("RzxItem"))
			{
				computer->removeChild(item);
				newComputer->insertChild(item);
			}
		}
		computer->deleteLater();
	}

	emit login( newComputer);

	iplist.insert( newComputer -> getIP().toString(), newComputer );
	RzxChat * chatWithLogin = chats.find( newComputer -> getIP().toString() );
	if ( !object && chatWithLogin )
		chatWithLogin->info( tr( "reconnected" ) );
	emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
}

///Enregistre la déconnexion d'un client
void RzxConnectionLister::logout( const RzxHostAddress& ip )
{
	QString key = ip.toString();
	RzxComputer * object = iplist.find( key );
	if ( object )
	{
		iplist.remove( key );
		emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
	}
	
	emit logout(key);

	RzxChat * chatWithLogin = chats.find( ip.toString() );
	if ( chatWithLogin )
		chatWithLogin->info( tr( "disconnected" ) );
}


void RzxConnectionLister::initConnection() {
	server -> setupConnection();
	if( ! client -> listenOnPort(RzxConfig::chatPort()) )
		RzxMessageBox::warning( (QWidget *) parent(), "qRezix",
		tr("Cannot create peer to peer socket !\n\nChat and properties browsing are disabled") );
}

/** No descriptions */
void RzxConnectionLister::serverDisconnected()
{}

/** No descriptions */
void RzxConnectionLister::serverConnected()
{
	iplist.clear();
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


void RzxConnectionLister::chat( QSocket* socket, const QString& msg )
{
	RzxHostAddress peer = socket->peerAddress();
	if ( RzxConfig::autoResponder() )
	{
		( ( RzxChatSocket* ) socket ) -> sendResponder( RzxConfig::autoResponderMsg() );
		emit status( tr( "%1 is now marked as away" ).arg( peer.toString() ), false );
	}
	else
	{
		RzxChat * chatWindow = chatCreate( peer );
		if ( !chatWindow ) return ;
		chatWindow->setSocket( ( RzxChatSocket* ) socket );      //on change le socket si nécessaire
		chatWindow -> receive( msg );
	}
}

RzxChat * RzxConnectionLister::chatCreate( const RzxHostAddress& peer )
{
	RzxChat * object = chats.find( peer.toString() );
	if ( !object )
	{
		RzxComputer * computer = iplist.find( peer.toString() );
		if ( !computer )
		{
			qWarning( tr( "Received a chat request from %1" ).arg( peer.toString() ) );
			qWarning( tr( "%1 is NOT logged" ).arg( peer.toString() ) );
			return 0;
		}
		object = new RzxChat( peer );

		QPixmap iconeProg( ( const char ** ) q );
		iconeProg.setMask( iconeProg.createHeuristicMask() );
		object->setIcon( iconeProg );

#ifdef WIN32
		object->setCaption( tr( "Chat" ) + " - " + computer->getName() + " [Qt]" );
#else
		object->setCaption( tr( "Chat" ) + " - " + computer->getName() );
#endif
		object->setHostname( computer->getName() );

		object->edMsg->setFocus();

		connect( object, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( chatDelete( const RzxHostAddress& ) ) );
		connect( RzxConfig::globalConfig(), SIGNAL( themeChanged() ), object, SLOT( changeTheme() ) );
		connect( RzxConfig::globalConfig(), SIGNAL( iconFormatChange() ), object, SLOT( changeIconFormat() ) );
		chats.insert( peer.toString(), object );
	}
	object -> show();

	return object;
}

/** No descriptions */
void RzxConnectionLister::chatDelete( const RzxHostAddress& peerAddress )
{
	// Auto-Delete = true, le chat est supprimé automatiquement. Qt rules !!!
	chats.remove( peerAddress.toString() );
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
	RzxChat * object = chats.find( peer.toString() );
	RzxComputer * computer = iplist.find( peer.toString() );
	if ( !computer )
		return ;
	QTime cur = QTime::currentTime();
	QString heure;
	heure.sprintf( "<i>%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if ( !object )
	{
		if ( RzxConfig::globalConfig() ->warnCheckingProperties() == 0 )
			return ;
		sysmsg( tr( "Properties sent to %2 (%3) at %1" ).arg( heure ).arg( computer->getName() ).arg( peer.toString() ) );
		return ;
	}
	object->notify( tr( "has checked your properties" ), true );
}

/** No descriptions */
void RzxConnectionLister::sysmsg( const QString& msg )
{
	// Boîte de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information( ( QWidget * ) parent(), tr( "XNet Server message:" ), msg );
}
/** No descriptions */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Boîte de dialogue modale
	RzxMessageBox::critical( ( QWidget * ) parent(), tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
}

/** No descriptions */
void RzxConnectionLister::recvIcon(QImage* icon, const RzxHostAddress& ip){
	RzxComputer * object = iplist.find(ip.toString());
	if (object) {
		icon -> save(RzxConfig::computerIconsDir().absFilePath(object -> getFilename()), "PNG");
		QPixmap pix;
		pix.convertFromImage(*icon);
		object -> setIcon(pix);
	}
}

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
	chatsByLogin.setAutoDelete( false ); //surtout pas true, sinon on d�truit 2 fois les m�mes objets
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
}

RzxConnectionLister::~RzxConnectionLister()
{
	iplist.clear();
	chats.clear();
	object = NULL;
}

///Enregistrement de l'arriv�e d'un nouveau client
/** Sert aussi au raffraichissement des donn�es*/
void RzxConnectionLister::login( const QString& ordi )
{
	RzxComputer * newComputer = new RzxComputer();
	connect( newComputer, SIGNAL( needIcon( const RzxHostAddress& ) ), this, SIGNAL( needIcon( const RzxHostAddress& ) ) );
	if( newComputer -> parse( ordi ) )
	{
		delete newComputer;
		return ;
	}

	// Recherche si cet ordinateur �tait d�j� pr�sent
	QString tempIP = newComputer -> getIP().toString();
	RzxComputer *computer = iplist.take( tempIP );
	if(computer)
	{
		//suppression de l'ancien computer du qdict par nom
		computerByLogin.remove(computer->getName());
		
		//mise � jour des donn�es concernant le chat
		RzxChat *chat = chatsByLogin.take(computer->getName());
		if(chat)
		{
			chatsByLogin.insert(newComputer->getName(), chat);
#ifdef WIN32
			chat->setCaption( tr( "Chat" ) + " - " + newComputer->getName() + " [Qt]" );
#else
			chat->setCaption( tr( "Chat" ) + " - " + newComputer->getName() );
#endif
			chat->setHostname(newComputer->getName());
		}
		
		//transfert des fen�tres fille vers le nouveau computer
		if(computer->children())
		{
			QObjectList list = *(computer->children());
			for(QObject *item = list.first() ; item ; item = list.next())
			{
//				if(item->inherits("RzxItem"))
				//{
					computer->removeChild(item);
					newComputer->insertChild(item);
				//}
			}
		}
		computer->deleteLater();
	}

	emit login( newComputer);

	//Ajout du nouvel ordinateur dans les qdict
	computerByLogin.insert(newComputer->getName(), newComputer);
	iplist.insert( newComputer -> getIP().toString(), newComputer );
	
	//Indication au chat de la reconnexion
	RzxChat * chatWithLogin = chats.find( newComputer -> getIP().toString() );
	if (!computer && chatWithLogin)
		chatWithLogin->info( tr( "reconnected" ) );
	emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
}

///Enregistre la d�connexion d'un client
void RzxConnectionLister::logout( const RzxHostAddress& ip )
{
	QString key = ip.toString();
	RzxComputer *computer = iplist.find( key );
	if (computer)
	{
		computerByLogin.remove(computer->getName());
		iplist.remove( key );
		emit countChange( tr( "%1 clients connected" ).arg( iplist.count() ) );
	}
	
	emit logout(key);

	RzxChat *chat = chats.find(key);
	if ( chat )
		chat->info( tr( "disconnected" ) );
}


///Retourne la liste des IP des gens connect�s
/** Permet pour les plug-ins d'obtenir facilement la liste les ip connect�es */
QStringList RzxConnectionLister::getIpList()
{
	QDictIterator<RzxComputer> it(iplist);
	QStringList ips;
	for( ; it.current() ; ++it)
	{
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

///Cr�ation d'une fen�tre de chat et r�f�rencement de celle-ci
/** Cr�ation � partir du login de l'utilisateur */
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

///Cr�ation d'une fen�tre de chat et r�f�rencement de celle-ci
/** Cr�ation � partir de l'adresse de l'utilisateur */
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

#ifdef WIN32
	chat->setCaption( tr( "Chat" ) + " - " + computer->getName() + " [Qt]" );
#else
	chat->setCaption( tr( "Chat" ) + " - " + computer->getName() );
#endif
	chat->setHostname( computer->getName() );

	chat->edMsg->setFocus();

	connect( chat, SIGNAL( closed( const RzxHostAddress& ) ), this, SLOT( chatDelete( const RzxHostAddress& ) ) );
	connect( RzxConfig::globalConfig(), SIGNAL( themeChanged() ), chat, SLOT( changeTheme() ) );
	connect( RzxConfig::globalConfig(), SIGNAL( iconFormatChange() ), chat, SLOT( changeIconFormat() ) );
	chats.insert( peer.toString(), chat );
	chatsByLogin.insert(computer->getName(), chat);
	return chat;
}


///Fermeture du chat (si il existe) associ� au login
void RzxConnectionLister::closeChat( const QString& login )
{
	RzxChat *chat = chatsByLogin.find(login);
	if(chat)
		chat->close();
}


/** No descriptions */
void RzxConnectionLister::chatDelete( const RzxHostAddress& peerAddress )
{
	// Auto-Delete = true, le chat est supprim� automatiquement. Qt rules !!!
	chats.remove( peerAddress.toString() );
	RzxComputer *computer = iplist.find( peerAddress.toString());
	if(computer)
		chatsByLogin.remove( computer->getName());
}

///Fermeture des chats en cours
/** Cette m�thode � pour but de fermer tous les chats en cours pour lib�rer en particulier le port 5050. Normalement l'appel de cette m�thode � la fermeture de qRezix doit corriger le probl�me de r�ouverture de l'�coute qui intervient � certains moments lors du d�marrage de qRezix */
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
	heure.sprintf( "<i>%2i:%.2i:%.2i",
	               cur.hour(),
	               cur.minute(),
	               cur.second() );

	if ( !chat )
	{
		if ( RzxConfig::globalConfig() ->warnCheckingProperties() == 0 )
			return ;
		sysmsg( tr( "Properties sent to %2 (%3) at %1" ).arg( heure ).arg( computer->getName() ).arg( peer.toString() ) );
		return ;
	}
	chat->notify( tr( "has checked your properties" ), true );
}

/** No descriptions */
void RzxConnectionLister::sysmsg( const QString& msg )
{
	// Bo�te de dialogue non modale, pour que les comms continuent.
	RzxMessageBox::information( ( QWidget * ) parent(), tr( "XNet Server message:" ), msg );
}
/** No descriptions */
void RzxConnectionLister::fatal( const QString& msg )
{
	// Bo�te de dialogue modale
	RzxMessageBox::critical( ( QWidget * ) parent(), tr( "Error" ) + " - " + tr( "XNet Server message" ), msg, true );
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

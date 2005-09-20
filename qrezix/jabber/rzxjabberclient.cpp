#include "rzxjabberclient.h"
#include "rzxjabberconfig.h"

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/discohandler.h>
#include <gloox/presencehandler.h>
#include <gloox/loghandler.h>
#include <gloox/gloox.h>
using namespace gloox;

#include <stdio.h>
#include <string>


RzxJabberClient::RzxJabberClient(std::string server) {
	j = new Client( server);
//	j->setForceNonSasl( true );
	j->setAutoPresence( true );
	j->setInitialPriority( 5 );
	j->registerConnectionListener( this );
	j->registerMessageHandler( this );
	j->registerPresenceHandler( this );
	j->registerLogHandler( this );
	j->disco()->registerDiscoHandler( this );
	j->disco()->setVersion( "qRezix Jabber", "0.0.1-svn");
	j->disco()->setIdentity( "client", "qRezix" );
}

void RzxJabberClient::run()
{
	j->jid().setJID(RzxJabberConfig::user().toStdString());
	j->setPassword(RzxJabberConfig::pass().toStdString());
	j->setServer(RzxJabberConfig::serverName().toStdString());
	j->setPort(RzxJabberConfig::serverPort());
	isStarted=true;
	while(isStarted){
		j->connect();
		sleep(10); // Pause à la reconnection
	}
}

void RzxJabberClient::stop(){
	isStarted=false;
	j->disconnect();
}

void RzxJabberClient::onConnect()
{
	emit connected();
};

void RzxJabberClient::onDisconnect( ConnectionError e )
{
	printf( "disconnected: %d\n", e );
	if( e == CONN_AUTHENTICATION_FAILED )
		printf( "auth failed. reason: %d\n", j->authError() );
	emit disconnected();
};

bool RzxJabberClient::onTLSConnect( const CertInfo& info )
{
	printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
		info.status, info.issuer.c_str(), info.server.c_str(),
		info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
		info.compression.c_str() );
	return true;
};

void RzxJabberClient::handleDiscoInfoResult( const std::string& id, const Stanza& stanza )
{
	printf( "handleDiscoInfoResult}\n" );
}

void RzxJabberClient::handleDiscoItemsResult( const std::string& id, const Stanza& stanza )
{
	printf( "handleDiscoItemsResult\n" );
}

void RzxJabberClient::handleMessage( Stanza *stanza )
{
	if(!stanza->body().empty()){
		printf( "type: %d, subject: %s, message: %s, thread id: %s\n", stanza->subtype(),
			stanza->subject().c_str(), stanza->body().c_str(), stanza->thread().c_str() );
		Tag *m = new Tag( "message" );
		m->addAttrib( "from", j->jid().full() );
		m->addAttrib( "to", stanza->from().full() );
		m->addAttrib( "type", "chat" );
		Tag *b = new Tag( "body", "Vous avez dit:\n> " + stanza->body() );
		m->addChild( b );
		if( !stanza->subject().empty() )
		{
			Tag *s = new Tag( "subject", "Re:" +  stanza->subject() );
			m->addChild( s );
		}
		j->send( m );
	}
}

void RzxJabberClient::handlePresence( Stanza *stanza ){
	if(stanza->show()==PRESENCE_UNAVAILABLE)
		emit login(QString::fromStdString(stanza->from().bare()), 0); /** @todo Gérer déconnexion */
	else if(stanza->show()==PRESENCE_AWAY || stanza->show()==PRESENCE_DND || stanza->show()==PRESENCE_XA)
		emit login(QString::fromStdString(stanza->from().bare()), 1);
	else
		emit login(QString::fromStdString(stanza->from().bare()), 2);
	if(stanza->show()==PRESENCE_UNKNOWN)
		printf("Unknown presence type");
	
}

void RzxJabberClient::handleLog( const std::string& xml, bool incoming ){
	/// @todo gérer les logs
};

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


RzxJabberClient::RzxJabberClient(QObject *parent)
	:QThread(parent)
{
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(readData()));
}

RzxJabberClient::~RzxJabberClient() {
	if(j)
		delete j;
	if(timer)
		delete timer;
};

void RzxJabberClient::run()
{
	j = new Client(RzxJabberConfig::user().append("/qRezix").toStdString(),
			RzxJabberConfig::pass().toStdString(), 
			RzxJabberConfig::serverPort()
		);
	j->setServer(RzxJabberConfig::serverName().toStdString());
	j->setAutoPresence( true );
	j->setInitialPriority( 5 );
	j->registerConnectionListener( this );
	j->registerMessageHandler( this );
	j->registerPresenceHandler( this );
	j->registerLogHandler( this );
	j->disco()->registerDiscoHandler( this );
	j->disco()->setVersion( "qRezix Jabber", "0.0.1-svn");
	j->disco()->setIdentity( "client", "qRezix" );
	j->connect(false);
	timer->start(100);
	exec();
}

void RzxJabberClient::readData(){
	j->recv(0);
}

void RzxJabberClient::stop(){
	j->disconnect();
	quit();
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
		emit presence(QString::fromStdString(stanza->from().full()), 0); /** @todo G�er d�onnexion */
	else if(stanza->show()==PRESENCE_AWAY || stanza->show()==PRESENCE_DND || stanza->show()==PRESENCE_XA)
		emit presence(QString::fromStdString(stanza->from().full()), 1);
	else
		emit presence(QString::fromStdString(stanza->from().full()), 2);
	if(stanza->show()==PRESENCE_UNKNOWN)
		printf("Unknown presence type");
	
}

void RzxJabberClient::handleLog( const std::string& xml, bool incoming ){
	/// @todo g�er les logs
};

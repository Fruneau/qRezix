#include "rzxjabberclient.h"
#include "rzxjabberconfig.h"

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/presencehandler.h>
#include <gloox/gloox.h>
using namespace gloox;

#include <stdio.h>
#include <string>


RzxJabberClient::RzxJabberClient(QObject *parent)
	:QThread(parent)
{
	j = 0;
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(readData()));
}

RzxJabberClient::~RzxJabberClient() {
	if(timer)
		delete timer;
	if(j){
		j->removeConnectionListener( this );
		j->removeMessageHandler( this );
		j->removePresenceHandler( this );
		delete j;
	}
};

void RzxJabberClient::run()
{
	j = new Client(std::string(RzxJabberConfig::user().append("/qRezix").toUtf8().data()),
		       std::string(RzxJabberConfig::pass().toUtf8().data()), 
			RzxJabberConfig::serverPort()
		);
	j->setServer(RzxJabberConfig::serverName().toUtf8().data());
	j->setAutoPresence( true );
	j->setInitialPriority( 5 );
	j->registerConnectionListener( this );
	j->registerMessageHandler( this );
	j->registerPresenceHandler( this );
	j->disco()->setVersion( "qRezix Jabber", "0.0.1-svn");
	j->disco()->setIdentity( "client", "qRezix" );
	if(j->connect(false))
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

bool RzxJabberClient::isStarted()
{
	return j ?j->authed():false;
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



void RzxJabberClient::handleMessage( Stanza *stanza )
{
	emit msgReceived(QString::fromUtf8(stanza->from().bare().data()) , QString::fromUtf8(stanza->body().data()) );
}


void RzxJabberClient::handlePresence( Stanza *stanza ){
	if(stanza->show()==PRESENCE_UNAVAILABLE)
		emit presence(QString::fromUtf8(stanza->from().bare().data()),QString::fromUtf8(stanza->from().bare().data()), 3);
	else if(stanza->show()==PRESENCE_AWAY || stanza->show()==PRESENCE_DND || stanza->show()==PRESENCE_XA)
		emit presence(QString::fromUtf8(stanza->from().bare().data()),QString::fromUtf8(stanza->from().bare().data()), 1);
	else
		emit presence(QString::fromUtf8(stanza->from().bare().data()),QString::fromUtf8(stanza->from().bare().data()), 2);
	emit rosterUpdated();
}


bool RzxJabberClient::send(Tag* t){
	if(j && isStarted()){
		j->send(t);
		return true;
	}
	return false;
};

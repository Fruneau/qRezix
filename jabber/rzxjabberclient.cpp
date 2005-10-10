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
	j = 0;
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(readData()));
}

RzxJabberClient::~RzxJabberClient() {
	if(timer)
		delete timer;
	if(j){
		//j->removeConnectionListener( this );
		j->removeMessageHandler( this );
		j->removePresenceHandler( this );
		j->removeLogHandler( this );
		j->disco()->removeDiscoHandler( this );
		j->rosterManager()->removeRosterListener();
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
	j->registerLogHandler( this );
	j->disco()->registerDiscoHandler( this );
	j->disco()->setVersion( "qRezix Jabber", "0.0.1-svn");
	j->disco()->setIdentity( "client", "qRezix" );
	j->rosterManager()->registerRosterListener(this);
	j->connect(false);
	timer->start(100);
	exec();
}

// bool gloox::RosterListener::unsubscriptionRequest(const std::string&, const std::string&)’:

void RzxJabberClient::itemAvailable(RosterItem & item, const std::string &msg){
	emit presence(QString::fromUtf8(item.jid().data()), QString::fromUtf8(item.name().data()) , 2);
};


void RzxJabberClient::itemUnavailable(RosterItem & item, const std::string &msg){
	emit presence(QString::fromUtf8(item.jid().data()), QString::fromUtf8(item.name().data()) , 0);
};


void RzxJabberClient::itemChanged(RosterItem & item, const std::string &msg){
	emit presence(QString::fromUtf8(item.jid().data()), QString::fromUtf8(item.name().data()) , 1);
};

void RzxJabberClient::itemUpdated(const std::string &jid){
	emit rosterUpdated();
};

void RzxJabberClient::itemRemoved(const std::string &jid){
	emit rosterUpdated();
};

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
	emit msgReceived(QString::fromUtf8(stanza->from().bare().data()) , QString::fromUtf8(stanza->body().data()) );
}


void RzxJabberClient::handlePresence( Stanza *stanza ){

}

void RzxJabberClient::handleLog( const std::string& xml, bool incoming ){
};

bool RzxJabberClient::send(Tag* t){
	if(j && isStarted()){
		j->send(t);
		return true;
	}
	return false;
};

/***************************************************************************
                          rzxclientlistener.cpp  -  description
                             -------------------
    begin                : Sat Jan 26 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qsocketnotifier.h>
#include <qregexp.h>
#include "rzxmessagebox.h"

#include "rzxclientlistener.h"
#include "rzxconfig.h"
	
const char * RzxClientListener::DCCFormat[] = {
	"^PROPQUERY.*",
	"^PROPANSWER .*",
	"^CHAT",
	0
};


RzxClientListener * RzxClientListener::globalObject = 0;
RzxClientListener* RzxClientListener::object() {
	if (!globalObject)
		globalObject = new RzxClientListener;
		
	return globalObject;
}


bool RzxClientListener::isValid( void ) const { return valid; }


RzxClientListener::RzxClientListener()
	: QObject(0, "Client"), udpSocket(QSocketDevice::Datagram)
		
{
	bufferSize = DCC_MSGSIZE;
	buffer = new char[bufferSize];

	valid = false;

	// Supprim'e automatiquement (qt rules !)
	QSocketNotifier * notify = new QSocketNotifier(udpSocket.socket(), QSocketNotifier::Read, this);
	notify -> setEnabled(true);
	connect(notify, SIGNAL(activated(int)), SLOT(socketRead(int)));
}

void RzxClientListener::enforceBufferSize( unsigned long size )
{
	if (bufferSize < size) {
		delete buffer;
		buffer = new char[size];
		bufferSize = size;
	}
}		

bool RzxClientListener::listenOnPort(Q_UINT32 port) {
	valid = false;
	if( !udpSocket.isValid() ){
		qDebug("udp socket not valid");
		return false;
	}

	udpSocket.setReceiveBufferSize(DCC_MSGSIZE * 2);
	udpSocket.setSendBufferSize(DCC_MSGSIZE * 2);
	
	if( !udpSocket.bind(QHostAddress(), port) ){
		qDebug("Could not bind to socket");
		return false;
	}
		
	udpSocket.setBlocking(false);
	udpSocket.setAddressReusable(false);

	valid = true;
	return true;
}

RzxClientListener::~RzxClientListener(){
}


/** No descriptions */
void RzxClientListener::socketRead(int socket){
	// On sait jamais
	if( socket != udpSocket.socket() ) {
		qDebug("assertion socket!=udpSocket.socket() failed!");
		return;
	}

	unsigned long size = udpSocket.bytesAvailable();

	QString msg;

	enforceBufferSize( size );
	udpSocket.readBlock(buffer, size);
		
		// on verifie que le message est bien termine par 0
	unsigned int idx;
	for (idx = 0; idx < size; idx++) {
		if (!buffer[idx]) break;
	}
	if (idx == size){
		qDebug("Message does not end with an \\0!!!");
		return;
	}
			
	msg = buffer;
	parse(msg);
}


/** No descriptions */
void RzxClientListener::parse(const QString& msg){
	qDebug("Parsing message: "+msg);
	int i = 0;
	QRegExp cmd; 	QString arg;
	int offset = msg.find(" ");
	if(offset >=0)
		arg = msg.right(msg.length() - offset).stripWhiteSpace();
	
	while(DCCFormat[i]) {
		cmd.setPattern(DCCFormat[i]);
#if (QT_VERSION >= 0x030000)
		if (cmd.search(msg, 0) >= 0) {
#else
		if (cmd.find(msg, 0) >= 0) {
#endif
			switch(i) {
			case DCC_PROPQUERY:
				emit propQuery(udpSocket.peerAddress());
				break;
			case DCC_PROPANSWER:
				if(arg.isEmpty() || !WaitingForProperties )		// si il n'y a pas les donnees necessaires 
					return;										// ou que l'on n'a rien demande on s'arrete
				emit propAnswer(udpSocket.peerAddress(), arg);
				WaitingForProperties = false;
				break;
			case DCC_CHAT:
				emit chat(udpSocket.peerAddress(), arg);
				break;
			};
		}
		i++;
	}
};

/** No descriptions */
void RzxClientListener::sendPropQuery(const RzxHostAddress& host) {
	if( !valid ) return;
	memset(buffer, 0, DCC_MSGSIZE);
	strcpy(buffer, "PROPQUERY");
	udpSocket.writeBlock(buffer, DCC_MSGSIZE, host, udpSocket.port());
	WaitingForProperties = true;
}


void RzxClientListener::sendProperties(const RzxHostAddress &peer)
{
	if( !valid ) return;
	QStringList strList;
	strList << tr("Surname") << RzxConfig::propName();
	strList << tr("First name") << RzxConfig::propLastName();
	strList << tr("Nick") << RzxConfig::propSurname();
	strList << tr("Phone") << RzxConfig::propTel();
	strList << tr("E-Mail") << RzxConfig::propMail();
	strList << tr("Web") << RzxConfig::propWebPage();
	strList << tr("Room") << RzxConfig::propCasert();
	strList << tr("Sport") << RzxConfig::propSport();
	strList << tr("Promo") << RzxConfig::propPromo();

	QString msg = strList.join("|");
	sendPropAnswer(peer, msg);
}
/** No descriptions */
void RzxClientListener::sendPropAnswer(const RzxHostAddress& host, const QString& msg){
	if( !valid ) return;
	memset(buffer, 0, DCC_MSGSIZE);
	
	QString temp = "PROPANSWER " + msg;
	if (temp.length() + 1 >= DCC_MSGSIZE)
		return;
	strcpy(buffer, temp.latin1());
	udpSocket.writeBlock(buffer, DCC_MSGSIZE, host, udpSocket.port());
}

/** No descriptions */
void RzxClientListener::sendChat(const RzxHostAddress& host, const QString& msg)
{
	emit chatSent();
	sendDccChat( host, msg );
}


void RzxClientListener::sendResponder(const RzxHostAddress& host, const QString& msg)
{
	sendDccChat( host, msg );
}


void RzxClientListener::sendDccChat(const RzxHostAddress& host, const QString& msg) {
	if( !valid ) return;
	QString remaining = msg;
	
	while (remaining.length() + 6 > DCC_MSGSIZE) {
		// Essaie d'envoyer le plus possible de lignes dans le message DCC
		int whereToCut = remaining.find( "<br>", DCC_MSGSIZE-6 );

		QString cutLeft;

		// Si la premiere ligne fait deja plus de DCC_MSGSIZE-6, on la coupe
		if( whereToCut < 0 )
		{
			whereToCut = DCC_MSGSIZE - 6 - 3;
			cutLeft = remaining.left( whereToCut ) + "...";
			remaining = "..." + remaining.mid( whereToCut );
		}
		else
		{
			cutLeft = remaining.left( whereToCut );
			remaining = remaining.mid( whereToCut + 4 );
		}

        QString temp = "CHAT " + cutLeft;
		
		enforceBufferSize( temp.length()+1 );
		strcpy(buffer, temp.latin1());
		udpSocket.writeBlock(buffer, temp.length() + 1, host, udpSocket.port());
	}

	QString temp = "CHAT " + remaining;

	enforceBufferSize( temp.length()+1 );
	strcpy(buffer, temp.latin1());
	udpSocket.writeBlock(buffer, temp.length() + 1, host, udpSocket.port());
}

/** No descriptions */
void RzxClientListener::close(){
	udpSocket.close();
}

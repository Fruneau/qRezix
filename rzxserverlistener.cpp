/***************************************************************************
                          rzxserverlistener.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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
#include <qregexp.h>
#include <qstringlist.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qregexp.h>

#include <stdlib.h>

#include "rzxserverlistener.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"


RzxServerListener * RzxServerListener::globalObject = 0;
RzxServerListener * RzxServerListener::object() {
	if (!globalObject)
		globalObject = new RzxServerListener;
	
	return globalObject;
}

RzxServerListener::RzxServerListener()
	: RzxProtocole("Serveur"),
		socket(0, "ServerSocket") {
	connect(&reconnection, SIGNAL(timeout()), this, SLOT(slotConnect()));	
	
	connect(this, SIGNAL(ping()), this, SLOT(sendPong()));
	connect(this, SIGNAL(ping()), this, SLOT(serverResetTimer()));
	connect(&socket, SIGNAL(bytesWritten(int)), this, SLOT(serverResetTimer()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverResetTimer()));
	connect(this, SIGNAL(send(const QString&)), this, SLOT(sendProtocolMsg(const QString&)));
	
	connect(&socket, SIGNAL(connectionClosed()), this, SLOT(serverClose()));
	connect(&socket, SIGNAL(error(int)), this, SLOT(serverError(int)));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverReceive()));
	
	connect(&socket, SIGNAL(hostFound()), this, SLOT(serverFound()));
	connect(&socket, SIGNAL(connected()), this, SLOT(serverConnected()));
	connect(&socket, SIGNAL(connected()), this, SLOT(beginAuth()));
	
	connect(&socket, SIGNAL(delayedCloseFinished()), this, SIGNAL(disconnected()));
	connect(&socket, SIGNAL(connectionClosed()), this, SIGNAL(disconnected()));
	connect(&socket, SIGNAL(error(int)), this, SIGNAL(disconnected()));
	connect(&socket, SIGNAL(connected()), this, SIGNAL(connected()));

	connect (&pingTimer, SIGNAL(timeout()), this, SLOT(serverTimeout()));
	
	premiereConnexion=1;
}


RzxServerListener::~RzxServerListener(){
}

void RzxServerListener::setupConnection() {
	slotConnect();
}

void RzxServerListener::setupReconnection(const QString& msg, bool fatal) {
	emit disconnected();

	int time = RzxConfig::reconnection();
	QString temp;
	if(premiereConnexion){
		premiereConnexion=0;
		reconnection.changeInterval(500);
		}
	else if (time) {
		time += rand() * 30000 / RAND_MAX;
		temp = msg + tr("Will try to reconnect in %1 seconds").arg(time/1000);
		emit status(temp, fatal);
		reconnection.changeInterval(time);
	}
}

/** Erreur ï¿½la conenction au serveur. On rï¿½ssaie en SERVER_RECONNECTION ms */
void RzxServerListener::serverError(int error) {
	pingTimer.stop();
	
	switch(error) {
	case QSocket::ErrConnectionRefused:
		setupReconnection(tr("Connexion refused"),true);
		break;
	case QSocket::ErrHostNotFound:
		emit status(tr("Host not found. Manual search..."), true);

		/* Recherche à la main du serveur. Pas gégène, mais ça marche. */
		if( !alternateGetHostByName() )
		{
			emit status(tr("Cannot find server %1").arg(RzxConfig::serverName()), true);

			QMessageBox::information( NULL, "qRezix",
				tr("Cannot find server %1:\n\nDNS request failed").arg(RzxConfig::serverName()));
		}
		break;

	case QSocket::ErrSocketRead:
		setupReconnection(tr("Socket error"), true);
		break;

	default:
		setupReconnection(tr("Unknown QSocket error: %1").arg(error), true);
	}

}

void RzxServerListener::serverClose() {
	setupReconnection(tr("Connection closed"), true);
}

/** Appelï¿½quand il n'y a pas eu de ping depuis plus de RzxConfig::pingTimeout() ms */
void RzxServerListener::serverTimeout(){
	setupReconnection(tr("Connection lost"), true);
}

void RzxServerListener::slotConnect(){
	iconMode = false;
	reconnection.stop();
	
	QString serverHostname = RzxConfig::serverName();
	unsigned long serverPort = RzxConfig::serverPort();
	if (serverHostname.isEmpty() || !serverPort) {
		emit status(tr("Server name and port are not configured"), true);
		return;
	}
			
	emit status(tr("Looking for server %1").arg(serverHostname), false);
	socket.connectToHost(serverHostname, serverPort);	
}

/** no comment. */
#ifdef WIN32
#include <winsock2.h>
#else //!WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif //WIN32

/** #### oui, c'est synchrone, donc bloquant. ####
    Et en plus c'est assez crade. */
bool RzxServerListener::alternateGetHostByName( void )
{
	const char * hostname = RzxConfig::serverName().latin1();
	struct hostent * p = gethostbyname( hostname );
	if( p )
	{
		struct in_addr sin_addr;
		sin_addr.s_addr = *(unsigned long *) p->h_addr;
		QString addr( inet_ntoa( sin_addr ) );

		emit status(tr("Looking for server %1").arg(addr), false);
		socket.connectToHost(addr, RzxConfig::serverPort());	
		return true;
    }

	return false;
}

void RzxServerListener::serverFound() {
	emit status(tr("Server found, trying to connect"), false);
}

/** No descriptions */
void RzxServerListener::serverConnected(){
	emit status(tr("Connected"), false);
	pingTimer.start(RzxConfig::pingTimeout());
}

/** No descriptions */
void RzxServerListener::beginAuth(){
	RzxComputer * localhost = RzxConfig::localHost();
	if (!localhost) {
		QMessageBox::critical(0, tr("qReziX error"), tr("Configuration error, cannot connect"));
		pingTimer.stop();
		return;
	}
		
	sendAuth(RzxConfig::pass(), localhost);
	
}

void RzxServerListener::serverReceive() {
	QString temp;
	QImage image(64, 64, 32), swapImg;
//	QPixmap pix;
	
	while(socket.canReadLine() || iconMode) {
		if (iconMode && socket.bytesAvailable() < ICON_SIZE)
			return;
		
		if (iconMode) {
			char iconBuffer[ICON_SIZE];
			socket.readBlock(iconBuffer, ICON_SIZE);

			// on convertit l'image recue pour la rendre affichable
			unsigned char * src = (unsigned char *) iconBuffer;	
			for (int scanline = 0; scanline < ICON_HEIGHT; scanline++) {
				unsigned char * line = image.scanLine(scanline);
				for (int pixel = 0; pixel < ICON_WIDTH; pixel++) {
					*line = *src;
					*(++line) = *(++src);
					*(++line) = *(++src);
					*(++line) = 255;
					++line; src++;
				}
			}
			swapImg = image.swapRGB();
			emit rcvIcon(&swapImg, iconHost);
			iconMode = false;
		}
		
		if (socket.canReadLine()) {
			temp = socket.readLine();
			parse(temp.left(temp.length() - 2));
		}
	}
}


void RzxServerListener::parse(const QString& msg) {
	QRegExp cmd;
	cmd.setPattern(RzxProtocole::ServerFormat[RzxProtocole::SERVER_ICON]);
#if (QT_VERSION >= 0x030000)
	if (cmd.search(msg, 0) >= 0) {
#else
	if (cmd.find(msg, 0) >= 0) {
#endif
		// on supprime l'en-tete du message
		QString msgClean = msg, msgParams;
		msgClean.stripWhiteSpace();
		int offset = msgClean.find(" ");
		if (offset >= 0)
			msgParams = msgClean.right(msgClean.length() - offset);
		else return;
			
		bool ok;	
		iconHost = RzxHostAddress::fromRezix(msgParams.stripWhiteSpace().toULong(&ok, 16));
		if (!ok)
			return;
		iconMode = true;
	}
	else {
		RzxProtocole::parse(msg);
	}
}

/** Change l'icone de l'ordinateur local */
void RzxServerListener::sendIcon(const QImage& image) {
	if (image.isNull()) return;
	// on convertit l'image recue pour la rendre affichable
	// on fait les copies par bloc de 4 octets --> besoin d'un peu de rab
//	QImage workImage = image.convertDepth(32,0).swapRGB();
	QImage workImage = image.convertDepth(32,0).smoothScale(ICON_WIDTH, ICON_HEIGHT);
//  workImage = workImage.mirror(false, true);

  unsigned char buffer[ICON_SIZE];
  unsigned char * dest;
  QRgb *src;
  memset(buffer, 255, ICON_SIZE);
  for (int scanline = 0; scanline < ICON_HEIGHT; scanline++) {
  	dest = buffer + ICON_WIDTH * scanline * 3;
  	src = (QRgb*) workImage.scanLine(scanline);
  	for (int pix = 0; pix < ICON_WIDTH; pix++) {
	  	dest[pix * 3] = qRed(src[pix]);
  		dest[pix * 3 + 1] = qGreen(src[pix]);
  		dest[pix * 3 + 2] = qBlue(src[pix]);
  	}
  }			
	sendProtocolMsg("UPLOAD\r\n");
	socket.writeBlock((char *)buffer, ICON_SIZE);
}

/** No descriptions */
void RzxServerListener::sendProtocolMsg(const QString& msg){	
	const char * strMsg = msg.latin1();
	if (socket.writeBlock(strMsg, msg.length()) != (int)msg.length())
		emit status(tr("Socket error, cannot write"), true);
}

/** No descriptions */
void RzxServerListener::serverResetTimer(){
	pingTimer.stop();
	pingTimer.start(RzxConfig::pingTimeout());
}

/** No descriptions */
bool RzxServerListener::isSocketClosed() const{
	return (socket.state() != QSocket::Connection);// || (socket.state() == QSocket::Closing);
}
/** No descriptions */
void RzxServerListener::close(){
	disconnect(&socket, SIGNAL(connectionClosed()), this, SLOT(serverClose()));
	disconnect(&socket, SIGNAL(error(int)), this, SLOT(serverError(int)));
	if (isSocketClosed()) {
		emit disconnected();
		return;
	}
	
	connect(&socket, SIGNAL(delayedCloseFinished()), this, SLOT(closeWaitFlush()));
	sendPart();
	socket.close();
	
	// delayedCloseFinished chie dans la colle (ou il y a un truc que je n'ai pas vu)
	// donc je met un flag ï¿½VRAI et j'attends qu'il ait fini d'ï¿½rire
	// une fois que le buffer de sortie est vide, je ferme tout
}

/** No descriptions */
void RzxServerListener::closeWaitFlush(){
	emit disconnected();
}
/** No descriptions */
RzxHostAddress RzxServerListener::getIP() const{
	return RzxHostAddress(socket.address());
	//TODO récupérer cette adresse du serveur
}


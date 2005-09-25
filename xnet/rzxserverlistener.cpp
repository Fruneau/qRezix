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
#include <QRegExp>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>

#include <RzxMessageBox>
#include <RzxComputer>
#include <RzxIconCollection>

#include "rzxserverlistener.h"
#include "rzxxnetconfig.h"


RZX_NETWORK_EXPORT(RzxServerListener)

///Construction du Socket
RzxServerListener::RzxServerListener()
	: RzxProtocole(), socket() {
	beginLoading();
	new RzxXNetConfig(this);
	connect(&reconnection, SIGNAL(timeout()), this, SLOT(waitReconnection()));	
	
	connect(this, SIGNAL(ping()), this, SLOT(sendPong()));
	connect(this, SIGNAL(ping()), this, SLOT(serverResetTimer()));
	connect(&socket, SIGNAL(bytesWritten(qint64)), this, SLOT(serverResetTimer()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverResetTimer()));
	connect(this, SIGNAL(send(const QString&)), this, SLOT(sendProtocolMsg(const QString&)));
	
	connect(&socket, SIGNAL(disconnected()), this, SLOT(serverClose()));
	connect(&socket, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(serverError(QTcpSocket::SocketError)));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverReceive()));
	
	connect(&socket, SIGNAL(hostFound()), this, SLOT(serverFound()));
	
	connect(&socket, SIGNAL(disconnected()), this, SIGNAL(disconnected(this)));
	connect(&socket, SIGNAL(connected()), this, SIGNAL(connected(this)));
	connect(&socket, SIGNAL(connected()), this, SLOT(serverConnected()));
	connect(&socket, SIGNAL(connected()), this, SLOT(beginAuth()));

	connect(&pingTimer, SIGNAL(timeout()), this, SLOT(serverTimeout()));
	
	premiereConnexion = true;
	hasBeenConnected = true;
	endLoading();
}


RzxServerListener::~RzxServerListener()
{
	beginClosing();
	delete RzxXNetConfig::global();
	disconnect(&socket, SIGNAL(disconnected()), 0, 0);
	endClosing();
}

void RzxServerListener::start() {
	connectToXnetserver();
}

void RzxServerListener::setupReconnection(const QString& msg) {
	if(reconnection.isActive()) return;

	emit disconnected(this);

	unsigned int time = RzxXNetConfig::reconnection();
	QString temp;
	if(premiereConnexion)
	{
		premiereConnexion = false;
		timeToConnection = 500;
		reconnection.setSingleShot(true);
		reconnection.start(500);
		notify(msg);
	}
	else
	{
		//on fait un random du temps entre 2 tentatives de connexion dans [time/2;3*time/2]
		if(!time) time = 60000;
		time >>= 1;
		if(time)
			time += rand() % (time<<1);
		temp = msg + "... " + tr("will try to reconnect in %1 seconds").arg(time/1000);
		message = msg;
		timeToConnection =  time;
		reconnection.setSingleShot(false);
		reconnection.start(1000);
		notify(temp);
	}
}

///Pour afficher le d�ompte avant la tentative de reconnexion
void RzxServerListener::waitReconnection()
{
	timeToConnection -= 1000;
	if(timeToConnection <= 0)
	{
		reconnection.stop();
		connectToXnetserver();
	}
	else
		notify(message + "... " + tr("will try to reconnect in %1 seconds").arg(timeToConnection/1000));
}

/** Erreur �la conenction au serveur. On r�ssaie en SERVER_RECONNECTION ms */
void RzxServerListener::serverError(QTcpSocket::SocketError error) {
	pingTimer.stop();
	QString reconnectionMsg;
	
	switch(error) {
		case QTcpSocket::ConnectionRefusedError:
			reconnectionMsg = tr("Connection refused");
			break;
		
		case QTcpSocket::RemoteHostClosedError:
			reconnectionMsg = tr("Connection reset by peer");
			break;

		case QTcpSocket::HostNotFoundError:
			reconnectionMsg = tr("Unable to find server %1").arg(RzxXNetConfig::serverName());

			if(hasBeenConnected)
				RzxMessageBox::information( NULL, "qRezix",
					tr("Cannot find server %1:\n\nDNS request failed").arg(RzxXNetConfig::serverName()));
			hasBeenConnected = false;
			break;

		case QTcpSocket::SocketAccessError:
			reconnectionMsg = tr("Socket access denied");
			break;

		case QTcpSocket::SocketResourceError:
			reconnectionMsg = tr("Too many sockets");
			break;

		case QTcpSocket::SocketTimeoutError:
			reconnectionMsg = tr("Operation timeout");
			break;

		case QTcpSocket::NetworkError:
			reconnectionMsg = tr("Network down");
			break;

		case QTcpSocket::UnsupportedSocketOperationError:
			reconnectionMsg = tr("Unsupported operation");
			break;

		default:
			reconnectionMsg = tr("Unknown QSocket error: %1").arg(error);
	}
	setupReconnection(reconnectionMsg);
	qDebug("Server socket error : %s", socket.errorString().toAscii().constData());
}

void RzxServerListener::serverClose() {
	setupReconnection(tr("Connection closed"));
}

/** Appel� quand il n'y a pas eu de ping depuis plus de RzxXNetConfig::pingTimeout() ms */
void RzxServerListener::serverTimeout(){
	setupReconnection(tr("Connection lost"));
}

void RzxServerListener::connectToXnetserver()
{
	iconMode = false;
	reconnection.stop();
	
	QString serverHostname = RzxXNetConfig::serverName();
	quint16 serverPort = RzxXNetConfig::serverPort();
	if (serverHostname.isEmpty() || !serverPort) {
		notify(tr("Server name and port are not configured"));
		return;
	}
	
	socket.connectToHost(serverHostname, serverPort);	
	notify(tr("Looking for server %1").arg(serverHostname));
}

void RzxServerListener::serverFound()
{
	notify(tr("Server found, trying to connect"));
}

/** No descriptions */
void RzxServerListener::serverConnected(){
	notify(tr("Connected"));
	hasBeenConnected = true;
	pingTimer.start(RzxXNetConfig::pingTimeout());
	emit receiveAddress(getIP());
}

void RzxServerListener::serverReceive() {
	QString temp;
	QImage image(64, 64, QImage::Format_ARGB32), swapImg;
	
	while(socket.canReadLine() || iconMode) {
		if (iconMode && socket.bytesAvailable() < ICON_SIZE)
			return;
		
		if (iconMode) {
			char iconBuffer[ICON_SIZE];
			socket.read(iconBuffer, ICON_SIZE);

			// on convertit l'image recue pour la rendre affichable
			unsigned char * src = (unsigned char *) iconBuffer;	
			for (int scanline = 0; scanline < ICON_HEIGHT; scanline++) {
				QRgb* line = (QRgb*)image.scanLine(scanline);
				for (int pixel = 0; pixel < ICON_WIDTH; pixel++) {
					char r,v,b,a;
					r = *(src++);
					v = *(src++);
					b = *(src++);
					a = *(src++);
					*(line++) = qRgba(r, v, b, a);
				}
			}
			emit receivedIcon(&image, iconHost);
			iconMode = false;
		}
		
		if (socket.canReadLine()) {
			temp = socket.readLine();
			if(temp.contains("\r\n"))
				parse(temp);
		}
	}
}

///Parsage des donn�s re�s du serveur
/** Permet de r�artir entre les donn�s brutes (ic�es) et les donn�s 'protocolaires' qui elles sont g��s par RzxProtocole */
void RzxServerListener::parse(const QString& msg)
{
	QRegExp cmd;
	
	/* R�eption d'une ic�e... passe l'�ute en mode attente de donn�s brutes */
	cmd.setPattern(RzxProtocole::ServerFormat[RzxProtocole::SERVER_ICON]);
	if(cmd.indexIn(msg, 0) >= 0)
	{
		// on supprime l'en-tete du message
		QString msgClean = msg, msgParams;
		msgClean.trimmed();
		int offset = msgClean.indexOf(" ");
		if (offset >= 0)
			msgParams = msgClean.right(msgClean.length() - offset);
		else return;
			
		bool ok;	
		iconHost = RzxHostAddress::fromRezix(msgParams.trimmed().toUInt(&ok, 16));
		if (!ok)
			return;
		iconMode = true;
		return;
	}
	
	/* Envoie de l'ic�e suite �la requ�e du serveur */
	cmd.setPattern(RzxProtocole::ServerFormat[RzxProtocole::SERVER_UPLOAD]);
	if(cmd.indexIn(msg) != -1)
	{
		sendIcon(RzxIconCollection::global()->localhostPixmap().toImage());
		return;
	}
	
	/* Gestion du protocol autre */
	RzxProtocole::parse(msg);
}

/** Change l'icone de l'ordinateur local */
void RzxServerListener::sendIcon(const QImage& image) {
//	if (image.isNull()) return;
	// on convertit l'image recue pour la rendre affichable
	// on fait les copies par bloc de 4 octets --> besoin d'un peu de rab
//	QImage workImage = image.convertDepth(32,0);
	QImage workImage;
	if(!image.isNull())
		workImage = image.scaled(ICON_WIDTH, ICON_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	else
		return;

	unsigned char buffer[ICON_SIZE];
	unsigned char * dest;
	QRgb *src;
	for (int scanline = 0; scanline < ICON_HEIGHT; scanline++) {
		dest = buffer + ICON_WIDTH * scanline * 4;
		src = (QRgb*) workImage.scanLine(scanline);
		for (int pix = 0; pix < ICON_WIDTH; pix++) {
			*(dest++) = qRed(src[pix]);
			*(dest++) = qGreen(src[pix]);
			*(dest++) = qBlue(src[pix]);
			*(dest++) = qAlpha(src[pix]);
		}
	}
	sendProtocolMsg("UPLOAD\r\n");
	socket.write((char *)buffer, ICON_SIZE);
}

/** No descriptions */
void RzxServerListener::sendProtocolMsg(const QString& msg){	
	if(socket.write(msg.toLatin1()) != (int)msg.length())
		notify(tr("Socket error, cannot write"));
}

/** No descriptions */
void RzxServerListener::serverResetTimer(){
	pingTimer.stop();
	pingTimer.start(RzxXNetConfig::pingTimeout());
}

/** No descriptions */
bool RzxServerListener::isStarted() const
{
	return (socket.state() == QTcpSocket::ConnectedState);
}

/** No descriptions */
void RzxServerListener::stop()
{
	disconnect(&socket, SIGNAL(connectionClosed()), this, SLOT(serverClose()));
	disconnect(&socket, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(serverError(QTcpSocker::SocketError)));
	if (isStarted()) {
		emit disconnected(this);
		return;
	}
	
	connect(&socket, SIGNAL(disconnected()), this, SLOT(closeWaitFlush()));
	sendPart();
	socket.close();
	
	// delayedCloseFinished chie dans la colle (ou il y a un truc que je n'ai pas vu)
	// donc je met un flag �VRAI et j'attends qu'il ait fini d'�rire
	// une fois que le buffer de sortie est vide, je ferme tout
}

/** No descriptions */
void RzxServerListener::closeWaitFlush(){
	emit disconnected(this);
}

/** No descriptions */
RzxHostAddress RzxServerListener::getServerIP() const
{
	return RzxHostAddress(socket.peerAddress());
}

/** No descriptions */
RzxHostAddress RzxServerListener::getIP() const{
	return RzxHostAddress(socket.localAddress());
	//TODO r�up�er cette adresse du serveur
}


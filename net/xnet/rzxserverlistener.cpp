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
	: RzxProtocole(), socket()
{
	beginLoading();
	new RzxXNetConfig(this);
	connect(&reconnection, SIGNAL(timeout()), this, SLOT(waitReconnection()));	
	
	connect(&socket, SIGNAL(bytesWritten(qint64)), this, SLOT(serverResetTimer()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverResetTimer()));
	
	connect(&socket, SIGNAL(disconnected()), this, SLOT(serverClose()));
	connect(&socket, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(serverError(QTcpSocket::SocketError)));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(serverReceive()));
	
	connect(&socket, SIGNAL(hostFound()), this, SLOT(serverFound()));
	
	connect(&socket, SIGNAL(disconnected()), this, SIGNAL(emitDisconnected()));
	connect(&socket, SIGNAL(connected()), this, SIGNAL(emitConnected()));
	connect(&socket, SIGNAL(connected()), this, SLOT(serverConnected()));
	connect(&socket, SIGNAL(connected()), this, SLOT(beginAuth()));

	connect(&pingTimer, SIGNAL(timeout()), this, SLOT(serverTimeout()));
	
	premiereConnexion = true;
	hasBeenConnected = true;
	restarting = false;
	endLoading();
}

///On ferme tout et on s'en va...
RzxServerListener::~RzxServerListener()
{
	beginClosing();
	delete RzxXNetConfig::global();
	disconnect(&socket, SIGNAL(disconnected()), 0, 0);
	endClosing();
}

/** \reimp */
void RzxServerListener::start()
{
	if(restarting)
	{
		restarting = false;
		disconnect(&socket, SIGNAL(disconnected()), this, SLOT(start()));
	}
	connectToXnetserver();
}

/** \reimp */
void RzxServerListener::restart()
{
	if(!isStarted())
	{
		qDebug() << "Restarting xNet 1";
		start();
	}
	else
	{
		qDebug() << "Restarting xNet 2";
		restarting = true;
		connect(&socket, SIGNAL(disconnected()), this, SLOT(start()));
		stop();
	}
}

///Initialise la reconnexion au serveur
void RzxServerListener::setupReconnection(const QString& msg) {
	if(reconnection.isActive()) return;

	emitDisconnected();

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

///Pour afficher le décompte avant la tentative de reconnexion
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

///Gestion des erreurs du socket
/** Erreur à la connexion au serveur. On rééssaie en SERVER_RECONNECTION ms */
void RzxServerListener::serverError(QTcpSocket::SocketError error)
{
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

///On vient d'être déconnecté, on tente de relancer la connexion
void RzxServerListener::serverClose()
{
	if(!restarting)
		setupReconnection(tr("Connection closed"));
}

///La connexion avec le serveur a timeouté
/** Appelée quand il n'y a pas eu de ping depuis plus de RzxXNetConfig::pingTimeout() ms */
void RzxServerListener::serverTimeout(){
	setupReconnection(tr("Connection lost"));
}

///Initialise la connexion avec le serveur xNet
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

///On a trouvé le serveur...
void RzxServerListener::serverFound()
{
	notify(tr("Server found, trying to connect"));
}

///On est connecté...
void RzxServerListener::serverConnected()
{
	notify(tr("Connected"));
	hasBeenConnected = true;
	pingTimer.start(RzxXNetConfig::pingTimeout());
	emit receiveAddress(getIP());
}

///On reçoit des données depuis le serveurs
/** Cette partie gère la réception de l'icône, alors que la gestion propre au protocol
 * se trouve dans RzxProtocole.
 */
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

///Parsage des donnï¿½s reï¿½s du serveur
/** Permet de rï¿½artir entre les donnï¿½s brutes (icï¿½es) et les donnï¿½s 'protocolaires' qui elles sont gï¿½ï¿½s par RzxProtocole */
void RzxServerListener::parse(const QString& msg)
{
	QRegExp cmd;
	
	/* Rï¿½eption d'une icï¿½e... passe l'ï¿½ute en mode attente de donnï¿½s brutes */
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
	
	/* Envoie de l'icï¿½e suite ï¿½la requï¿½e du serveur */
	cmd.setPattern(RzxProtocole::ServerFormat[RzxProtocole::SERVER_UPLOAD]);
	if(cmd.indexIn(msg) != -1)
	{
		sendIcon(RzxIconCollection::global()->localhostPixmap().toImage());
		return;
	}
	
	/* Gestion du protocol autre */
	RzxProtocole::parse(msg);
}

///On a reçu un ping
/** On envoie pong et on réinitialise le timer d'attente de ping
 */
void RzxServerListener::pingReceived()
{
	sendPong();
	serverResetTimer();
}

/** Change l'icone de l'ordinateur local */
void RzxServerListener::sendIcon(const QImage& image) {
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
	send("UPLOAD\r\n");
	socket.write((char *)buffer, ICON_SIZE);
}

/** No descriptions */
void RzxServerListener::send(const QString& msg)
{	
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
	if(isStarted())
	{
		emitDisconnected();
		return;
	}
	
	connect(&socket, SIGNAL(disconnected()), this, SLOT(closeWaitFlush()));
	sendPart();
	socket.close();
	
	// delayedCloseFinished chie dans la colle (ou il y a un truc que je n'ai pas vu)
	// donc je met un flag ï¿½VRAI et j'attends qu'il ait fini d'ï¿½rire
	// une fois que le buffer de sortie est vide, je ferme tout
}

/** No descriptions */
void RzxServerListener::closeWaitFlush(){
	emitDisconnected();
}

/** No descriptions */
RzxHostAddress RzxServerListener::getServerIP() const
{
	return RzxHostAddress(socket.peerAddress());
}

/** No descriptions */
RzxHostAddress RzxServerListener::getIP() const{
	return RzxHostAddress(socket.localAddress());
	//TODO rï¿½upï¿½er cette adresse du serveur
}


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
#define RZX_MODULE_NAME "xNet"
#define RZX_MODULE_DESCRIPTION "Native support for the xNet protocole version 4.0"
#define RZX_MODULE_ICON RzxThemedIcon(Rzx::ICON_NETWORK)

#include <QRegExp>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QMessageBox>

#include <RzxMessageBox>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxApplication>

#include "rzxserverlistener.h"
#include "rzxxnetconfig.h"


RZX_NETWORK_EXPORT(RzxServerListener)

///Construction du Socket
RzxServerListener::RzxServerListener()
	: RzxProtocole(), socket()
{
	beginLoading();
	new RzxXNetConfig(this);

	connect(&pingTimer, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(&socket, SIGNAL(bytesWritten(qint64)), this, SLOT(haveActivity()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(haveActivity()));
	connect(&socket, SIGNAL(readyRead()), this, SLOT(read()));
	
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
	connect(&socket, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
	connect(&socket, SIGNAL(connected()), this, SLOT(serverConnected()));
	
	connect(&socket, SIGNAL(hostFound()), this, SLOT(serverFound()));

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
		disconnect(this, SIGNAL(disconnected(RzxNetwork*)), this, SLOT(start()));
	}

	userConnection = true;
	reconnection = None;
	wantDisconnection = false;

	connectToXnetserver();
}

/** \reimp */
void RzxServerListener::stop()
{
	if(!isStarted() && valid) return;

	wantDisconnection = true;
	if(valid)
		sendPart();
	else
		socket.close();
}

/** \reimp */
void RzxServerListener::restart()
{
	if(!isStarted())
		start();
	else
	{
		restarting = true;
		connect(this, SIGNAL(disconnected(RzxNetwork*)), this, SLOT(start()));
		stop();
	}
}

///Initialise la connexion avec le serveur xNet
void RzxServerListener::connectToXnetserver()
{
	iconMode = false;
	auth = false;
	valid = false;
	sendingBuffer = QString();
	reconnectionTimer.stop();
	
	QString serverHostname = RzxXNetConfig::serverName();
	quint16 serverPort = RzxXNetConfig::serverPort();
	if (serverHostname.isEmpty() || !serverPort)
	{
		notify(tr("Server name and port are not configured"));
		return;
	}
	
	socket.connectToHost(serverHostname, serverPort);	
	notify(tr("Looking for server %1").arg(serverHostname));
}

///Initialise la reconnexion au serveur
void RzxServerListener::setupReconnection(const QString& msg)
{
	if(reconnectionTimer.isActive()) return;

	unsigned int time = RzxXNetConfig::reconnection();
	disconnect(&reconnectionTimer, SIGNAL(timeout()), this, SLOT(waitReconnection()));
	if(reconnection == None)
	{
		timeToReconnection = 500;
		reconnectionTimer.setSingleShot(true);
		reconnectionTimer.start(500);
		notify(msg);
	}
	else
	{
		//on fait un random du temps entre 2 tentatives de connexion dans [time/2;3*time/2]
		if(!time) time = 60000;
		time >>= 1;
		if(time)
			time += rand() % (time<<1);
		timeToReconnection = time;

		connect(&reconnectionTimer, SIGNAL(timeout()), this, SLOT(waitReconnection()));
		reconnectionTimer.setSingleShot(false);
		reconnectionTimer.start(1000);

		notify(msg + "... " + tr("will try to reconnect in %1 seconds").arg(time/1000));
		message = msg;
	}
}

///Pour afficher le décompte avant la tentative de reconnexion
void RzxServerListener::waitReconnection()
{
	timeToReconnection -= 1000;
	if(timeToReconnection <= 0)
	{
		reconnectionTimer.stop();
		connectToXnetserver();
	}
	else
		notify(message + "... " + tr("will try to reconnect in %1 seconds").arg(timeToReconnection/1000));
}

///Gestion des erreurs du socket
/** Erreur à la connexion au serveur. On rééssaie en SERVER_RECONNECTION ms */
void RzxServerListener::error(QAbstractSocket::SocketError error)
{
	pingTimer.stop();
	QString reconnectionMsg;
	
	switch(error) 
	{
		case QTcpSocket::ConnectionRefusedError:
			reconnectionMsg = tr("Connection refused");
			break;
		
		case QTcpSocket::RemoteHostClosedError:
			reconnectionMsg = tr("Connection reset by peer");
			break;

		case QTcpSocket::HostNotFoundError:
			reconnectionMsg = tr("Unable to find server %1").arg(RzxXNetConfig::serverName());

			if(reconnection == Normal)
				RzxMessageBox::information( NULL, "qRezix",
					tr("Cannot find server %1:\n\nDNS request failed").arg(RzxXNetConfig::serverName()));
			reconnection = Quiet;
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
	serverDisconnected(reconnectionMsg, true);
}

///La connexion avec le serveur a timeouté
/** Appelée quand il n'y a pas eu de ping depuis plus de RzxXNetConfig::pingTimeout() ms */
void RzxServerListener::timeout()
{
	socket.close();
	serverDisconnected(tr("Connection lost"));
}

///On vient d'être déconnecté, on tente de relancer la connexion
void RzxServerListener::serverDisconnected(const QString& msg, bool force)
{
	if((reconnectionTimer.isActive() || isStarted()) && !force) return;

	emit disconnected(this);
	auth = false;
	if(!wantDisconnection)
	{
		if(!msg.isNull())
			setupReconnection(msg);
		else
			setupReconnection(tr("Connection closed"));
	}
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
	userConnection = false;
	reconnection = Normal;

	haveActivity();
	emit connected(this);
	emit receiveAddress(getIP());

	//Envoie du buffer
	if(!sendingBuffer.isEmpty())
	{
		send(sendingBuffer);
		sendingBuffer = QString();
	}

	beginAuth();
}

///On reçoit des données depuis le serveurs
/** Cette partie gère la réception de l'icône, alors que la gestion propre au protocol
 * se trouve dans RzxProtocole.
 */
void RzxServerListener::read()
{
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

///Parsage des données reçues du serveur
/** Permet de répartir entre les données brutes (icônes) 
 * et les données 'protocolaires' qui elles sont gérées par RzxProtocole 
 */
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
	haveActivity();
}

/** Change l'icone de l'ordinateur local */
void RzxServerListener::sendIcon(const QImage& image)
{
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
	if(!isStarted())
		sendingBuffer += msg;
	else
	{
		if(socket.write(msg.toLatin1()) != (int)msg.length())
			notify(tr("Socket error, cannot write"));
	}
}

/** No descriptions */
void RzxServerListener::haveActivity()
{
	pingTimer.stop();
	pingTimer.start(RzxXNetConfig::pingTimeout());
}

/** No descriptions */
bool RzxServerListener::isStarted() const
{
	return (socket.state() == QTcpSocket::ConnectedState);
}

///Retourne l'adresse du serveur
RzxHostAddress RzxServerListener::getServerIP() const
{
	return RzxHostAddress(socket.peerAddress());
}

///Retourne l'adresse locale
RzxHostAddress RzxServerListener::getIP() const
{
	return RzxHostAddress(socket.localAddress());
}


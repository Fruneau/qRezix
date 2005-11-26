/***************************************************************************
                          rzxchatsocket  -  description
                             -------------------
    begin                : Sun Jul 24 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//Pour les communications
#include <QRegExp>

#include <RzxConfig>
#include <RzxComputer>
#include <RzxApplication>

#include "rzxchatsocket.h"

#include "rzxchat.h"
#include "rzxclientlistener.h"
#include "rzxchatlister.h"
#include "rzxchatconfig.h"

//attention a toujours avoir DCCFormat[DCC_message] = messageFormat
QString RzxChatSocket::DCCFormat[] = {
	"(^PROPQUERY )\r\n",
	"(^PROPANSWER )(.*)\r\n",
	"(^CHAT )(.*)\r\n",
	"(^PING )\r\n",
	"(^PONG )\r\n",
	"(^TYPING )(0|1)\r\n",
	0
};

///Construction d'un socket brute
RzxChatSocket::RzxChatSocket()
	:QTcpSocket(), host()
{
	alone = false;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(SocketError)), this, SLOT(chatConnexionError(SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
}

///Construction d'un socket de chat sans liaison
RzxChatSocket::RzxChatSocket(RzxComputer *c, bool salone)
	:QTcpSocket(), host(c->ip())
{
	alone = salone;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(SocketError)), this, SLOT(chatConnexionError(SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	if(alone)
		connectToHost();
	RzxClientListener::global()->attach(this);
}

///Destruction d'un socket de chat
RzxChatSocket::~RzxChatSocket()
{
}

///Fermeture du socket
void RzxChatSocket::close()
{
	QTcpSocket::close();
	deleteLater();
}

///Connexion à l'hôte
void RzxChatSocket::connectToHost()
{
	QTcpSocket::connectToHost(host.toString(), RzxChatConfig::chatPort());
	timeOut.start(10*1000); //descend le timeout de connexion à 10s
}

///Installation d'un socket
void RzxChatSocket::setSocketDescriptor(int socket)
{
	QTcpSocket::setSocketDescriptor(socket);
	host = peerAddress();
	RzxClientListener::global()->attach(this);
}

////Parser des messages
/** C'est cette méthode qui va vraiment faire le tri entre un chat et une demande de propriété. Lorsqu'un chat est envoyé, le message est émis vers rzxrezal qui alors redonne le message à la bonne fenêtre de chat si elle existe, ou la crée dans le cas contraire */
int RzxChatSocket::parse(const QString& msg)
{
	int i = 0;
	QRegExp cmd;
	if(!msg.contains("\r\n")) //recherche de la fin du message
		return -1;
	
	while(!DCCFormat[i].isNull())
	{
		cmd.setPattern(DCCFormat[i]);
		if(cmd.indexIn(msg, 0) != -1) 
		{
			switch(i) {
				case DCC_PROPQUERY:
					qDebug("Parsing PROPQUERY");
					sendProperties();
					return DCC_PROPQUERY;
					break;

				case DCC_PROPANSWER:
					qDebug() << "Parsing PROPANSWER:" << cmd.cap(2);
					if(cmd.cap(2).isEmpty())					// si il n'y a pas les donnees necessaires 
					{
						emit notify(tr("has send empty properties"));
						return DCC_PROPANSWER;		// ou que l'on n'a rien demande on s'arrete
					}
					host.computer()->setProperties(cmd.cap(2));
					emit haveProperties(host);
					if(alone)
						close();
					return DCC_PROPANSWER;
					break;

				case DCC_CHAT:
					qDebug("Parsing CHAT : %s", cmd.cap(2).toAscii().constData());
					if(RzxConfig::autoResponder())
						sendResponder(RzxConfig::autoResponderMsg());
					if(RzxComputer::localhost()->state() == Rzx::STATE_REFUSE)
						return DCC_CHAT;
					host.computer()->receiveChat(Rzx::Chat, cmd.cap(2));
					host.computer()->receiveChat(Rzx::StopTyping);
					return DCC_CHAT;
					break;

				case DCC_PING:
					sendPong();
					qDebug("Parsing PING");
					host.computer()->receiveChat(Rzx::Ping);
					return DCC_PING;
					break;

				case DCC_PONG:
					host.computer()->receiveChat(Rzx::Pong);
					qDebug("Parsing PONG");
					return DCC_PONG;
					break;

				case DCC_TYPING:
					host.computer()->receiveChat(cmd.cap(2)=="1" ? Rzx::Typing: Rzx::StopTyping);
					qDebug("Parsing TYPING");
					return DCC_TYPING;
					break;
			};
		}
		i++;
	}
	return -1;
}

/*Les méthodes qui suivent servent à l'émission des différents messages*/
///Envoi d'une demande de propriété
void RzxChatSocket::sendPropQuery() {
	send("PROPQUERY \r\n\0");
}

///Envoi d'une requête ping
void RzxChatSocket::sendPing()
{
	pongTime = QTime::currentTime();
	send("PING \r\n");
}

///Envoi d'une réponse pong
void RzxChatSocket::sendPong()
{
	send("PONG \r\n");
}

///Envoi de l'état de la frappe
void RzxChatSocket::sendTyping(bool state)
{
	QString msg = "TYPING ";
	msg += (state?"1":"0");
	send(msg + "\r\n");
}

///Formatage des propriétés de l'utilisateur
void RzxChatSocket::sendProperties()
{
	QStringList strList;
	strList << tr("Name") << RzxConfig::propName();
	strList << tr("Last name") << RzxConfig::propLastName();
	strList << tr("Nick") << RzxConfig::propSurname();
	strList << tr("Phone") << RzxConfig::propTel();
	strList << tr("E-Mail") << RzxConfig::propMail();
	strList << tr("Web Page") << RzxConfig::propWebPage();
	strList << tr("Room") << RzxConfig::propCasert();
	strList << tr("Sport") << RzxConfig::propSport();
	strList << tr("Class") << RzxConfig::propPromo();

	QString msg = strList.join("|");
	send("PROPANSWER " + msg + "\r\n\0");
	emit propertiesSent(host);
}

///Emission d'un message de chat
/** Composition du message de chat envoyé par l'utilisateur, pour qu'il soit envoyé au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendChat(const QString& msg)
{
	sendDccChat(msg);
	sendTyping(false);
}

///Emission du message du répondeur automatique
/** Composition du message de chat indiquant que l'utilisateur est sur répondeur, pour qu'il soit envoyé au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendResponder(const QString& msg)
{
	sendDccChat(msg);
}

///Envoi d'un message de chat
/** Met en forme un message de chat quelconque pour qu'il soit envoyé. Cette méthode ajoute la part lié au protocole au message.
 * <br>Il faut utiliser \ref sendChat() pour envoyer un chat.
 */
void RzxChatSocket::sendDccChat(const QString& msg) {
	send(QString("CHAT " + msg + "\r\n\0"));
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket défini.*/
void RzxChatSocket::send(const QString& msg)
{
	switch(state())
	{
		case QAbstractSocket::ConnectedState:
			if(write(msg.toLatin1()) == -1)
			{
				qDebug("Impossible d'émettre les données vers ");
				host.computer()->receiveChat(Rzx::InfoMessage, tr("Unable to send data... writeBlock returns -1"));
			}
			else
			{
				flush();
				qDebug("Message envoyé : %s", msg.left(msg.length()-2).toAscii().constData());
			}
			return;
			
		case QAbstractSocket::UnconnectedState:
			connectToHost();	
		default:
			tmpChat += msg;
			return;
	}
}

///Lecture d'un message en attente sur le socket sock
/** Cette méthode lit un message envoyé sur un socket particulier et balance directement vers le parser pour que le message soit interprété*/
int RzxChatSocket::readSocket()
{
	QString msg;
	int p = -1;

	if(!canReadLine()) return -1;

	while(canReadLine())
	{
		msg = readLine();
		if(msg.contains("\r\n"))
		{
			if(p == -1) p = parse(msg);
		}
	}
	return p;
}

/** Emission du message lorsque la connexion est établie */
void RzxChatSocket::chatConnexionEtablished()
{
	qDebug("Socket ouvert vers %s... envoi du message", host.toString().toAscii().constData());
	if(alone)
		sendPropQuery();
	else if(!tmpChat.isNull())
	{
		send(tmpChat);
		tmpChat = QString::null;
	}
	timeOut.stop();
	host = peerAddress();
}

/**La connexion a été fermée (sans doute par fermeture de la fenêtre de chat) on l'indique à l'utilisateur */
void RzxChatSocket::chatConnexionClosed()
{
	if(!alone) host.computer()->receiveChat(Rzx::Closed);
	qDebug("Connection with %s closed by peer", host.toString().toAscii().constData());
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxChatSocket::chatConnexionError(SocketError error)
{
	switch(error)
	{
		case ConnectionRefusedError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("can't be contacted, check his firewall... CONNECTION ERROR"));
			qDebug("Connexion has been refused by the client");
			close();
			break;
		case HostNotFoundError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("can't be found... CONNECTION ERROR"));
			qDebug("Can't find client");
			close();
			break;
		case QAbstractSocket::RemoteHostClosedError:
			break;
//		case SocketRead:
		default:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("has sent corrupt data... CONNECTION ERROR"));
			qDebug("Error while reading datas %d", error);
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

///Cas où la connexion n'a pas pu être établie dans les délais
void RzxChatSocket::chatConnexionTimeout()
{
	chatConnexionError(SocketTimeoutError);
}

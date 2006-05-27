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
	"^PROPQUERY \r\n",
	"^PROPANSWER (.*)\r\n",
	"^CHAT (.*)\r\n",
	"^PING \r\n",
	"^PONG \r\n",
	"^TYPING (0|1)\r\n",
	0
};

///Construction d'un socket brute
RzxChatSocket::RzxChatSocket()
	:QTcpSocket(), host()
{
	alone = false;
	connect(this, SIGNAL(disconnected()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(chatConnexionError(QAbstractSocket::SocketError)));
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
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(chatConnexionError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	if(alone)
		connectToHost();
	RzxChatLister::global()->listener()->attach(this);
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

///Connexion � l'h�te
void RzxChatSocket::connectToHost()
{
	QTcpSocket::connectToHost(host.toString(), RzxChatConfig::chatPort());
	timeOut.start(10*1000); //descend le timeout de connexion � 10s
}

///Installation d'un socket
void RzxChatSocket::setSocketDescriptor(int socket)
{
	QTcpSocket::setSocketDescriptor(socket);
	host = peerAddress();
	RzxChatLister::global()->listener()->attach(this);
}

///Parser des messages
/** C'est cette m�thode qui va vraiment faire le tri entre un chat et une demande de propri�t�. Lorsqu'un chat est envoy�, le message est �mis vers rzxrezal qui alors redonne le message � la bonne fen�tre de chat si elle existe, ou la cr�e dans le cas contraire */
int RzxChatSocket::parse(const QString& msg)
{
	int i = 0;
	QRegExp cmd;
	if(!msg.contains("\r\n")) //recherche de la fin du message
		return -1;
	
	while(!DCCFormat[i].isNull())
	{
		cmd.setPattern(DCCFormat[i]);
		if(cmd.indexIn(msg) != -1) 
		{
			switch(i) {
				case DCC_PROPQUERY:
					sendProperties();
					return DCC_PROPQUERY;
					break;

				case DCC_PROPANSWER:
					if(cmd.cap(1).isEmpty())					// si il n'y a pas les donnees necessaires 
					{
						emit notify(tr("has send empty properties"));
						return DCC_PROPANSWER;		// ou que l'on n'a rien demande on s'arrete
					}
					host.computer()->setProperties(cmd.cap(1));
					emit haveProperties(host);
					if(alone)
						close();
					return DCC_PROPANSWER;
					break;

				case DCC_CHAT:
					if(RzxConfig::autoResponder())
						sendResponder(RzxConfig::autoResponderMsg());
					if(RzxComputer::localhost()->state() == Rzx::STATE_REFUSE)
						return DCC_CHAT;
					host.computer()->receiveChat(Rzx::Chat, cmd.cap(1));
					host.computer()->receiveChat(Rzx::StopTyping);
					return DCC_CHAT;
					break;

				case DCC_PING:
					sendPong();
					host.computer()->receiveChat(Rzx::Ping);
					return DCC_PING;
					break;

				case DCC_PONG:
					host.computer()->receiveChat(Rzx::Pong, QString::number(pongTime.msecsTo(QTime::currentTime())));
					return DCC_PONG;
					break;

				case DCC_TYPING:
					host.computer()->receiveChat(cmd.cap(1)=="1" ? Rzx::Typing: Rzx::StopTyping);
					return DCC_TYPING;
					break;
			}
		}
		i++;
	}
	return -1;
}

/*Les m�thodes qui suivent servent � l'�mission des diff�rents messages*/
///Envoi d'une demande de propri�t�
void RzxChatSocket::sendPropQuery()
{
	send("PROPQUERY \r\n");
}

///Envoi d'une requ�te ping
void RzxChatSocket::sendPing()
{
	pongTime = QTime::currentTime();
	send("PING \r\n");
}

///Envoi d'une r�ponse pong
void RzxChatSocket::sendPong()
{
	send("PONG \r\n");
}

///Envoi de l'�tat de la frappe
void RzxChatSocket::sendTyping(bool state)
{
	QString msg = "TYPING ";
	msg += (state?"1":"0");
	send(msg + "\r\n");
}

///Formatage des propri�t�s de l'utilisateur
void RzxChatSocket::sendProperties()
{
	send("PROPANSWER " + RzxComputer::localhost()->properties() + "\r\n");
	emit propertiesSent(host);
}

///Emission d'un message de chat
/** Composition du message de chat envoy� par l'utilisateur, pour qu'il soit envoy� au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendChat(const QString& msg)
{
	sendDccChat(msg);
	sendTyping(false);
}

///Emission du message du r�pondeur automatique
/** Composition du message de chat indiquant que l'utilisateur est sur r�pondeur, pour qu'il soit envoy� au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendResponder(const QString& msg)
{
	if(!msg.simplified().isEmpty())
		sendDccChat(msg);
}

///Envoi d'un message de chat
/** Met en forme un message de chat quelconque pour qu'il soit envoy�. Cette m�thode ajoute la part li� au protocole au message.
 * <br>Il faut utiliser \ref sendChat() pour envoyer un chat.
 */
void RzxChatSocket::sendDccChat(const QString& msg) {
	QString message = msg;
	send(QString("CHAT " + message.remove('\r').replace('\n', "<br>") + "\r\n"));
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket d�fini.*/
void RzxChatSocket::send(const QString& msg)
{
	switch(state())
	{
		case QAbstractSocket::ConnectedState:
			if(write(msg.toLatin1()) == -1)
			{
				qDebug("Impossible d'�mettre les donn�es vers ");
				host.computer()->receiveChat(Rzx::InfoMessage, tr("Unable to send data... writeBlock returns -1"));
			}
			else
				flush();
			return;
			
		case QAbstractSocket::UnconnectedState:
			connectToHost();	
		default:
			tmpChat += msg;
			return;
	}
}

///Lecture d'un message en attente sur le socket sock
/** Cette m�thode lit un message envoy� sur un socket particulier et balance directement vers le parser pour que le message soit interpr�t�*/
void RzxChatSocket::readSocket()
{
	QString msg;

	if(!canReadLine()) return;

	while(canReadLine())
	{
		msg = readLine();
		if(msg.contains("\r\n"))
		{
			QStringList list = msg.split("\r\n");
			for(int i = 0 ; i < list.size() ; i++)
			{
				if(parse(list[i] + "\r\n") == -1)
					break;
			}
		}
	}
}

/** Emission du message lorsque la connexion est �tablie */
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

/**La connexion a �t� ferm�e (sans doute par fermeture de la fen�tre de chat) on l'indique � l'utilisateur */
void RzxChatSocket::chatConnexionClosed()
{
	RzxComputer *computer = host.computer();
	if(!alone && computer)
		computer->receiveChat(Rzx::Closed);
	qDebug("Connection with %s closed by peer", host.toString().toAscii().constData());
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxChatSocket::chatConnexionError(QAbstractSocket::SocketError error)
{
	switch(error)
	{
		case ConnectionRefusedError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("Connection error: connection refused"));
			qDebug("Connection has been refused by the client");
			close();
			break;
		case HostNotFoundError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("Connection error: host not found"));
			qDebug("Can't find client");
			close();
			break;
		case QAbstractSocket::SocketTimeoutError:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("Connection error: timeout"));
			qDebug("Connection timeout");
			close();
			break;
		case QAbstractSocket::RemoteHostClosedError:
			break;
		default:
			host.computer()->receiveChat(Rzx::InfoMessage, tr("Connection error: data corruption"));
			qDebug("Error while reading datas %d", error);
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

///Cas o� la connexion n'a pas pu �tre �tablie dans les d�lais
void RzxChatSocket::chatConnexionTimeout()
{
	chatConnexionError(SocketTimeoutError);
}

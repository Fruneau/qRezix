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
#include <qsocket.h>
#include "rzxmessagebox.h"

#include "rzxclientlistener.h"
#include "rzxconfig.h"
#include "rzxchat.h"

//attention a toujours avoir DCCFormat[DCC_message] = messageFormat
QString RzxChatSocket::DCCFormat[] = {
	"(^PROPQUERY )\r\n",
	"(^PROPANSWER )(.*)\r\n",
	"(^CHAT )(.*)\r\n",
	"(^PING )\r\n",
	"(^PONG )\r\n",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

///Construction d'un socket brute
RzxChatSocket::RzxChatSocket()
	:QSocket(), host()
{
	alone = false;
	chatWindow = NULL;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::object()->attach(this);
}

///Construction d'un socket de chat lié à une fenêtre
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, RzxChat *parent)
	:QSocket(), host(s_host)
{
	chatWindow = parent;
	alone = false;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	RzxClientListener::object()->attach(this);
}

///Construction d'un socket de chat sans liaison
RzxChatSocket::RzxChatSocket(const RzxHostAddress& s_host, bool s_alone)
	:QSocket(), host(s_host)
{
	chatWindow = NULL;
	alone = s_alone;
	connect(this, SIGNAL(connectionClosed()), this, SLOT(chatConnexionClosed()));
	connect(this, SIGNAL(readyRead()), this, SLOT(readSocket()));
	connect(this, SIGNAL(error(int)), this, SLOT(chatConnexionError(int)));
	connect(this, SIGNAL(connected()), this, SLOT(chatConnexionEtablished()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(chatConnexionTimeout()));
	if(alone)
		connectToHost();
	RzxClientListener::object()->attach(this);
}

///Destruction d'un socket de chat
RzxChatSocket::~RzxChatSocket()
{
}

///Fermeture du socket
void RzxChatSocket::close()
{
	QSocket::close();
	if(chatWindow)
		chatWindow->setSocket(NULL);
	chatWindow = NULL;
	deleteLater();
}

///Liaison du socket à un chat
void RzxChatSocket::setParent(RzxChat *parent)
{
	chatWindow = parent;
}

///Connexion à l'hôte
void RzxChatSocket::connectToHost()
{
	QSocket::connectToHost(host.toString(), RzxConfig::chatPort());
}

///Installation d'un socket
void RzxChatSocket::setSocket(int socket)
{
	QSocket::setSocket(socket);
	host = peerAddress();
}

////Parser des messages
/** C'est cette méthode qui va vraiment faire le tri entre un chat et une demande de propriété. Lorsqu'un chat est envoyé, le message est émis vers rzxrezal qui alors redonne le message à la bonne fenêtre de chat si elle existe, ou la crée dans le cas contraire */
int RzxChatSocket::parse(const QString& msg)
{
	int i = 0;
	QRegExp cmd;
	int fin = msg.find("\r\n");     //recherche de la fin du message
	if(fin == -1) return -1;
	
	while(DCCFormat[i])
	{
		cmd.setPattern(DCCFormat[i]);
		if(cmd.search(msg, 0) >= 0) 
		{
			switch(i) {
				case DCC_PROPQUERY:
					qDebug("Parsing PROPQUERY");
					sendProperties();
					return DCC_PROPQUERY;
					break;
				case DCC_PROPANSWER:
					qDebug("Parsing PROPANSWER: " + cmd.cap(2));
					if(cmd.cap(2).isEmpty())					// si il n'y a pas les donnees necessaires 
					{
						emit notify(tr("has send empty properties"));
						return DCC_PROPANSWER;		// ou que l'on n'a rien demande on s'arrete
					}
					if(!chatWindow)
						emit propAnswer(host, cmd.cap(2));
					else
						chatWindow->receiveProperties(cmd.cap(2));
					if(alone)
						close();
					return DCC_PROPANSWER;
					break;
				case DCC_CHAT:
					qDebug("Parsing CHAT: " + cmd.cap(2));
					if(!chatWindow)
						emit chat(this, cmd.cap(2));
					else
						emit chat(cmd.cap(2));
					return DCC_CHAT;
					break;
				case DCC_PING:
					sendPong();
					qDebug("Parsing PING");
					emit notify(tr("Ping received"));
					break;
				case DCC_PONG:
					emit pongReceived(pongTime.msecsTo(QTime::currentTime()));
					qDebug("Parsing PONG");
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

///Formatage des propriétés de l'utilisateur
void RzxChatSocket::sendProperties()
{
	RzxHostAddress peer = peerAddress();
	
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
	send("PROPANSWER " + msg + "\r\n\0");
	emit propertiesSent(peer);
}

///Emission d'un message de chat
/** Composition du message de chat envoyé par l'utilisateur, pour qu'il soit envoyé au correspondant
 * <br>Voir aussi : \ref sendDccChat()
 */
void RzxChatSocket::sendChat(const QString& msg)
{
	emit chatSent();
	sendDccChat(msg);
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
//	if( !valid ) return;

	send(QString("CHAT " + msg + "\r\n\0"));
}

///Emission d'un message vers un autre client
/** Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket défini.*/
void RzxChatSocket::send(const QString& msg)
{
	switch(state())
	{
		case Connected:
			if(writeBlock(msg.latin1(), (msg.length())) == -1)
				qDebug("Impossible d'émettre les données vers ");
			else
			{
				flush();
				qDebug("Message envoyé : " + msg.left(msg.length()-2));
			}
			return;
			
		case Idle:
			connectToHost();
		default:
			tmpChat = msg;
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
		if(msg.find("\r\n") != -1)
		{
			if(p == -1) p = parse(msg);
		}
	}
	return p;
}

/** Emission du message lorsque la connexion est établie */
void RzxChatSocket::chatConnexionEtablished()
{
	qDebug("Socket ouvert vers " + host.toString() + "... envoi du message");
	if(alone)
		sendPropQuery();
	else if(tmpChat)
	{
		send(tmpChat);
		tmpChat = QString::null;
	}
	timeOut.stop();
}

/**La connexion a été fermée (sans doute par fermeture de la fenêtre de chat) on l'indique à l'utilisateur */
void RzxChatSocket::chatConnexionClosed()
{
	emit info(tr("ends the chat"));
	qDebug("Connection with " + host.toString() + " closed by peer");
	close();
}

/** Gestion des erreurs lors de la connexion et de la communication chaque erreur donne lieu a une mise en garde de l'utilisateur*/
void RzxChatSocket::chatConnexionError(int Error)
{
	switch(Error)
	{
		case ErrConnectionRefused:
			emit info(tr("can't be contact, check his firewall... CONNECTION ERROR"));
			qDebug("Connexion has been refused by the client");
			close();
			break;
		case ErrHostNotFound:
			emit info(tr("can't be found... CONNECTION ERROR"));
			qDebug("Can't find client");
			close();
			break;
		case ErrSocketRead:
			emit info(tr("has sent datas which can't be read... CONNECTION ERROR"));
			qDebug("Error while reading datas");
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

//Cas où la connexion n'a pas pu être établie dans les délais
void RzxChatSocket::chatConnexionTimeout()
{
	chatConnexionError(ErrConnectionRefused);
}






RzxClientListener * RzxClientListener::globalObject = 0;
RzxClientListener* RzxClientListener::object() {
	if (!globalObject)
		globalObject = new RzxClientListener;
		
	return globalObject;
}


bool RzxClientListener::isValid( void ) const { return valid; }


RzxClientListener::RzxClientListener()
	: QObject(0, "Client"), listenSocket(QSocketDevice::Stream)
{
	valid = false;
}

///Ouverture de l'écoute du port 5050
/** Ouverture du port tcp 5050 (par défaut) pour une écoute*/
bool RzxClientListener::listenOnPort(Q_UINT32 port) {
	valid = false;
	if( !listenSocket.isValid() ){
		qDebug("tcp socket not valid");
		return false;
	}

	if( !listenSocket.bind(QHostAddress(), port) ){
		qDebug("Could not bind to socket");
		return false;
	}
		
	listenSocket.setBlocking(false);
	listenSocket.setAddressReusable(false);
	if( !listenSocket.listen(50) ) //bon 50 pourquoi pas...
	{
		qDebug("Could not listen on listenSocket");
		return false;
	}

	valid = true;
	
	QSocketNotifier * notify = new QSocketNotifier(listenSocket.socket(), QSocketNotifier::Read, this);
	notify -> setEnabled(true);
	connect(notify, SIGNAL(activated(int)), SLOT(socketRead(int)));
	return true;
}

RzxClientListener::~RzxClientListener(){
}

void RzxClientListener::close()
{
	listenSocket.close();
}

///Connexion d'un RzxChatSocket au reste du programme
void RzxClientListener::attach(RzxChatSocket *sock)
{
	connect(sock, SIGNAL(propAnswer(const RzxHostAddress&, const QString& )), this, SIGNAL(propAnswer(const RzxHostAddress&, const QString& )));
	connect(sock, SIGNAL(propertiesSent(const RzxHostAddress& )), this, SIGNAL(propertiesSent(const RzxHostAddress& )));
	connect(sock, SIGNAL(chat(QSocket*, const QString& )), this, SIGNAL(chat(QSocket*, const QString& )));
	connect(sock, SIGNAL(chatSent()), this, SIGNAL(chatSent()));
}

///Réception d'une connexion entrante
/**Ce slot est appelé dès que l'écoute enregistre une demande d'écriture sur le port tcp 5050
 *		-# On récupère le socket de la connexion avec le client
 *		-# On analyse les données envoyées jusqu'à obtenir un message 'compréhensible'
 *		-# On dispatch alors ce message sur les différentes possibilités (pour l'instant chat ou prop)
 */
void RzxClientListener::socketRead(int socket){
	QHostAddress host;
	RzxChatSocket *sock;
	
	// On sait jamais
	if( socket != listenSocket.socket() ) {
		qDebug("assertion socket!=listenSocket.socket() failed!");
		return;
	}

	sock = new RzxChatSocket();
	sock->setSocket(listenSocket.accept());
	host = sock->peerAddress();
	qDebug("Accept connexion to client " + host.toString());
}

///Demande des propriétés de manière indépendante
/** Crée un socket pour la demande des propriétés à l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	new RzxChatSocket(host, true);
}

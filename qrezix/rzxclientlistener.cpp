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

//attention a toujours avoir DCCFormat[DCC_message] = messageFormat
const char * RzxClientListener::DCCFormat[] = {
	"^PROPQUERY \r\n",
	"^PROPANSWER .*\r\n",
	"^CHAT .*\r\n",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
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
	: QObject(0, "Client"), listenSocket(QSocketDevice::Stream), propSocket(0, "propCheck"), timeOut(0, "propCheckTimeOut")
{
	bufferSize = DCC_MSGSIZE;
	buffer = new char[bufferSize];

	valid = false;

	// Supprim'e automatiquement (qt rules !)
	QSocketNotifier * notify = new QSocketNotifier(listenSocket.socket(), QSocketNotifier::Read, this);
	notify -> setEnabled(true);
	connect(notify, SIGNAL(activated(int)), SLOT(socketRead(int)));

	//connexion du checker de propri�t�s.
	connect(&propSocket, SIGNAL(error(int)), this, SLOT(propCheckError(int)));
	connect(&propSocket, SIGNAL(readyRead()), this, SLOT(receivePropCheck()));
	connect(&propSocket, SIGNAL(connectionClosed()), this, SLOT(propCheckDeconnected()));
	connect(&propSocket, SIGNAL(connected()), this, SLOT(propCheckConnected()));
	connect(&timeOut, SIGNAL(timeout()), this, SLOT(propCheckTimeout()));
}

/* J'ai laiss� cette m�thode et le champ buffer de la classe pour le cas
o� on laisserait une �coute sur le port udp... �a peut toujours servir*/
void RzxClientListener::enforceBufferSize( unsigned long size )
{
	if (bufferSize < size)
	{
		delete buffer;
		buffer = new char[size];
		bufferSize = size;
	}
}		

/*Ouverture du port tcp 5050 (par d�faut) pour une �coute*/
bool RzxClientListener::listenOnPort(Q_UINT32 port) {
	valid = false;
	if( !listenSocket.isValid() ){
		qDebug("tcp socket not valid");
		return false;
	}

	listenSocket.setReceiveBufferSize(DCC_MSGSIZE * 2);
	listenSocket.setSendBufferSize(DCC_MSGSIZE * 2);
	
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
	return true;
}

RzxClientListener::~RzxClientListener(){
}


/*Ce slot est appel� d�s que l'�coute enregistre une demande d'�criture sur le port tcp 5050
		On r�cup�re le socket de la connexion avec le client
		On analyse les donn�es envoy�es jusqu'� obtenir un message 'compr�hensible'
		On dispatch alors ce message sur les diff�rentes possibilit�s (pour l'instant chat ou prop)*/
void RzxClientListener::socketRead(int socket){
	QHostAddress host;
	QSocket *sock;
	
	// On sait jamais
	if( socket != listenSocket.socket() ) {
		qDebug("assertion socket!=listenSocket.socket() failed!");
		return;
	}

	sock = new QSocket();
	sock->setSocket(listenSocket.accept());
	host = sock->peerAddress();
	qDebug("Accept connexion to client " + host.toString());

	//J'ai mis cette boucle pour filtrer les messages parmi des parasites
	int i=0;
	int size;
	do
	{
		if(sock->bytesAvailable() || sock->waitForMore(10))
		{
			int p = readSocket(sock);
			if(p == DCC_PROPQUERY)
			{
				propSendSocket = sock;
				connect(propSendSocket, SIGNAL(connectionClosed()), this, SLOT(endSendProp()));
				return;
			}
			if(p != -1) return;
		}
		i++;
	}
	while(i<20);  //attent au maximum 1/5 de seconde un message coh�rent
	qDebug("The connection has been closed because unable to get reliable datas");
	sock->close();
	delete sock;
}

/*Parser des messages
C'est cette m�thode qui va vraiment faire le tri entre un chat et une demande de propri�t
Lorsqu'un chat est envoy�, le message est �mis vers rzxrezal qui alors redonne le message
� la bonne fen�tre de chat si elle existe, ou la cr�e dans le cas contraire*/
int RzxClientListener::parse(const QString& msg, QSocket* sock){
	RzxHostAddress host = sock->peerAddress();
	int i = 0;
	QRegExp cmd; 	QString arg;
	int offset = msg.find(" ");     //recherche de la fin de l'en-t�te
	int fin = msg.find("\r\n");     //recherche de la fin du message
	if(offset >=0 && fin>offset)
		arg = msg.mid(offset, fin - offset).stripWhiteSpace();  //extraction du corp du message
	
	while(DCCFormat[i]) {
		cmd.setPattern(DCCFormat[i]);
		if(cmd.search(msg, 0) >= 0) {
			switch(i) {
				case DCC_PROPQUERY:
					qDebug("Parsing PROPQUERY");
					sendProperties(sock);
					return DCC_PROPQUERY;
					break;
				case DCC_PROPANSWER:
					qDebug("Parsing PROPANSWER: " + arg);
					if(arg.isEmpty() || !WaitingForProperties )		// si il n'y a pas les donnees necessaires 
						return DCC_PROPANSWER;										// ou que l'on n'a rien demande on s'arrete
					emit propAnswer(host, arg);
					WaitingForProperties = false;
					return DCC_PROPANSWER;
					break;
				case DCC_CHAT:
					qDebug("Parsing CHAT: " + arg);
					emit chat(sock, arg);
					return DCC_CHAT;
					break;
			};
		}
		qDebug("Parsing Unknown : " + msg);
		i++;
	}
//	qDebug("Parsing UnKnown: " + msg); //on skip les message inconnus
	return -1;
}

/*Les m�thodes qui suivent servent � l'�mission des diff�rents messages*/
void RzxClientListener::sendPropQuery(QSocket* sock) {
	send(sock, "PROPQUERY \r\n\0");
	WaitingForProperties = true;
}


//formatage des propri�t�s de l'utilisateur
void RzxClientListener::sendProperties(QSocket* sock)
{
	RzxHostAddress peer = sock->peerAddress();
	
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
	send(sock, "PROPANSWER " + msg + "\r\n\0");
	emit propertiesSent(peer);
}

/*  
void RzxClientListener::sendPropAnswer(QSocket* sock, const QString& msg){
	QString temp;
	temp = QString("PROPANSWER " + msg + "\r\n");
	send(sock, temp);
} */

//emission d'un message de chat
void RzxClientListener::sendChat(QSocket* sock, const QString& msg)
{
	emit chatSent();
	sendDccChat(sock, msg);
}

//emission du message du r�pondeur automatique
void RzxClientListener::sendResponder(QSocket* sock, const QString& msg)
{
	sendDccChat(sock, msg);
}

//on utilise une m�me fonction pour l'envoi des deux messages pr�c�dent... c discutable
void RzxClientListener::sendDccChat(QSocket* sock, const QString& msg) {
//	if( !valid ) return;

	send(sock, QString("CHAT " + msg + "\r\n\0"));
}

/*Envoie d'un message QUI DOIT AVOIR ETE FORMATE AUPARAVANT par le socket d�fini*/
void RzxClientListener::send(QSocket* sock, const QString& msg)
{
	if(sock->writeBlock(msg.latin1(), (msg.length())) == -1)
		qDebug("Impossible d'�mettre les donn�es vers ");
	else
		qDebug("Message envoy� : "+msg);
}

/*Lecture d'un message en attente sur le socket sock*/
//balance directement vers le parser
int RzxClientListener::readSocket(QSocket* sock)
{
	QString msg;
	int p = -1;

	if(!sock->canReadLine()) return -1;

	msg = sock->readLine();
	if(msg.find("\r\n") != -1)
		p = parse(msg, sock);
	else
		p = -1;

	if(sock->canReadLine())
	{
		if(p == -1) return readSocket(sock);
		readSocket(sock);
	}
	return p;
	
/*	unsigned long size = sock->bytesAvailable();
	char *buf;
	int p = -1;

	buf = new char[size];
	sock->readBlock(buf, size);

	p = parse(QString(buf), sock);
	delete buf;
	return p; */
}

/**Les m�thodes et slots qui suivent servent � checker les propri�t�s lorsqu'aucun chat n'a �t
ouvert avec le client en question. Il a donc fallu r�impl�menter toutes les fonctions utiles � cet effet.
De fait, on a quelque peu des doublons avec la partie de rzxchat***/
//ouverture de la communication
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	qDebug("Connecting to host " + host.toString() + " for getting properties");
	propSocket.connectToHost(host.toString(), RzxConfig::chatPort());
	timeOut.start(1000, TRUE);
}

//D�s que la connexion est �tablie on envoie le PROPQUERY
void RzxClientListener::propCheckConnected()
{
	sendPropQuery(&propSocket);
	timeOut.stop();
}

//On attend d'avoir re�u la r�ponse avant de fermer la connexion
//C'EST LE DEMANDEUR DES PROPRIETES QUI DANS LE PROTOCOL ACTUEL FERME LA CONNEXION
void RzxClientListener::receivePropCheck()
{
	if(readSocket(&propSocket) == DCC_PROPANSWER)
		propSocket.close();
}

//La connexion a �t� perdue annormalement
void RzxClientListener::propCheckDeconnected()
{
	RzxMessageBox::warning(0, tr("Error"), tr("Connection has been anormally lost while getting properties"));
	qDebug("Lost connection with host while getting properties");
	propSocket.close();
}

//Gestion des erreurs de connexion
void RzxClientListener::propCheckError(int Error)
{
	switch(Error)
	{
		case QSocket::ErrConnectionRefused:
			RzxMessageBox::warning(0, tr("Error"), tr("Unable to set a connection\nHost may not have open the chat port from his firewall"));
			qDebug("Connection has been refused by the client for checking properties");
			break;
		case QSocket::ErrHostNotFound:
			RzxMessageBox::warning(0, tr("Error"), tr("Unable to find host"));
			qDebug("Can't find host for checking properties");
			break;
		case QSocket::ErrSocketRead:
			RzxMessageBox::warning(0, tr("Error"), tr("Can't read datas from host"));
			qDebug("Error while reading datas for checking properties");
			break;
	}
	if(timeOut.isActive()) timeOut.stop();
}

//La connexion n'a pas pu �tre �tablie dans les d�lais
void RzxClientListener::propCheckTimeout()
{
	propSocket.close();
	propCheckError(QSocket::ErrConnectionRefused);
}

/** Ce slot esseul�, g�re la partie inverse des slots pr�c�dents, en effet, il permet d'attendre que
l'emetteur de la demande de propri�t�s est ferm� sa connexion pour d�truire le socket **/
void RzxClientListener::endSendProp()
{
	qDebug("Properties sending connection has been closed");
	if(propSendSocket) delete propSendSocket;
}


/** No descriptions */
void RzxClientListener::close(){
	listenSocket.close();
}

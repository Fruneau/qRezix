/***************************************************************************
                  rzxclientlistener.cpp - description
                         -------------------
 begin             : Sat Jan 26 2002
 copyright         : (C) 2002 by Sylvain Joyeux
 email             : sylvain.joyeux@m4x.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <RzxMessageBox>
#include <RzxComputer>
#include <RzxConfig>
#include <RzxBanList>
#include <RzxApplication>

#include "rzxclientlistener.h"

#include "rzxchatsocket.h"
#include "rzxchat.h"

RzxClientListener::RzxClientListener()
{ }

RzxClientListener::~ RzxClientListener()
{
	close();
}

///Connexion d'un RzxChatSocket au reste du programme
void RzxClientListener::attach(RzxChatSocket *sock)
{
	sockets.insert(sock->peer(), sock);
	connect(sock, SIGNAL(propertiesSent(RzxComputer*)), this, SIGNAL(propertiesSent(RzxComputer*)));
	connect(sock, SIGNAL(chatSent()), this, SIGNAL(chatSent()));
	connect(sock, SIGNAL(haveProperties(RzxComputer*)), this, SIGNAL(haveProperties(RzxComputer*)));
}

///Réception d'une connexion entrante
/**Ce slot est appelé dès que l'écoute enregistre une demande d'écriture sur le port tcp 5050
 *		-# On récupère le socket de la connexion avec le client
 *		-# On analyse les données envoyées jusqu'à obtenir un message 'compréhensible'
 *		-# On dispatch alors ce message sur les différentes possibilités (pour l'instant chat ou prop)
 */
void RzxClientListener::incomingConnection(int socketDescriptor)
{
	//Récupération de la connexion
	RzxChatSocket *sock = new RzxChatSocket();
	sock->setSocketDescriptor(socketDescriptor);

	// On vérifie au passage que la connexion est valide
	QHostAddress host;
	host = sock->peerAddress();
	if(!RzxBanList::global()->contains(host))
		qDebug() << "New connection from " << host.toString() << ": accepted";
	else
	{
		qDebug() << "New connection from " << host.toString() << ": refused (in ban list)";
		sock->abort();
		delete sock;
	}
}

///Demande des propriétés de manière indépendante
/** Crée un socket pour la demande des propriétés à l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	RzxChatSocket *socket = sockets[host];
	if(!socket)
		connect(new RzxChatSocket(host, true), SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
	else
		socket->sendPropQuery();
}

///Permet l'affichage des messages d'erreur des socket conçus uniquement pour le check de propriétés
void RzxClientListener::info(const QString& msg)
{
	RzxMessageBox::information(RzxApplication::mainWindow(), tr("Connection error"), tr("An error occured while checking properties :\n") + msg);
}

///Envoie d'un message
void RzxClientListener::sendChatMessage(RzxComputer* computer, Rzx::ChatMessageType type, const QString& msg)
{
	if(!computer) return;

	RzxChatSocket *socket = sockets[computer->ip()];
	if(!socket && type != Rzx::Typing && type != Rzx::StopTyping && type != Rzx::Closed)
		socket = new RzxChatSocket(computer, false);

	if(!socket) return;

	switch(type)
	{
		case Rzx::Chat: socket->sendChat(msg); break;
		case Rzx::Responder: socket->sendResponder(msg); break;
		case Rzx::Ping: socket->sendPing(); break;
		case Rzx::Typing: socket->sendTyping(true); break;
		case Rzx::StopTyping: socket->sendTyping(false); break;
		case Rzx::Closed: socket->close(); break;
		default: break;
	}
}

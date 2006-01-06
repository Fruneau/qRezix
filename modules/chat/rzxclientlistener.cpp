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

///R�ception d'une connexion entrante
/**Ce slot est appel� d�s que l'�coute enregistre une demande d'�criture sur le port tcp 5050
 *		-# On r�cup�re le socket de la connexion avec le client
 *		-# On analyse les donn�es envoy�es jusqu'� obtenir un message 'compr�hensible'
 *		-# On dispatch alors ce message sur les diff�rentes possibilit�s (pour l'instant chat ou prop)
 */
void RzxClientListener::incomingConnection(int socketDescriptor)
{
	//R�cup�ration de la connexion
	RzxChatSocket *sock = new RzxChatSocket();
	sock->setSocketDescriptor(socketDescriptor);

	// On v�rifie au passage que la connexion est valide
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

///Demande des propri�t�s de mani�re ind�pendante
/** Cr�e un socket pour la demande des propri�t�s � l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	RzxChatSocket *socket = sockets[host];
	if(!socket)
		connect(new RzxChatSocket(host, true), SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
	else
		socket->sendPropQuery();
}

///Permet l'affichage des messages d'erreur des socket con�us uniquement pour le check de propri�t�s
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

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
#include "rzxclientlistener.h"

#include "rzxchatsocket.h"
#include "rzxmessagebox.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxchat.h"
#include "rzxapplication.h"

RzxClientListener * RzxClientListener::object = 0;

RzxClientListener::RzxClientListener()
{ }

RzxClientListener::~ RzxClientListener()
{
	close();
}

///Connexion d'un RzxChatSocket au reste du programme
void RzxClientListener::attach(RzxChatSocket *sock)
{
	connect(sock, SIGNAL(propertiesSent(const RzxHostAddress& )), this, SIGNAL(propertiesSent(const RzxHostAddress& )));
	connect(sock, SIGNAL(chatSent()), this, SIGNAL(chatSent()));
}

///Réception d'une connexion entrante
/**Ce slot est appelé dès que l'écoute enregistre une demande d'écriture sur le port tcp 5050
 *		-# On récupère le socket de la connexion avec le client
 *		-# On analyse les données envoyées jusqu'à obtenir un message 'compréhensible'
 *		-# On dispatch alors ce message sur les différentes possibilités (pour l'instant chat ou prop)
 */
void RzxClientListener::incomingConnection(int socketDescriptor) {
	//Récupération de la connexion
	RzxChatSocket *sock = new RzxChatSocket(socketDescriptor);
	sock->setSocketDescriptor(socketDescriptor);

	// On vérifie au passage que la connexion est valide
	QHostAddress host;
	host = sock->peerAddress();
	if(!RzxConfig::global()->isBan(host)) 
		qDebug("Accept connexion to client %s", host.toString().toAscii().constData());
	else {
		qDebug("Message from client %s has been ignored", host.toString().toAscii().constData());
		sock->abort();
		delete sock;
		return;
	}
}

///Demande des propriétés de manière indépendante
/** Crée un socket pour la demande des propriétés à l'utilisateur en face */
void RzxClientListener::checkProperty(const RzxHostAddress& host)
{
	connect(new RzxChatSocket(host, true), SIGNAL(info(const QString&)), this, SLOT(info(const QString&)));
}

///Permet l'affichage des messages d'erreur des socket conçus uniquement pour le check de propriétés
void RzxClientListener::info(const QString& msg)
{
	RzxMessageBox::information(RzxApplication::mainWindow(), tr("Connection error"), tr("An error occured while checking properties :\n") + msg);
}

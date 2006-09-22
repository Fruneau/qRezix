/***************************************************************************
	                  rzxfilelistener.cpp - description
                         -------------------
	begin             : Mon Jan 30 2006
	copyright         : (C) 2006 by Guillaume Bandet
	email             : guillaume.bandet@m4x.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// A voir
#include <RzxMessageBox>
#include <RzxComputer>
#include <RzxBanList>
#include <RzxApplication>

#include "rzxfilelistener.h"
#include "rzxfilesocket.h"

RzxFileListener::RzxFileListener()
{ }

RzxFileListener::~RzxFileListener()
{
	close();
}

///Connexion d'un RzxFileSocket au reste du programme
void RzxFileListener::attach(RzxFileSocket *sock)
{
	sockets[sock->peer()].append(sock);
}

///Réception d'une connexion entrante
//Ce slot est appelé dès que l'écoute enregistre une demande d'écriture sur le port tcp du transfert de fichier
void RzxFileListener::incomingConnection(int socketDescriptor)
{
	//Récupération de la connexion
	RzxFileSocket *sock = new RzxFileSocket();
	sock->setSocketDescriptor(socketDescriptor);

	// On vérifie au passage que la connexion est valide
	QHostAddress host;
	host = sock->peerAddress();
	if(!RzxBanList::global()->contains(host))
	{
		qDebug() << "New file connection from " << host.toString() << ": accepted";
		sock->envoi = false;
	}
	else
	{
		qDebug() << "New file connection from " << host.toString() << ": refused (in ban list)";
		sock->abort();
		delete sock;
	}
}

void RzxFileListener::closeTransfers(const RzxHostAddress& host)
{
	QList< QPointer< RzxFileSocket > > transferList = sockets.take(host);
	for(QList< QPointer< RzxFileSocket > >::iterator it = transferList.begin(); it != transferList.end(); it++)
		(*it)->close();
}


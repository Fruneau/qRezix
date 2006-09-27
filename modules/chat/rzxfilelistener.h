/***************************************************************************
                          rzxfilelistener.h  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 2006 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXFILELISTENER_H
#define RZXFILELISTENER_H

#include <QTcpServer>
#include <QString>
#include <QPointer>
#include <QHash>
#include <QList>
#include <RzxHostAddress>

#include "rzxfilesocket.h"

/**
  *@author Guillaume Bandet
  */

/// Permet l'ecoute sur le port destiné au transfert de fichier
/** Cette classe du module chat a pour but d'implémenter l'écoute
 * du réseau pour le transfert de fichier
 *
 * La partie communication est gérée par \ref RzxFileSocket
 */

class RzxFileListener : public QTcpServer
{
	Q_OBJECT

	QHash< RzxHostAddress, QList< QPointer< RzxFileSocket > > > sockets;

	public:
		RzxFileListener();
		~RzxFileListener();

		bool listen(quint16 port);
		void attach(RzxFileSocket* socket);
		void detach(RzxFileSocket* socket);
		void closeTransfers(const RzxHostAddress& host);

	protected slots:
		virtual void incomingConnection(int);
};

///Surcharge pour simplifier l'appel
inline bool RzxFileListener::listen(quint16 port)
{
	return QTcpServer::listen(QHostAddress::Any, port);
}

#endif

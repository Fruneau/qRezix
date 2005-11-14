/***************************************************************************
                          rzxserverlistener.h  -  description
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

#ifndef RZXSERVERLISTENER_H
#define RZXSERVERLISTENER_H

#include <QTimer>
#include <QTcpSocket>
#include <QPixmap>

#ifdef RZX_XNET_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include "rzxprotocole.h"


/**
  *@author Sylvain Joyeux
  */

class QStringList;
class QPixmap;
class RzxComputer;

class RzxServerListener : public RzxProtocole {
	Q_OBJECT

	bool restarting;

public:
	RzxServerListener();
	~RzxServerListener();

	/** Reimplemente depuis @ref RzxProtocole pour ajouter la gestion des icones. */
	virtual void parse(const QString& msg);
	/** Change l'icone de l'ordinateur local */
	void sendIcon(const QImage& image);
	void setupReconnection(const QString& msg);
	RzxHostAddress getServerIP() const;
	RzxHostAddress getIP() const;
	virtual bool isStarted() const;

public slots:
	virtual void start();
	virtual void stop();
	virtual void restart();
	
protected slots:
	void serverClose();
	void serverReceive();
	void serverError(QTcpSocket::SocketError);
	void serverTimeout();
	void connectToXnetserver();
	virtual void send(const QString& msg);
	virtual void pingReceived();
	void serverConnected();
	void serverFound();
	void serverResetTimer();
	void closeWaitFlush();
	void waitReconnection();

	void emitConnected();
	void emitDisconnected();

protected: // Protected attributes
	QTcpSocket socket;
	bool premiereConnexion;

	/** Buffer de lecture/ecriture dans le socket */
	RzxHostAddress iconHost;
	bool iconMode;	

	/** Timer utilisé pour les reconnexions automatiques */
	QTimer reconnection;
	/** Nom d'hote du serveur */
	QTimer pingTimer;
	/** Temps restant avant la tentative de reconnexion */
	int timeToConnection;
	/** Message */
	QString message;
	/** indique si une connexion a été établie depuis la dernière erreur indiquée à l'utilisateur */
	bool hasBeenConnected;
	
private:
	void notify(const QString& text);
};

///Notifie de la modification du statut de la connexion
inline void RzxServerListener::notify(const QString& text)
{
    emit status(text);
}

///Envoie le signal 'Connecté'
inline void RzxServerListener::emitConnected()
{
	emit connected(this);
}

///Envoie le signale 'Déconnecté'
inline void RzxServerListener::emitDisconnected()
{
	emit disconnected(this);
}

#endif

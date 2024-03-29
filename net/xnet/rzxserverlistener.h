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

#undef RZX_BUILTIN
#undef RZX_PLUGIN
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

///Gestion de la couche r�seau du module xNet
/** Gestion brute de la couche r�seau... RzxProtocole g�re la partie
 * haut niveau du protocole, RzxServerListener g�re le niveau socket
 * et transfert de donn�es binaires...
 */
class RzxServerListener : public RzxProtocole
{
	Q_OBJECT

	QTcpSocket socket;
	QTimer reconnectionTimer;
	QTimer pingTimer;

	//Pour le transfert de donn�es
	QString sendingBuffer;
	bool iconMode;
	RzxHostAddress iconHost;
	
	enum ReconnectionMode
	{
		None = 0,
		Quiet = 1,
		Normal = 2
	};

	//Pour la gestion de l'�tat de connexion
	bool restarting;
	bool userConnection;
	ReconnectionMode reconnection;
	bool wantDisconnection;

	//Pour l'affichage de l'avancement de la reconnexion
	QString message;
	int timeToReconnection;

public:
	RzxServerListener();
	~RzxServerListener();

	RzxHostAddress getServerIP() const;
	RzxHostAddress getIP() const;
	virtual bool isStarted() const;

//Gestion de la connexion
public slots:
	virtual void start();
	virtual void stop();
	virtual void restart();

protected:
	void connectToXnetserver();
	void setupReconnection(const QString& msg);

private slots:
	void waitReconnection();

//Fonctions de notification
protected slots:
	void notify(const QString& text);
	void serverFound();
	void serverDisconnected(const QString& = QString(), bool force = false);
	void serverConnected();
	void error(QAbstractSocket::SocketError);
	

//Echange de donn�es
protected slots:
	void read();
	virtual void parse(const QString& msg);

	virtual void send(const QString& msg);
	virtual void pingReceived();
	virtual void sendIcon(const QImage& image);

	void timeout();
	void haveActivity();
};

///Notifie de la modification du statut de la connexion
inline void RzxServerListener::notify(const QString& text)
{
    emit status(this, text);
}

#endif

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

	QTcpSocket socket;
	QTimer reconnectionTimer;
	QTimer pingTimer;

	//Pour le transfert de données
	QString sendingBuffer;
	bool iconMode;
	RzxHostAddress iconHost;

	//Pour la gestion de l'état de connexion
	bool restarting;
	bool userConnection;
	bool reconnection;
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
	void serverDisconnected(const QString& = QString());
	void serverConnected();
	void error(QTcpSocket::SocketError);
	

//Echange de données
protected slots:
	void read();
	virtual void parse(const QString& msg);

	virtual void send(const QString& msg);
	virtual void pingReceived();
	void sendIcon(const QImage& image);

	void timeout();
	void haveActivity();
};

///Notifie de la modification du statut de la connexion
inline void RzxServerListener::notify(const QString& text)
{
    emit status(text);
}

#endif

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

#include <qtimer.h>
#include <q3socket.h>
#include <qtimer.h>
//Added by qt3to4:
#include <QPixmap>

#include "rzxprotocole.h"


/**
  *@author Sylvain Joyeux
  */

class QStringList;
class QPixmap;
class RzxComputer;

class RzxServerListener : public RzxProtocole {
	Q_OBJECT
	
	RzxServerListener();
	static RzxServerListener * globalObject;
	
public:
	static RzxServerListener * object();
		
	~RzxServerListener();

	/** Reimplemente depuis @ref RzxProtocole pour ajouter la gestion des icones. */
	virtual void parse(const QString& msg);
	/** Change l'icone de l'ordinateur local */
	void sendIcon(const QImage& image);
	void setupConnection();
	void setupReconnection(const QString& msg);
	bool isSocketClosed() const;
	void close();
	RzxHostAddress getServerIP() const;
	RzxHostAddress getIP() const;

protected:
	//bool alternateGetHostByName( void );
	
protected slots:
	void serverClose();
	void serverReceive();
	void serverError(int);
	void serverTimeout();
	void connectToXnetserver();
	void sendProtocolMsg(const QString& msg);
	void serverConnected();
	void beginAuth();
	void serverFound();
	void serverResetTimer();
	void closeWaitFlush();
	void waitReconnection();

signals: // Signals
	/** Emit lorsque on passe un message ICON a @ref parse
	* @param ip ip de l'ordinateur auquel appartien l'icone
	* @param image icone dans le bon sens avec les bonnes couleurs RGB */
	void rcvIcon(QImage* image, const RzxHostAddress& ip);
	void status(const QString& msg, bool fatal);

	void connected();
	void disconnected();

protected: // Protected attributes
	Q3Socket socket;
	bool premiereConnexion;

	/** Buffer de lecture/ecriture dans le socket */
	RzxHostAddress iconHost;
	bool iconMode;	

	/** Timer utilisé pour les reconnexions automatiques */
	QTimer reconnection;
	/** Nom d'hote du serveur */
	QString serverHostname;
	/** Pour le timeout sur les ping/pong */
	QTimer pingTimer;
	/** Temps restant avant la tentative de reconnexion */
	int timeToConnection;
	/** Message */
	QString message;
	/** indique si une connexion a été établie depuis la dernière erreur indiquée à l'utilisateur */
	bool hasBeenConnected;
	
private:
	inline void notify(const QString& text) { emit status(text, socket.state()!=Q3Socket::Connected); }
};

#endif

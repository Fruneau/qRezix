/***************************************************************************
                          parser  -  description
                             -------------------
    begin                : mar ao 30 2005
    copyright            : (C) 2005 by Guillaume Porcher
    email                : pico@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXJABBERCLIENT_H
#define RZXJABBERCLIENT_H

#include <QTimer>
#include <QThread>
#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/rostermanager.h>
#include <gloox/presencehandler.h>
#include <gloox/registrationhandler.h>
#include <gloox/gloox.h>


using namespace gloox;

class RzxJabberClient : public QThread, MessageHandler, ConnectionListener, PresenceHandler, RegistrationHandler
{
	Q_OBJECT

	public:
		RzxJabberClient(QObject *parent=0);
		~RzxJabberClient();
		void onConnect();
		void onDisconnect( ConnectionError e );
		bool onTLSConnect( const CertInfo& info );
		void onResourceBindError (ResourceBindError){};
		void onSessionCreateError (SessionCreateError){};
		void handleMessage( Stanza *stanza );
		void handlePresence( Stanza *stanza );
		void run();
		void stop();
		Client* client(){ return j; }
		bool isStarted();
		bool send(Tag* t);
		void changePass(const QString &newPass);
		void handleRegistrationResult (resultEnum result);
		void handleRegistrationFields (int fields, std::string instructions){};
		void handleAlreadyRegistered (){};
		
	private:
		Client *j;
		QTimer *timer;

	signals:
		void presence(QString jid, QString name, int type);
		void msgReceived(QString from, QString msg);
		void connected();
		void disconnected();
		void rosterUpdated();
	private slots:
		void readData();
		
};




#endif

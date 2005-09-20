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
#include <gloox/discohandler.h>
#include <gloox/presencehandler.h>
#include <gloox/loghandler.h>
#include <gloox/gloox.h>


using namespace gloox;

class RzxJabberClient : public QThread, DiscoHandler, MessageHandler, ConnectionListener, PresenceHandler, LogHandler
{
	Q_OBJECT

	public:
		RzxJabberClient(std::string server);
		~RzxJabberClient();
		virtual void onConnect();
		virtual void onDisconnect( ConnectionError e );
		virtual bool onTLSConnect( const CertInfo& info );
		virtual void handleDiscoInfoResult( const std::string& id, const Stanza& stanza );
		virtual void handleDiscoItemsResult( const std::string& id, const Stanza& stanza );
		virtual void handleMessage( Stanza *stanza );
		virtual void handlePresence( Stanza *stanza );
		virtual void handleLog( const std::string& xml, bool incoming );
		void run();
		void stop();
		Client *j;
	private:
		bool isStarted;
		QTimer *timer;
	signals:
		void login(QString str, int type);
		void connected();
		void disconnected();
	private slots:
		void readData();
};




#endif

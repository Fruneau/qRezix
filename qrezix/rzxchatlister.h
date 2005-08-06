/***************************************************************************
                          rzxchatlister  -  description
                             -------------------
    begin                : Thu Jul 28 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXCHATLISTER_H
#define RZXCHATLISTER_H

#include <QObject>
#include <QString>
#include <QHash>

#include "rzxhostaddress.h"

class QPoint;
class RzxChat;
class RzxComputer;
class RzxClientListener;

/**
@author Florent Bruneau
*/
class RzxChatLister:public QObject
{
	Q_OBJECT

	QHash<RzxHostAddress, RzxChat*> chatByIP;
	QHash<QString, RzxChat*> chatByLogin;

	RzxChat *getChatByName(const QString&) const;
	RzxChat *getChatByIP(const RzxHostAddress&) const;

	RzxClientListener *client;

	bool wellInit;

	RzxChatLister();
	static RzxChatLister *object;

	public:
		static RzxChatLister *global();
		~RzxChatLister();

		bool isInitialised() const;

	public slots:
		void login(RzxComputer*);
		void logout(RzxComputer*);

		void warnProperties(const RzxHostAddress&);

		QWidget *historique(const RzxHostAddress&, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL);
		QWidget *historique(RzxComputer*, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL);
		QWidget *showProperties(const RzxHostAddress&, const QString&, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL);
		QWidget *showProperties(RzxComputer*, const QString&, bool withFrame = true, QWidget *parent = NULL, QPoint *pos = NULL);
		void proprietes(RzxComputer*);
		RzxChat *createChat(RzxComputer*);
		RzxChat *createChat(const RzxHostAddress&);

		void deleteChat(const RzxHostAddress&);
		void closeChat(const QString& login);
		void closeChats();

	signals:
		void wantQuit();
		void wantPreferences();
		void wantToggleResponder();
};

///Renvoie un objet
inline RzxChatLister *RzxChatLister::global()
{
	if(!object)
		object = new RzxChatLister();
	return object;
}

///Indique si l'objet est bien initialis�
inline bool RzxChatLister::isInitialised() const
{
	return wellInit;
}

///Renvoie la fen�tre de chat associ�e � name
inline RzxChat *RzxChatLister::getChatByName(const QString& name) const
{
	return chatByLogin[name];
}

///Renvoie la fen�tre de chat associ�e � l'IP
inline RzxChat *RzxChatLister::getChatByIP(const RzxHostAddress& ip) const
{
	return chatByIP[ip];
}

#endif

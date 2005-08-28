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

#include <RzxHostAddress>

#ifdef RZX_CHAT_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <RzxModule>

class QPoint;
class RzxChat;
class RzxComputer;
class RzxClientListener;
namespace Ui { class RzxChatPropUI; }

/**
@author Florent Bruneau
*/
class RzxChatLister:public RzxModule
{
	Q_OBJECT
	RZX_GLOBAL(RzxChatLister)

	QHash<RzxHostAddress, RzxChat*> chatByIP;
	QHash<QString, RzxChat*> chatByLogin;

	RzxChat *getChatByName(const QString&) const;
	RzxChat *getChatByIP(const RzxHostAddress&) const;

	RzxClientListener *client;

	QWidget *propWidget;
	Ui::RzxChatPropUI *ui;

	bool wellInit;

	public:
		RzxChatLister();
		~RzxChatLister();

		virtual bool isInitialised() const;

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

	public:
		virtual QList<QWidget*> propWidgets();
		virtual QStringList propWidgetsName();

	public slots:
		virtual void propInit(bool def = false);
		virtual void propUpdate();
		virtual void propClose();

	protected slots:
		void chooseBeep();
};

///Indique si l'objet est bien initialisé
inline bool RzxChatLister::isInitialised() const
{
	return wellInit;
}

///Renvoie la fenêtre de chat associée à name
inline RzxChat *RzxChatLister::getChatByName(const QString& name) const
{
	return chatByLogin[name];
}

///Renvoie la fenêtre de chat associée à l'IP
inline RzxChat *RzxChatLister::getChatByIP(const RzxHostAddress& ip) const
{
	return chatByIP[ip];
}

#endif

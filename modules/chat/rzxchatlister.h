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
#include <QPointer>

#include <RzxHostAddress>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_CHAT_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <RzxModule>

#include "rzxchat.h"

class QPoint;
class RzxComputer;
class RzxClientListener;
namespace Ui { class RzxChatProp; }

/**
@author Florent Bruneau
*/

///Classe principale du module de chat
/** Cette classe a pour but premier de lister les fenêtres de chat ouverte.
 * Mais c'est également la classe centrale du module Chat. Cet module a une
 * double utilité :
 * 	- interface graphique de chat
 * 	- interface réseau de chat
 *
 * Cette classe sert donc également de relai entre les deux structure indépendantes
 * que comporte le module, et avec le reste du programme.
 */
class RzxChatLister:public RzxModule
{
	Q_OBJECT
	RZX_GLOBAL(RzxChatLister)

	QHash<RzxHostAddress, QPointer<RzxChat> > chatByIP;
	QHash<QString, QPointer<RzxChat> > chatByLogin;

	RzxChat *getChatByName(const QString&) const;
	RzxChat *getChatByIP(const RzxHostAddress&) const;

	RzxClientListener *client;

	QWidget *propWidget;
	Ui::RzxChatProp *ui;
	QHash<RzxHostAddress, QPointer<QWidget> > warnWindow;

	bool wellInit;

	public:
		RzxChatLister();
		~RzxChatLister();
		
		virtual bool isInitialised() const;
		RzxClientListener *listener() const;

	public slots:
		void login(RzxComputer*);
		void logout(RzxComputer*);

		void warnProperties(RzxComputer*);

		QWidget *historique(RzxComputer*, bool withFrame = true, QWidget *parent = NULL, QAbstractButton *button = NULL);
		QWidget *showPropertiesWindow(RzxComputer*, bool withFrame = true, QWidget *parent = NULL, QAbstractButton *button = NULL);

		virtual void history(RzxComputer*);
		virtual void properties(RzxComputer*);
		virtual void showProperties(RzxComputer*);
		virtual void chat(RzxComputer*);
		RzxChat *createChat(RzxComputer*);

		void deleteChat(RzxComputer*);
		void closeChat(const QString& login);
		void closeChats();

		virtual void sendChatMessage(RzxComputer *, Rzx::ChatMessageType, const QString& = QString());
		virtual void receiveChatMessage(RzxComputer *, Rzx::ChatMessageType, const QString& = QString());

	public:
		virtual QList<QWidget*> propWidgets();
		virtual QStringList propWidgetsName();

	public slots:
		virtual void propInit(bool def = false);
		virtual void propUpdate();
		virtual void propClose();
		void previewSmileys(const QString&);

	protected slots:
		void chooseBeep();
		void changeTheme();

	protected:
		virtual bool eventFilter(QObject *, QEvent *);
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

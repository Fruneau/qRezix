/***************************************************************************
                          rzxchat.h  -  description
                             -------------------
    begin                : Sat Jan 26 2002
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

#ifndef RZXCHAT_IMPL_H
#define RZXCHAT_IMPL_H

#include <qtextedit.h>
#include <qsocket.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qframe.h>

#include "rzxhostaddress.h"
#include "rzxclientlistener.h"
#include "rzxchatui.h"

/**
  *@author Sylvain Joyeux
  */

class QDns;
class QTimer;
class QTextEdit;
class RzxChatSocket;

///Classe pour les fenêtre propriété et historique du chat
/** Affiche une frame qui permet de contenir les données des propriétés et de l'historique.<br>
  * Cette classe permet simplement une abstraction de la chose, sans pour autant apporter quoi que ce soit de vraiment nouveau
  */
class RzxPopup : public QFrame
{
	Q_OBJECT
	
	public:
		RzxPopup(QWidget *parent, const char *name);
		~RzxPopup();
		
		void forceVisible(bool pos);

	signals:
		void aboutToQuit();
};

inline void RzxPopup::forceVisible(bool pos)
{
	if(pos) show();
	else hide();
}


//Rajout de Stéphane Lescuyer 2004/05/27
//pour customiser la boite d'edition
class RzxTextEdit : public QTextEdit {
	Q_OBJECT
	
	friend class RzxChat;
	
public:
	RzxTextEdit(QWidget *parent=0, const char*name=0);
	~RzxTextEdit();
	
protected:
	void keyPressEvent(QKeyEvent *e);

signals:
	void enterPressed();
	void arrowPressed(bool down);
	void textWritten();
};

class RzxChat : public RzxChatUI {
	Q_OBJECT
	
	friend class RzxRezal;
	
	class ListText {
	public:
		ListText * pPrevious;
		QString texte;
		ListText * pNext;
		
	public:	
		ListText(QString t, ListText * pN);
		~ListText();
	};

public: 
	RzxChat(const RzxHostAddress& peerAddress);
	RzxChat(RzxChatSocket* sock);
	~RzxChat();
	RzxChatSocket *getSocket();
	void setSocket(RzxChatSocket* sock);
	void sendChat(const QString& msg);

private:
	void init();
	QPopupMenu menuPlugins;
	QWidget *hist;
	QWidget *prop;
	
protected:
	RzxHostAddress peer;
	QString hostname;
	QString textHistorique;
	QTimer * timer;
	ListText * history;
	ListText * curLine;
	QFont* defFont;
	RzxChatSocket *socket;
	
signals: // Signals
	void send(const QString& message);
	void closed(const RzxHostAddress& peer);
	void showHistorique(unsigned long ip, QString hostname, bool, QWidget*, QPoint*);
	void showProperties(const RzxHostAddress&, const QString&, bool, QWidget*, QPoint*);

public slots: // Public slots
	void receive(const QString& msg);
	void info(const QString& msg);
	void notify(const QString& msg, bool withHostname = false);
	void setHostname(const QString& name);
	void changeTheme();
	void changeIconFormat();
	void receiveProperties(const QString& msg);
	
public:
	inline QString getHostName() const { return hostname; }

protected slots:
	void btnSendClicked();
	void messageReceived();
	void pong(int ms);
	void btnHistoriqueClicked(bool on = false);
	void btnPropertiesClicked(bool on = false);
	void fontChanged(int index);
	void sizeChanged(int index);
	void activateFormat(bool on);
	void onReturnPressed();
	void onArrowPressed(bool down);
	void onTextChanged();
	bool event(QEvent *e);
	void pluginsMenu();
	RzxChatSocket *getValidSocket();

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
#ifdef WIN32
	void showEvent ( QShowEvent * e);
#endif
	void closeEvent(QCloseEvent * e);
};

/// Retourne un socket valide
/** Cette méthode permet d'obtenir à coup sur un socket valide pour le chat. C'est à dire que si le socket n'existe pas, il est créé */
inline RzxChatSocket *RzxChat::getValidSocket()
{
	if(!socket)
		setSocket(new RzxChatSocket(peer, this));
	return socket;
}

#endif

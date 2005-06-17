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

#include <QMenu.h>
#include <QFrame.h>
#include <QTextEdit>
#include <QWidget>
#include <QPointer>

#include "rzxhostaddress.h"
#include "rzxclientlistener.h"

/*#ifdef Q_OS_MAC
#include "rzxchatui_mac.h"
#else
#include "rzxchatui.h"
#endif
*/
/**
  *@author Sylvain Joyeux
  */

class QCloseEvent;
class QShowEvent;
class QMoveEvent;
class QKeyEvent;
class QEvent;
class QTimer;
class QSplitter;
class RzxChatSocket;
class RzxChatUI;

///Classe pour les fen�tre propri�t� et historique du chat
/** Affiche une frame qui permet de contenir les donn�es des propri�t�s et de l'historique.<br>
  * Cette classe permet simplement une abstraction de la chose, sans pour autant apporter quoi que ce soit de vraiment nouveau
  */
class RzxPopup : public QFrame
{
	Q_OBJECT
	
	public:
		RzxPopup(QWidget *parent = 0);
		~RzxPopup() { }
		
		void forceVisible(bool pos);
};

inline void RzxPopup::forceVisible(bool pos)
{
	if(pos) show();
	else hide();
}


//Rajout de St�phane Lescuyer 2004/05/27
//pour customiser la boite d'edition
class RzxTextEdit : public QTextEdit {
	Q_OBJECT
	
	friend class RzxChat;
	
public:
	RzxTextEdit(QWidget *parent=0):QTextEdit(parent) { }
	~RzxTextEdit();
	
protected:
	void keyPressEvent(QKeyEvent *e);
	bool nickAutocompletion();

signals:
	void enterPressed();
	void arrowPressed(bool down);
	void textWritten();
};

#ifdef Q_OS_MAC
#include "ui_rzxchatui_mac.h"
#else
#include "ui_rzxchatui.h"
#endif

class RzxChat : public QWidget, public Ui::RzxChatUI {
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

	static const QColor preDefinedColors[16];
	
	QTextEdit *txtHistory;
	QWidget *editor;
	QSplitter *splitter;

public: 
	RzxChat(const RzxHostAddress& peerAddress);
	RzxChat(RzxChatSocket* sock);
	virtual ~RzxChat();
	RzxChatSocket *getSocket();
	void setSocket(RzxChatSocket* sock);
	void sendChat(const QString& msg);

protected:
	RzxChatSocket *getValidSocket();

private:
	void init();
	void addColor(QColor color);
	QMenu menuPlugins;
	QPointer<RzxPopup> hist;
	QPointer<RzxPopup> prop;
	
protected:
	RzxHostAddress peer;
	QString hostname;
	QString textHistorique;
	QTimer *timer;
	ListText *history;
	ListText *curLine;
	QFont *defFont;
	RzxChatSocket *socket;
	QColor curColor;
	bool typing, peerTyping;
	QTimer typingTimer;
	int unread;
	
signals: // Signals
	void send(const QString& message);
	void sendResponder(const QString& message);
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
	void peerTypingStateChanged(bool state);
	
public:
	inline QString getHostName() const { return hostname; }

protected slots:
	void pong(int ms);
	void on_btnHistorique_toggled(bool on = false);
	void on_btnProperties_toggled(bool on = false);
	void on_cbColorSelect_activated(int index);
	void on_cbFontSelect_activated(int index);
	void on_cbSize_activated(int index);
	void on_cbSendHTML_toggled(bool on);
	void on_btnSend_clicked();
	void onReturnPressed();
	void onArrowPressed(bool down);
	void onTextChanged();
	bool event(QEvent *e);
	void on_btnPlugins_clicked();
	void updateTitle();
#ifdef Q_OS_MAC
    virtual void languageChange();
#endif

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
#ifdef WIN32
	void showEvent ( QShowEvent * e);
#endif
	void closeEvent(QCloseEvent * e);
	void moveEvent(QMoveEvent *e);
};

/// Retourne un socket valide
/** Cette m�thode permet d'obtenir � coup sur un socket valide pour le chat. C'est � dire que si le socket n'existe pas, il est cr�� */
inline RzxChatSocket *RzxChat::getValidSocket()
{
	if(!socket)
		setSocket(new RzxChatSocket(peer, this));
	return socket;
}

///Retourne du socket de chat
/** N'assure pas le retour d'un socket valide. Retourne null si aucun socket n'existe pour ce chat */
inline RzxChatSocket *RzxChat::getSocket()
{
	return socket;
}

#endif

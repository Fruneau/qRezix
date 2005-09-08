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

#include <QMenu>
#include <QFrame>
#include <QTextEdit>
#include <QPointer>

#include <RzxHostAddress>

#include "rzxchatsocket.h"

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

///Classe pour les fenêtre propriété et historique du chat
/** Affiche une frame qui permet de contenir les données des propriétés et de l'historique.<br>
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

///Pour customiser la boîte d'édition
/** Rajout de Stéphane Lescuyer 2004/05/27 */
class RzxTextEdit : public QTextEdit 
{
	Q_OBJECT
	
	friend class RzxChat;
	RzxChat *chat;
	
public:
	RzxTextEdit(QWidget *parent=0);
	~RzxTextEdit();
	
protected:
	void setChat(RzxChat*);
	void keyPressEvent(QKeyEvent*);
	bool nickAutocompletion();

signals:
	void enterPressed();
	void arrowPressed(bool);
	void textWritten();
};

inline RzxTextEdit::RzxTextEdit(QWidget *parent)
	:QTextEdit(parent), chat(NULL) { }

inline void RzxTextEdit::setChat(RzxChat *m_chat)
{ chat = m_chat; }


#ifdef Q_OS_MAC
#	include "ui_rzxchatui_mac.h"
#else
#	include "ui_rzxchatui.h"
#endif

///Fenêtre de dialogue
/** (et pas boîte de dialogue ;)... gere la totalité de l'interface de chat.
 *
 * Cette classe s'interface directement sur un RzxChatSocket pour permettre
 * une gestion des communications totalement autonome.
 */
class RzxChat : public QWidget, private Ui::RzxChatUI
{
	Q_OBJECT
	Q_PROPERTY(QString hostname READ hostname WRITE setHostname)
	Q_PROPERTY(RzxChatSocket* socket READ socket WRITE setSocket)
	
	friend class RzxRezal;
	
	class ListText
	{
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
	RzxChatSocket *socket();
	void sendChat(const QString& msg);

protected:
	RzxChatSocket *validSocket();

private:
	void init();
	void addColor(QColor color);
	QMenu menuPlugins;
	QPointer<RzxPopup> hist;
	QPointer<RzxPopup> prop;
	
protected:
	RzxHostAddress peer;
	QString m_hostname;
	QString textHistorique;
	QTimer *timer;
	ListText *history;
	ListText *curLine;
	QFont *defFont;
	RzxChatSocket *m_socket;
	QColor curColor;
	bool typing, peerTyping;
	QTimer typingTimer;
	int unread;
	
signals: // Signals
	void send(const QString& message);
	void sendResponder(const QString& message);
	void closed(const RzxHostAddress& peer);
	void showHistorique(const RzxHostAddress&, const QString& hostname, bool, QWidget*, QPoint*);
	void showProperties(const RzxHostAddress&, const QString&, bool, QWidget*, QPoint*);

public slots: // Public slots
	void receive(const QString&);
	void info(const QString&);
	void notify(const QString&, bool withHostname = false);
	void setHostname(const QString&);
	void changeTheme();
	void changeIconFormat();
	void receiveProperties(const QString&);
	void peerTypingStateChanged(bool);
	void setSocket(RzxChatSocket* sock);
	
public:
	const QString &hostname() const;

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

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
#ifdef WIN32
	virtual void showEvent ( QShowEvent * e);
#endif
	virtual void closeEvent(QCloseEvent * e);
	virtual void moveEvent(QMoveEvent *e);
	virtual void changeEvent(QEvent *e);
};

inline const QString &RzxChat::hostname() const
{ return m_hostname; }

/// Retourne un socket valide
/** Cette méthode permet d'obtenir à coup sur un socket valide pour le chat. C'est à dire que si le socket n'existe pas, il est créé */
inline RzxChatSocket *RzxChat::validSocket()
{
	if(!m_socket)
		setSocket(new RzxChatSocket(peer, this));
	return m_socket;
}

///Retourne du socket de chat
/** N'assure pas le retour d'un socket valide. Retourne null si aucun socket n'existe pour ce chat */
inline RzxChatSocket *RzxChat::socket()
{
	return m_socket;
}

#endif

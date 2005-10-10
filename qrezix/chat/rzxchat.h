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
#include <QTimer>

#include <RzxComputer>

/**
  *@author Sylvain Joyeux
  */

class QCloseEvent;
class QShowEvent;
class QMoveEvent;
class QKeyEvent;
class QEvent;
class QSplitter;
class RzxChat;

///Classe pour les fen�re propri��et historique du chat
/** Affiche une frame qui permet de contenir les donn�s des propri�� et de l'historique.<br>
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

///Pour customiser la bo�e d'�ition
/** Rajout de St�hane Lescuyer 2004/05/27 */
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

///Fen�re de dialogue
/** (et pas bo�e de dialogue ;)... gere la totalit�de l'interface de chat.
 *
 * Cette classe s'interface directement sur un RzxChatSocket pour permettre
 * une gestion des communications totalement autonome.
 */
class RzxChat : public QWidget, private Ui::RzxChatUI
{
	Q_OBJECT
	Q_PROPERTY(RzxComputer* computer READ computer WRITE setComputer)
	
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
	RzxChat(RzxComputer *);
//	RzxChat(RzxChatSocket* sock);
	virtual ~RzxChat();
//	RzxChatSocket *socket();
	void sendChat(const QString& msg);

private:
	void init();
	void addColor(QColor color);
	QMenu menuPlugins;
	QPointer<RzxPopup> hist;
	QPointer<RzxPopup> prop;
	
protected:
	QPointer<RzxComputer> m_computer;

	QString textHistorique;
	QTimer *timer;
	ListText *history;
	ListText *curLine;
	QFont *defFont;
	QColor curColor;
	bool typing, peerTyping;
	QTimer typingTimer;
	int unread;
	
signals: // Signals
	void closed(RzxComputer *);
	void showHistorique(RzxComputer *, const QString& hostname, bool, QWidget*, QPoint*);
	void showProperties(RzxComputer *, const QString&, bool, QWidget*, QPoint*);

public slots: // Public slots
	void receive(const QString&);
	void info(const QString&);
	void notify(const QString&, bool withHostname = false);
	void changeTheme();
	void changeIconFormat();
	void receiveProperties(const QString&);
	void peerTypingStateChanged(bool);
	void setComputer(RzxComputer* computer);
	void updateTitle();

	void receiveChatMessage(Rzx::ChatMessageType, const QString& = QString());
	
public:
	RzxComputer *computer() const;

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

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
#ifdef WIN32
	virtual void showEvent ( QShowEvent * e);
#endif
	virtual void closeEvent(QCloseEvent * e);
	virtual void moveEvent(QMoveEvent *e);
	virtual void changeEvent(QEvent *e);
};

inline RzxComputer *RzxChat::computer() const
{ return m_computer; }


#endif

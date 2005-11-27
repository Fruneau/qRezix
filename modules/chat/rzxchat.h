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
#include <QPointer>
#include <QTimer>

#include <RzxComputer>

#ifdef Q_OS_MAC
#	include "ui_rzxchatui_mac.h"
#else
#	include "ui_rzxchatui.h"
#endif

#include "rzxchatpopup.h"
#include "rzxsmileyui.h"

/**
  *@author Sylvain Joyeux
  */

class QCloseEvent;
class QShowEvent;
class QMoveEvent;
class QKeyEvent;
class QEvent;
class QSplitter;
class QTextEdit;

///Fen�tre de dialogue
/** (et pas bo�te de dialogue ;)... gere la totalit� de l'interface de chat.
 *
 * Cette classe s'interface directement sur un RzxChatSocket pour permettre
 * une gestion des communications totalement autonome.
 */
class RzxChat : public QWidget, private Ui::RzxChatUI
{
	Q_OBJECT
	Q_PROPERTY(RzxComputer* computer READ computer WRITE setComputer)
	
	static const QColor preDefinedColors[16];
	
	QTextEdit *txtHistory;
	QWidget *editor;
	QSplitter *splitter;

public: 
	RzxChat(RzxComputer *);
	virtual ~RzxChat();
	void sendChat(const QString& msg);

private:
	void init();
	void addColor(QColor color);
	QMenu menuPlugins;
	QPointer<RzxChatPopup> hist;
	QPointer<RzxChatPopup> prop;
	QPointer<RzxSmileyUi> smileyUi;
	
protected:
	QPointer<RzxComputer> m_computer;
	qint32 lastIP;
	QString lastName;

	QString textHistorique;
	QTimer *timer;
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
	const QString& name() const;
	qint32 rezixIP() const;

protected slots:
	void pong(int ms);
	void on_btnHistorique_toggled(bool on = false);
	void on_btnProperties_toggled(bool on = false);
	void setColor(int index);
	void setFont(int index);
	void setSize(int index);
	void setHtml(bool);
	void on_btnSend_clicked();
	void on_btnSmiley_toggled(bool on);
	void onReturnPressed();
	void onTextChanged();
	bool event(QEvent *e);
	void on_btnPlugins_clicked();

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
	QString replaceSmiley(const QString& txt);
#ifdef WIN32
	virtual void showEvent ( QShowEvent * e);
#endif
	virtual void closeEvent(QCloseEvent * e);
	virtual void moveEvent(QMoveEvent *e);
	virtual void changeEvent(QEvent *e);
};

///Retourne la machine associ�e � la fen�tre de chat
inline RzxComputer *RzxChat::computer() const
{ return m_computer; }

///Retourne le nom de la machine associ�e
inline const QString& RzxChat::name() const
{ return lastName; }

///Retourne l'adresse de la machine associ�e
inline qint32 RzxChat::rezixIP() const
{ return lastIP; }

#endif
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
#include <QPointer>
#include <QTimer>

#include <RzxComputer>

#ifdef Q_OS_MAC
#	include "ui_rzxchatui_mac.h"
#else
#	include "ui_rzxchatui.h"
#endif

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

///Fenêtre de dialogue
/** (et pas boîte de dialogue ;)... gere la totalité de l'interface de chat.
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
	QPointer<RzxPopup> hist;
	QPointer<RzxPopup> prop;
	QPointer<RzxSmileyUi> smileyUi;
	QList<QStringList> smileys;
	
protected:
	QPointer<RzxComputer> m_computer;
	qint32 lastIP;
	QString lastName;

	QString textHistorique;
	QTimer *timer;
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
	const QString& name() const;
	qint32 rezixIP() const;

protected slots:
	void pong(int ms);
	void on_btnHistorique_toggled(bool on = false);
	void on_btnProperties_toggled(bool on = false);
	void on_cbColorSelect_activated(int index);
	void on_cbFontSelect_activated(int index);
	void on_cbSize_activated(int index);
	void on_cbSendHTML_toggled(bool on);
	void on_btnSend_clicked();
	void onSmileyToggled(bool on);
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

///Retourne la machine associée à la fenêtre de chat
inline RzxComputer *RzxChat::computer() const
{ return m_computer; }

///Retourne le nom de la machine associée
inline const QString& RzxChat::name() const
{ return lastName; }

///Retourne l'adresse de la machine associée
inline qint32 RzxChat::rezixIP() const
{ return lastIP; }

#endif

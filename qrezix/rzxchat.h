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
#include "rzxhostaddress.h"
#include "rzxchatui.h"

/**
  *@author Sylvain Joyeux
  */

class QDns;
class QTimer;
class QTextEdit;

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
};

class RzxChat : public RzxChatUI {
	Q_OBJECT
	
	friend class RzxRezal;
public: 
	RzxChat(const RzxHostAddress& peerAddress);
	~RzxChat();
	
protected:
	RzxHostAddress peer;
	QString hostname;
	QTimer * timer;
	
signals: // Signals
	void send(const RzxHostAddress& peer, const QString& message);
	void closed(const RzxHostAddress& peer);
	void showHistorique(unsigned long ip, QString hostname);
    	void askProperties(const RzxHostAddress& peer);

public slots: // Public slots
	void receive(const QString& msg);
    void info(const QString& msg);
	void setHostname(const QString& name);

protected slots:
	void btnSendClicked();
	void messageReceived();
	void btnHistoriqueClicked();
	void btnPropertiesClicked();
	void onReturnPressed();
	void soundToggled(bool state);

protected: // Protected methods
	void append(const QString& color, const QString& host, const QString& msg);
#ifdef WIN32
  bool event (QEvent * e);
	void showEvent ( QShowEvent * e);
#endif
	void closeEvent(QCloseEvent * e);
};

#endif

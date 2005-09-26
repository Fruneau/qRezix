/***************************************************************************
                          rzxprotocole.h  -  description
                             -------------------
    begin                : Fri Jan 25 2002
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

#ifndef RZXJABBERPROTOCOLE_H
#define RZXJABBERPROTOCOLE_H

#define RZX_PLUGIN

#include <QObject>
#include <QString>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

#include <RzxHostAddress>
#include <RzxNetwork>

#include "ui_rzxjabberpropui.h"
#include "rzxjabberclient.h"
#include "rzxjabbercomputer.h"

/**
	*Gere le protocole Jabber.
  */

class QStringList;
class RzxComputer;

class RzxJabberProtocole : public RzxNetwork
{
	Q_OBJECT
	
public: 
	RzxJabberProtocole();
	~RzxJabberProtocole();
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();
	virtual bool isStarted() const;


//Gestion de propri�� du module
private:
	Ui::RzxJabberPropUI *ui;
	QWidget *propWidget;
	RzxJabberClient *client;
	QHash<QString,RzxJabberComputer> computerList;
	QMutex mutex;


public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	virtual void getIcon(const RzxHostAddress& ip){};
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();
	
	virtual void start();
	virtual void stop();

	virtual void refresh();
	void presenceRequest(QString jid,QString name, int type);
	void buildRosterList();	
	void addRosterItem();
	void removeRosterItem();
	void changeRosterText(QListWidgetItem *cur,QListWidgetItem *old);
	/** Demande de changement de pass */
	virtual void wantChangePass(){};
	virtual void changePass(const QString&){};
	virtual void usePass(const QString&){};
	void connection();
	void deconnection();

	void sendMsg(QString to, QString msg);

// Signals
signals:
 	/** emit quand l'objet RzxJabberProtocole a besoin d'envoyer une commande au serveur,
 	*typiquement lors d'appel aux fonctions @ref sendAuth, @ref sendRefresh,
 	*@ref sendPart, @ref sendPong et @ref getIcon */
	void send(const QString& msg);
	void connected(RzxJabberProtocole*);
	void disconnected(RzxJabberProtocole*);
};


#endif

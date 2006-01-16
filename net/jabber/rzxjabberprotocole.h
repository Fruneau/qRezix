/***************************************************************************
                          rzxjabberprotocole.h  -  description
                             -------------------
    begin                : Fri Sept 25 2005
    copyright            : (C) 2002 by Guillaume Porcher
    email                : pico@m4x.org
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

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_JABBER_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <QObject>
#include <QString>
#include <QHash>
#include <QListWidgetItem>

#include <RzxHostAddress>
#include <RzxNetwork>


#include "rzxjabberclient.h"
#include "rzxjabbercomputer.h"
#include "rzxjabberproperty.h"

/**
	*Gere le protocole Jabber.
  */

class QStringList;
class RzxComputer;
namespace Ui { class RzxJabberProp; };

class RzxJabberProtocole : public RzxNetwork
{
	Q_OBJECT
	
public: 
	RzxJabberProtocole();
	~RzxJabberProtocole();
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();
	virtual bool isStarted() const;
	
	
//Gestion de propriétés du module
private:
	Ui::RzxJabberProp *ui;
	QWidget *propWidget;
	RzxJabberClient *client;
	QHash<QString,RzxJabberComputer*> computerList;
	void getProperties(QString jid, QString comp);
	

public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	virtual void getIcon(const RzxHostAddress&);
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();

protected slots:
	virtual void translate();

public slots:
	virtual void start();
	virtual void stop();

	virtual void refresh();
	void presenceRequest(QString jid,QString name, int type);
	void buildRosterList();	
	void addRosterItem();
	void removeRosterItem();
	void changeRosterText(QListWidgetItem *cur,QListWidgetItem *old);
	/** Demande de changement de pass */
	virtual void wantChangePass();
	virtual void changePass(const QString&);
	virtual void usePass(const QString&);
	void wantNewAccount();
	void connection();
	void deconnection();
	
	void sendMsg(QString to, QString msg);
	void recvMsg(QString to, QString msg);
	
	virtual void chat(RzxComputer*);
	virtual void sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());
	virtual void properties(RzxComputer*);
	void receivedProperties(RzxJabberComputer*);

private slots:
	void updateLocalhost();
	void sendProperties();
	
// Signals
signals:
 	/** emit quand l'objet RzxJabberProtocole a besoin d'envoyer une commande au serveur,
 	*typiquement lors d'appel aux fonctions @ref sendAuth, @ref sendRefresh,
 	*@ref sendPart, @ref sendPong et @ref getIcon */
	void send(const QString& msg);
	void connected(RzxJabberProtocole*);
	void disconnected(RzxJabberProtocole*);
	void haveProperties(RzxComputer*);
};


#endif

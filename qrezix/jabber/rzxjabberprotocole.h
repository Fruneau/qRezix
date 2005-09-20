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
	
	virtual bool isStarted() const;


public slots:
	virtual void start();
	virtual void stop();

//Gestion de propri�� du module
private:
	Ui::RzxJabberPropUI *ui;
	QWidget *propWidget;
	RzxJabberClient *client;
	QList<RzxJabberComputer> computerList;

public:
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();

public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	virtual void getIcon(const RzxHostAddress& ip){};
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();
	
	virtual void refresh(){};
	void presenceRequest(QString str, int type);
	
	/** Demande de changement de pass */
	virtual void wantChangePass(){};
	virtual void changePass(const QString&){};
	virtual void usePass(const QString&){};
	void connection(){ emit connected(); };
	void deconnection(){ emit disconnected(); };
// Signals
signals:
	/** ping() est emit quand on passe une commande PING a @ref parse */
 	void ping();
 	
 	/** emit quand l'objet RzxJabberProtocole a besoin d'envoyer une commande au serveur,
 	*typiquement lors d'appel aux fonctions @ref sendAuth, @ref sendRefresh,
 	*@ref sendPart, @ref sendPong et @ref getIcon */
	void send(const QString& msg);
	void connected();
	void disconnected();
};


#endif

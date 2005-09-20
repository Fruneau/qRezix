/***************************************************************************
                          rzxprotocole.cpp  -  description
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
#include <QStringList>
#include <QRegExp>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QRegExpValidator>
#include <QPixmap>
#include <QImage>
#include <QDialog>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxWrongPass>
#include <RzxChangePass>

#include "rzxjabberprotocole.h"
#include "rzxjabberconfig.h"

RZX_NETWORK_EXPORT(RzxJabberProtocole)
RZX_CONFIG_INIT(RzxJabberConfig)

///Construction... RAS
RzxJabberProtocole::RzxJabberProtocole()
	: RzxNetwork("Jabber", "Native support for the jabber protocole",0,0,1,"svn")
{
	beginLoading();
	ui = NULL;
	propWidget = NULL;
	new RzxJabberConfig(this);

	client = new RzxJabberClient ("test");
	connect(client, SIGNAL(login(QString, int)), this, SLOT(presenceRequest(QString, int)));
	setIcon(RzxThemedIcon(Rzx::ICON_NETWORK));
	endLoading();
}

///Destruction...
RzxJabberProtocole::~RzxJabberProtocole(){
	beginClosing();
	delete RzxJabberConfig::global();
	delete client;
	endClosing();
}


/****************************************************************************
* FEN�RE DE PROPRI��
*/

/** \reimp */
QList<QWidget*> RzxJabberProtocole::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxJabberPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxJabberProtocole::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxJabberProtocole::propInit(bool def)
{
	ui->server_name->setText( RzxJabberConfig::serverName() );
	ui->user->setText( RzxJabberConfig::user() );
	ui->pass->setText( RzxJabberConfig::pass() );
	ui->server_port->setValue( RzxJabberConfig::serverPort() );
	ui->reconnection->setValue( RzxJabberConfig::reconnection() / 1000 );
	ui->ping_timeout->setValue( RzxJabberConfig::pingTimeout() / 1000 );
}

/** \reimp */
void RzxJabberProtocole::propUpdate()
{
	if(!ui) return;

	bool restart =  false;

	if(ui->server_name->text() != RzxJabberConfig::serverName() || ui->server_port->value() != RzxJabberConfig::serverPort() || ui->user->text() != RzxJabberConfig::user() || ui->pass->text() != RzxJabberConfig::pass())
	{
		restart=true;
	}

	RzxJabberConfig::setServerName(ui->server_name->text() );
	RzxJabberConfig::setServerPort(ui->server_port->value() );
	RzxJabberConfig::setUser(ui->user->text() );
	RzxJabberConfig::setPass(ui->pass->text() );
	RzxJabberConfig::setReconnection(ui->reconnection->value() * 1000 );
	RzxJabberConfig::setPingTimeout(ui->ping_timeout->value() * 1000 );
	
	if(restart){
		stop();
		start();
	}
}

/** \reimp */
void RzxJabberProtocole::propClose()
{
	if(propWidget)
	{
		delete propWidget;
		propWidget = NULL;
	}
	if(ui)
	{
		delete ui;
		ui = NULL;
	}
}

bool RzxJabberProtocole::isStarted() const
{
	return client->isRunning();
}

void RzxJabberProtocole::start() {
	client->start();
}

void RzxJabberProtocole::stop() {
	client->stop();
	client->wait(1000);
}


void RzxJabberProtocole::presenceRequest(QString str, int type) {
	struct options_t
	{
	unsigned Server                 :6;
	unsigned SysEx                          :3;     //0=Inconnu, 1=Win9X, 2=WinNT, 3=Linux, 4=MacOS, 5=MacOS X, 6=BSD
	unsigned Promo                          :2; //0 = Orange, 1=Jne, 2=Rouje (Chica la rouje ! <== bah nan, �la j�e !!!)
	unsigned Repondeur              :2; //0=accepter, 1= repondeur, 2=refuser les messages, 3= unused
	// total 13 bits / 32
	unsigned Capabilities   :19;
	};
	options_t opt;
	RzxJabberComputer newComputer(str, computerList.size());
	if(computerList.indexOf(newComputer)>=0){
		newComputer = computerList.at(computerList.indexOf(newComputer));
		newComputer.nbClients++;
	}else
		computerList << newComputer;
	switch(type){
		case 0: // Hors ligne
			newComputer.nbClients--;
			if(newComputer.nbClients<=0){
				emit logout(newComputer.ip());
				computerList.removeAll(newComputer);
			}
			break;
		case 1: // Away
			opt.Repondeur=1;
			emit login( this, 
			RzxHostAddress(newComputer.ip()), //IP
			str, //Nom de la machine 
			*((quint32*) &opt), //Options
			0, //Version du client
			0, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
		case 2: // En ligne
			opt.Repondeur=0;
			emit login(  this, 
			RzxHostAddress(newComputer.ip()), //IP
			str, //Nom de la machine 
			*((quint32*) &opt), //Options
			0, //Version du client
			0, //Hash de l'ic�e
			0, //Flags ?????
			"Client Jabber"); //Remarque
			break;
	}
}

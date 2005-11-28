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
#define RZX_MODULE_NAME "xNet"

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
#include <RzxApplication>

#include "rzxprotocole.h"
#include "rzxxnetconfig.h"
#include "md5.h"

#include "ui_rzxxnetpropui.h"

#define MD5_ADD "Vive le BR"

///Masques pour les messages du protocole xNet
const char * RzxProtocole::ServerFormat[] = {
	"^JOIN ([0-9A-Fa-f]+) (\\S+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) (.*)\r\n",
	"^REFRESH ([0-9A-Fa-f]+) (\\S+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) ([0-9A-Fa-f]+) (.*)\r\n",
	"^SYSMSG (.*)\r\n",
	"^PING\r\n",
	"^PASS ([0-9A-Za-z]+)\r\n",
	"^PART ([0-9A-Za-z]+)\r\n",
	"^UPGRADE (v[0-9]+,[0-9]+,[0-9]+ .*)\r\n",
	"^FATAL (.*)\r\n",
	"^ICON ([0-9A-Fa-f]+)\r\n",
	"^WRONGPASS\r\n",
	"^CHANGEPASSOK\r\n",
	"^CHANGEPASSFAILED (.*)\r\n",
	"^UPLOAD\r\n",
	0
};

const QString RzxProtocole::serialPattern = "$nn $xo $xv $xf $rem";

///Construction... RAS
RzxProtocole::RzxProtocole()
	: RzxNetwork(RZX_MODULE_NAME, QT_TR_NOOP("Native support for the xNet protocole version 4.0"), RZX_MODULE_VERSION),
		 ui(NULL), propWidget(NULL)
{
	setIcon(RzxThemedIcon(Rzx::ICON_NETWORK));
}

///Destruction...
RzxProtocole::~RzxProtocole(){
}


/** Analyse les messages serveur -> client. Cette methode ne gere
pas les messages ICON du fait des donnees binaires qui arrivent. Elle est
reimplementer dans RzxServerListener */
void RzxProtocole::parse(const QString& msg)
{
	bool ok=true;
	quint32 val;
	static bool testOldPass = false;
	
	QRegExp cmd;
	for (int i = 0; ServerFormat[i]; i++)
	{
		cmd.setPattern(ServerFormat[i]);
		if (cmd.indexIn(msg) == -1)
			continue;
		
		switch (i)
		{
			case SERVER_JOIN:
			case SERVER_REFRESH:
				emit login(this, RzxHostAddress::fromRezix(cmd.cap(1).toUInt(0, 16)), //IP
						   cmd.cap(2), //Nom de la machine 
						   cmd.cap(3).toUInt(0, 16), //Options
						   cmd.cap(4).toUInt(0, 16), //Version du client
						   cmd.cap(5).toUInt(0, 16), //Hash de l'ic�ne
						   cmd.cap(6).toUInt(0, 16), //Flags ?????
						   cmd.cap(7)); //Remarque
				
				//Si le pass donn� par le serveur est encore valide, alors, on demande le changement de pass
				if(testOldPass && !RzxXNetConfig::oldPass().isNull())
				{
					testOldPass = false;
					m_oldPass = RzxXNetConfig::oldPass();
					wantChangePass();
				}
				break;
				
			case SERVER_SYSMSG:
				if (!cmd.cap(1).isEmpty())
				  emit info(cmd.cap(1));
				break;
				
			case SERVER_PING:
				pingReceived();
				break;

			case SERVER_WRONGPASS:
				if(!RzxXNetConfig::oldPass().isNull() && !testOldPass)
				{
					sendAuth(RzxXNetConfig::oldPass());
					testOldPass = true;
				}
				else
				{
					new RzxWrongPass(this);
					RzxXNetConfig::global()->setOldPass();
				}
				break;

			case SERVER_FATAL:
				if (!cmd.cap(1).isEmpty())
					emit fatal(cmd.cap(1));
				break;
				
			case SERVER_PASS:
				RzxXNetConfig::global()->setOldPass(cmd.cap(1));
				m_oldPass = cmd.cap(1);
				wantChangePass();
				break;
			
			case SERVER_CHANGEPASSOK:
				emit info(tr("Your password has been successfully changed on the server. Remember it carefully, it can be useful."));
				if(!RzxXNetConfig::oldPass().isNull()) RzxXNetConfig::setOldPass();
				RzxXNetConfig::setPass(m_newPass);
				break;
			
			case SERVER_CHANGEPASSFAILED:
				emit info(tr("Server was unable to change your pass :\n") + cmd.cap(1));
				RzxXNetConfig::setPass(m_oldPass);
				break;
				
			case SERVER_PART:
				val = cmd.cap(1).toULong(&ok, 16);
				if(ok)
					emit logout(RzxHostAddress::fromRezix(val));
				break;
				
			case SERVER_ICON: 	// non geres par l'objet protocole du fait du pb
						// des donnees binaires. Les icones sont par contre
						// gerees dans RzxServerListener
				break;

			case SERVER_UPGRADE:
			default:;
		}
	}
}

/*******************************************************************************
* MESSAGES A ENVOYER AU SERVEUR
*/
/// D�fini le mot de passe � utiliser
void RzxProtocole::usePass(const QString& pass)
{
	QString pwd(pass);
	if(pwd.length())
	{
		pwd += MD5_ADD;
		pwd=MD5String(pwd.toLatin1());
		sendAuth(pwd);
		RzxXNetConfig::setPass(pwd);
	}
	else
		stop();
}

/** No descriptions */
void RzxProtocole::beginAuth()
{
	sendAuth(RzxXNetConfig::pass());
}

void RzxProtocole::sendAuth(const QString& passcode)
{
	QString msg = "VERSION 4.0\r\n";
	msg = msg + "PASS " + passcode + "\r\n";
	msg = msg + "JOIN " + RzxComputer::localhost()->serialize(serialPattern) + "\r\n";
	
	send(msg);	
}

void RzxProtocole::sendRefresh() {
	QString msg = "REFRESH ";
	msg = msg + RzxComputer::localhost()->serialize(serialPattern) + "\r\n";
	send(msg);
}

void RzxProtocole::sendPart() {
	send("PART\r\n");
}
void RzxProtocole::sendPong() {
	send("PONG\r\n");
}
void RzxProtocole::getIcon(const RzxHostAddress& ip) {
	QString msg = "DOWNLOAD " + QString::number(ip.toRezix(), 16) + "\r\n";
	send(msg);
}

/****************************************************************************
* CHANGEMENT DU PASS
*/
void RzxProtocole::wantChangePass()
{
	new RzxChangePass(this, m_oldPass);
}

void RzxProtocole::changePass(const QString& newPass)
{
	m_newPass = newPass + MD5_ADD;
	m_newPass = MD5String(m_newPass.toLatin1());
	send("CHANGEPASS " + m_newPass + "\r\n");
}


/****************************************************************************
* FEN�TRE DE PROPRI�T�S
*/

/** \reimp */
QList<QWidget*> RzxProtocole::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxXNetPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect(ui->btnChangePass, SIGNAL(clicked()), this, SLOT(wantChangePass()));
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxProtocole::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxProtocole::propInit(bool def)
{
	ui->server_name->setText( RzxXNetConfig::serverName(def) );
	ui->server_port->setValue( RzxXNetConfig::serverPort(def) );
	ui->reconnection->setValue( RzxXNetConfig::reconnection(def) / 1000 );
	ui->ping_timeout->setValue( RzxXNetConfig::pingTimeout(def) / 1000 );
	ui->btnChangePass->setEnabled(isStarted());
}

/** \reimp */
void RzxProtocole::propUpdate()
{
	if(!ui) return;

	if(ui->server_name->text() != RzxXNetConfig::serverName() || ui->server_port->value() != RzxXNetConfig::serverPort())
	{
		RzxXNetConfig::setServerName(ui->server_name->text() );
		RzxXNetConfig::setServerPort(ui->server_port->value() );
		restart();
	}

	RzxXNetConfig::setReconnection(ui->reconnection->value() * 1000 );
	RzxXNetConfig::setPingTimeout(ui->ping_timeout->value() * 1000 );
}

/** \reimp */
void RzxProtocole::propClose()
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

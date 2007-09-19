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
#define RZX_MODULE_DESCRIPTION "Native support for the xNet protocole version 4.0"
#define RZX_MODULE_ICON RzxThemedIcon(Rzx::ICON_NETWORK)

#include <QStringList>
#include <QRegExp>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QRegExpValidator>
#include <QPixmap>
#include <QImage>
#include <QTimer>
#include <QDialog>
#include <QCryptographicHash>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxWrongPass>
#include <RzxChangePass>
#include <RzxApplication>
#include <RzxTranslator>
#include <RzxConnectionLister>

#include "rzxprotocole.h"
#include "rzxxnetconfig.h"

#include "ui_rzxxnetprop.h"

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
	: RzxNetwork(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Native support for the xNet protocole version 4.0"), RZX_MODULE_VERSION),
		 ui(NULL), propWidget(NULL)
{
	setIcon(RZX_MODULE_ICON);
	valid = auth = false;
	RzxTranslator::connect(this, SLOT(translate()));
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
						   cmd.cap(5).toUInt(0, 16), //Hash de l'icône
						   cmd.cap(6).toUInt(0, 16), //Flags ?????
						   cmd.cap(7)); //Remarque
				
				//Si le pass donné par le serveur est encore valide, alors, on demande le changement de pass
				if(testOldPass && !RzxXNetConfig::oldPass().isNull())
				{
					testOldPass = false;
					m_oldPass = RzxXNetConfig::oldPass();
					wantChangePass();
				}
				if(!valid)
				{
					valid = true;
					RzxXNetConfig::setPass(RzxXNetConfig::pass());
				}
				break;
				
			case SERVER_SYSMSG:
				if (!cmd.cap(1).isEmpty())
				  emit info(this, cmd.cap(1));
				break;
				
			case SERVER_PING:
				pingReceived();
				break;

			case SERVER_WRONGPASS:
				valid = false;
				if(!RzxXNetConfig::oldPass().isNull() && !testOldPass)
				{
					sendAuth(RzxXNetConfig::oldPass());
					testOldPass = true;
				}
				else
				{
					stop();
					wrongTime.start();
					new RzxWrongPass(this);
					RzxXNetConfig::global()->setOldPass();
				}
				break;

			case SERVER_FATAL:
				if (!cmd.cap(1).isEmpty())
					emit fatal(this, cmd.cap(1));
				break;
				
			case SERVER_PASS:
				RzxXNetConfig::global()->setOldPass(cmd.cap(1));
				m_oldPass = cmd.cap(1);
				wantChangePass();
				break;
			
			case SERVER_CHANGEPASSOK:
				emit info(this, tr("Your password has been successfully changed on the server. Remember it carefully, it can be useful."));
				if(!RzxXNetConfig::oldPass().isNull()) RzxXNetConfig::setOldPass();
				RzxXNetConfig::setPass(m_newPass);
				break;
			
			case SERVER_CHANGEPASSFAILED:
				emit info(this, tr("Server was unable to change your pass :\n") + cmd.cap(1));
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
/// Défini le mot de passe à utiliser
void RzxProtocole::usePass(const QString& pass)
{
	QString pwd(pass);
	if(pwd.length())
	{
		pwd += MD5_ADD;
		pwd = QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Md5).toHex();
		RzxXNetConfig::setPass(pwd);
		if(wrongTime.elapsed() >= 5000)
			beginAuth();
		else
			QTimer::singleShot(5000 - wrongTime.elapsed(), this, SLOT(beginAuth()));
	}
	else
		stop();
}

/** No descriptions */
void RzxProtocole::beginAuth()
{
	sendAuth(RzxXNetConfig::pass());
}

///Envoie les messages d'authentification pour initier la connexion
void RzxProtocole::sendAuth(const QString& passcode)
{
	QString msg = "VERSION 4.0\r\n";
	msg = msg + "PASS " + passcode + "\r\n";
	msg = msg + "JOIN " + RzxComputer::localhost()->serialize(serialPattern) + "\r\n";
	
	//On ne lance pas de process d'authentification si la connexion est déjà établie
	if(!auth)
	{
		if(!isStarted()) start();
		send(msg);
	}
	auth = true;
}

///Envoie les informations concernant localhost au serveur...
void RzxProtocole::sendRefresh()
{
	if(!auth)
	{
		beginAuth();
		return;
	}

	if(!valid)
		return;

	RzxComputer *me = RzxConnectionLister::global()->getComputerByName(RzxComputer::localhost()->name());
	if(me && me->icon().serialNumber() != RzxComputer::localhost()->icon().serialNumber())
		sendIcon(RzxComputer::localhost()->icon().toImage());

	QString msg = "REFRESH ";
	msg = msg + RzxComputer::localhost()->serialize(serialPattern) + "\r\n";
	send(msg);
}

///Demande la fermeture de la connexion
void RzxProtocole::sendPart() {
	send("PART\r\n");
}

///Réponse au PING envoyé par le serveur
/** Permet de tester l'état de la connexion
 */
void RzxProtocole::sendPong() {
	send("PONG\r\n");
}

///Demande le téléchargement de l'icône associée à l'ip indiquée
void RzxProtocole::getIcon(const RzxHostAddress& ip) {
	QString msg = "DOWNLOAD " + QString::number(ip.toRezix(), 16) + "\r\n";
	send(msg);
}

/****************************************************************************
* CHANGEMENT DU PASS
*/
///Demande à l'utilisateur d'entrer un nouveau mot de passe
void RzxProtocole::wantChangePass()
{
	new RzxChangePass(this, m_oldPass);
}

///Envoie au serveur une demande de changement de mot de passe
void RzxProtocole::changePass(const QString& newPass)
{
	m_newPass = newPass + MD5_ADD;
	m_newPass = QCryptographicHash::hash(m_newPass.toLatin1(), QCryptographicHash::Md5).toHex();
	send("CHANGEPASS " + m_newPass + "\r\n");
}


/****************************************************************************
* FENÊTRE DE PROPRIÉTÉS
*/

/** \reimp */
QList<QWidget*> RzxProtocole::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxXNetProp;
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

///Mise à jour de la traduction
void RzxProtocole::translate()
{
	if(ui)
		ui->retranslateUi(propWidget);
}

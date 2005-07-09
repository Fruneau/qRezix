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

#include "rzxprotocole.h"

#include "ui_rzxwrongpassui.h"

#include "rzxhostaddress.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "md5.h"

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
	: QObject()
{
	changepass = NULL;
}

///Destruction...
RzxProtocole::~RzxProtocole(){
}


/** Analyse les messages serveur -> client. Cette methode ne gere
pas les messages ICON du fait des donnees binaires qui arrivent. Elle est
reimplementer dans RzxServerListener */
void RzxProtocole::parse(const QString& msg){
	bool ok=true;
	quint32 val;
	static bool testOldPass = false;
	
	QRegExp cmd;
	for (int i = 0; ServerFormat[i]; i++)
	{
		cmd.setPattern(ServerFormat[i]);
		if (cmd.search(msg) == -1)
			continue;
		
		switch (i)
		{
			case SERVER_JOIN:
			case SERVER_REFRESH:
				emit login(RzxHostAddress::fromRezix(cmd.cap(1).toUInt(0, 16)), //IP
						   cmd.cap(2), //Nom de la machine 
						   cmd.cap(3).toUInt(0, 16), //Options
						   cmd.cap(4).toUInt(0, 16), //Version du client
						   cmd.cap(5).toUInt(0, 16), //Hash de l'ic�ne
						   cmd.cap(6).toUInt(0, 16), //Flags ?????
						   cmd.cap(7)); //Remarque
				
				//Si le pass donn� par le serveur est encore valide, alors, on demande le changement de pass
				if(testOldPass && !RzxConfig::oldPass().isNull())
				{
					testOldPass = false;
					changePass(RzxConfig::oldPass());
				}
				break;
				
			case SERVER_SYSMSG:
				if (!cmd.cap(1).isEmpty())
				  emit sysmsg(cmd.cap(1));
				break;
				
			case SERVER_PING:
				emit ping();
				break;

			case SERVER_WRONGPASS:
				if(!RzxConfig::oldPass().isNull() && !testOldPass)
				{
					sendAuth(RzxConfig::oldPass(), RzxConfig::localHost());
					testOldPass = true;
				}
				else
				{
					Ui::RzxWrongPassUI wp;
					QDialog wpDialog;
					wp.setupUi(&wpDialog);
					RzxConfig::globalConfig()->setOldPass();
					QIcon icon(RzxConfig::themedIcon("ok"));
					wp.btnOK->setIconSet(icon);
					wpDialog.exec();
					QString pwd = wp.ledPassword->text();
					if(pwd.length())
					{
						pwd += MD5_ADD;
						pwd=MD5String(pwd.latin1());
						qDebug(pwd);
						sendAuth(pwd, RzxConfig::localHost());
						RzxConfig::globalConfig()->setPass(pwd);
					}
				}
				break;

			case SERVER_FATAL:
				if (!cmd.cap(1).isEmpty())
					emit fatal(cmd.cap(1));
				break;
				
			case SERVER_PASS:
				RzxConfig::globalConfig()->setOldPass(cmd.cap(1));
				changePass(cmd.cap(1));
				break;
			
			case SERVER_CHANGEPASSOK:
				emit sysmsg(tr("Your pass has been successfully changed by the server. Keep it well because it can be useful."));
				if(!RzxConfig::oldPass().isNull()) RzxConfig::globalConfig()->setOldPass();
				RzxConfig::globalConfig()->setPass(m_newPass);
				RzxConfig::globalConfig()->flush();
				break;
			
			case SERVER_CHANGEPASSFAILED:
				emit sysmsg(tr("Server can't change your pass :\n") + cmd.cap(1));
				RzxConfig::globalConfig()->setPass(m_oldPass);
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

void RzxProtocole::sendAuth(const QString& passcode, RzxComputer * thisComputer) {
	QString msg = "VERSION 4.0\r\n";
	msg = msg + "PASS " + passcode + "\r\n";
	msg = msg + "JOIN " + thisComputer->serialize(serialPattern) + "\r\n";
	
	emit send(msg);	
}

void RzxProtocole::sendRefresh(RzxComputer * thisComputer) {
	QString msg = "REFRESH ";
	msg = msg + thisComputer->serialize(serialPattern) + "\r\n";
	emit send(msg);
}

void RzxProtocole::sendPart() {
	emit send("PART\r\n");
}
void RzxProtocole::sendPong() {
	emit send("PONG\r\n");
}
void RzxProtocole::getIcon(const RzxHostAddress& ip) {
	QString msg = "DOWNLOAD " + QString::number(ip.toRezix(), 16) + "\r\n";
	emit send(msg);
}

/****************************************************************************
* CHANGEMENT DU PASS
*/
void RzxProtocole::changePass(const QString& oldPass)
{
	if(oldPass.isNull())
		changepass = new QDialog();
	else
		changepass = new QDialog(NULL, Qt::WStyle_Customize | Qt::WStyle_StaysOnTop);
	changepassui.setupUi(changepass);
	
	//Application du masque pour �tre sur du formatage du password
	changepassui.leNewPass->setValidator(new QRegExpValidator(QRegExp(".{6,63}"), this));
	connect(changepassui.leNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	changepassui.btnOK->setEnabled(false);
	
	//Rajout des ic�nes aux boutons
	QIcon ok, cancel;
	ok.addPixmap(RzxConfig::themedIcon("ok"));
	cancel.addPixmap(RzxConfig::themedIcon("cancel"));
	changepassui.btnOK->setIconSet(ok);
	changepassui.btnCancel->setIconSet(cancel);

	//Connexion des boutons comme il va bien
	connect(changepassui.btnOK, SIGNAL(clicked()), this, SLOT(validChangePass()));
	connect(changepassui.btnCancel, SIGNAL(clicked()), this, SLOT(cancelChangePass()));
	
	//Affichage de la fen�tre
	changepass->raise();
	changepass->show();
}

void RzxProtocole::validChangePass()
{
	if(!changepass) return;
	
	//Si le nouveau texte et sa confirmation coincident, on envoie la requ�te au serveur
	//et on ferme la fen�tre
	if(changepassui.leNewPass->text() == changepassui.leReenterNewPass->text())
	{
		m_newPass = changepassui.leNewPass->text();
       //avec hash:
		m_newPass = m_newPass+MD5_ADD;
		m_newPass = MD5String(m_newPass.latin1());
		emit send("CHANGEPASS " + m_newPass + "\r\n");

		changepass->deleteLater();
		changepass = NULL;
	}
}

void RzxProtocole::cancelChangePass()
{
	if(!changepass) return;
	changepass->deleteLater();
	changepass = NULL;
}

void RzxProtocole::analyseNewPass()
{
	changepassui.btnOK->setEnabled(changepassui.leNewPass->hasAcceptableInput());
}


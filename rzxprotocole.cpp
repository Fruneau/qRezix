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
#include <qstringlist.h>
#include <qregexp.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qvalidator.h>
#include <qpixmap.h>
#include <qimage.h>

#include "rzxprotocole.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxwrongpassui.h"
#include "md5.h"
#define MD5_ADD "Vive le BR"
const char * RzxProtocole::ServerFormat[] = {
	"^JOIN ([0-9A-Fa-f]+ .* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ .*)\r\n",
	"^REFRESH ([0-9A-Fa-f]+ .* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ .*)\r\n",
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

const unsigned int RzxProtocole::ServerCounts[] = {
	7, 7, 1, 0, 1, 1, 2, 0, 1
};

RzxProtocole::RzxProtocole()
	: QObject(0, "Protocole")
{
	changepass = NULL;
}

RzxProtocole::RzxProtocole(const char * name)
	: QObject(0, name)
{
	changepass = NULL;
}

RzxProtocole::~RzxProtocole(){
}


/** Analyse les messages serveur -> client. Cette methode ne gere
pas les messages ICON du fait des donnees binaires qui arrivent. Elle est
reimplementer dans RzxServerListener */
void RzxProtocole::parse(const QString& msg){
	// on supprime l'en-tete du message
/*	QString msgClean = msg, msgParams;
	msgClean.stripWhiteSpace();
	int offset = msgClean.find(" ");
	if (offset >= 0)
		msgParams = msgClean.right(msgClean.length() - offset);
	msgParams = msgParams.stripWhiteSpace();*/
		
	bool ok=true; unsigned long val;
	
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
				emit login(cmd.cap(1));
				break;
				
			case SERVER_SYSMSG:
				if (!cmd.cap(1).isEmpty())
				  emit sysmsg(cmd.cap(1));
				break;
				
			case SERVER_PING:
				emit ping();
				break;

			case SERVER_WRONGPASS:
				{
					RzxWrongPassUI wp;
					QIconSet icon(*RzxConfig::themedIcon("ok"));
					wp.btnOK->setIconSet(icon);
					wp.exec();
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
				RzxConfig::globalConfig()->setPass(cmd.cap(1));
				RzxConfig::globalConfig()->flush();
				
				changePass(cmd.cap(1));
				break;
			
			case SERVER_CHANGEPASSOK:
				emit sysmsg(tr("Your pass has been successfully changed by the server. Keep it well because it can be useful."));
				RzxConfig::globalConfig()->setPass(m_newPass);
				RzxConfig::globalConfig()->flush();
				break;
			
			case SERVER_CHANGEPASSFAILED:
				emit sysmsg(tr("Server can't change your pass :\n") + cmd.cap(1));
				RzxConfig::globalConfig()->setPass(m_oldPass);
				break;
				
			case SERVER_PART:
				val = cmd.cap(1).toULong(&ok, 16);
				if (ok)
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

/** Extracts the parameters of the command specified */
QStringList RzxProtocole::split(char sep, const QString& command, unsigned int count){
	QStringList ret; QString temp;
	
	int begin = 0, end = 0;

	while(end >= 0 && ret.count() < count - 1) {
		end = command.find(sep, begin);
		if (end >= 0)			
			temp = command.mid(begin, end - begin);
		else
			temp = command.right(command.length() - begin);
			
		temp = temp.stripWhiteSpace();
		ret.append(temp);
			
		begin = end + 1;
	}
	if (begin >= 0 && end != -1)
		ret.append(command.right(command.length() - begin));
		
	while(ret.count() < count){	ret.append("");};
	
	return ret;
}


/*******************************************************************************
* MESSAGES A ENVOYER AU SERVEUR
*/

void RzxProtocole::sendAuth(const QString& passcode, RzxComputer * thisComputer) {
	QString msg = "VERSION 3.9\r\n";
	msg = msg + "PASS " + passcode + "\r\n";
/*      //avec hash:
	QString pass_added=passcode+MD5_ADD;
	QString hash=MD5String(pass_added.latin1());
	msg = msg + "HASH " + hash + "\r\n";*/

	msg = msg + "JOIN " + thisComputer -> serialize() + "\r\n";
	
	emit send(msg);	
}

void RzxProtocole::sendRefresh(RzxComputer * thisComputer) {
	QString msg = "REFRESH ";
	msg = msg + thisComputer -> serialize() + "\r\n";
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
	if(!oldPass)
		changepass = new RzxChangePassUI(NULL, "ChangePass");
	else
		changepass = new RzxChangePassUI(NULL, "ChangePass", false, WStyle_Customize | WStyle_StaysOnTop);
	
	//Application du masque pour être sur du formatage du password
	changepass->leNewPass->setValidator(new QRegExpValidator(QRegExp(".{6,63}"), this));
	connect(changepass->leNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	changepass->btnOK->setEnabled(false);
	
	//Rajout des icônes aux boutons
	QIconSet ok, cancel;
	ok.setPixmap(*RzxConfig::themedIcon("ok"), QIconSet::Automatic);
	cancel.setPixmap(*RzxConfig::themedIcon("cancel"), QIconSet::Automatic);
	changepass->btnOK->setIconSet(ok);
	changepass->btnCancel->setIconSet(cancel);
	
	//Si on nous donne un ancien pass, alors on force le changement
	//donc, on met le pass dans la fenêtre qui va bein, et on la désactive
	//et en même temps on désactive le bouton annuler
	if(oldPass)
	{
		changepass->leOldPass->setText(oldPass);
		changepass->leOldPass->setEnabled(false);
		changepass->btnCancel->setEnabled(false);
	}
	
	//Connexion des boutons comme il va bien
	connect(changepass->btnOK, SIGNAL(clicked()), this, SLOT(validChangePass()));
	connect(changepass->btnCancel, SIGNAL(clicked()), this, SLOT(cancelChangePass()));
	
	//Affichage de la fenêtre
	changepass->raise();
	changepass->show();
}

void RzxProtocole::validChangePass()
{
	if(!changepass) return;
	
	//Si le nouveau texte et sa confirmation coincident, on envoie la requête au serveur
	//et on ferme la fenêtre
	if(changepass->leNewPass->text() == changepass->leReenterNewPass->text())
	{
		m_oldPass = changepass->leOldPass->text();
		m_newPass = changepass->leNewPass->text();
/*		qDebug(m_oldPass + " ==> " + m_newPass);
		emit send("CHANGEPASS " + m_oldPass + " " + m_newPass + "\r\n");*/
       //avec hash:
		m_oldPass = m_oldPass+MD5_ADD;
		m_newPass = m_newPass+MD5_ADD;
		m_oldPass = MD5String(m_oldPass.latin1());
		m_newPass = MD5String(m_newPass.latin1());
		qDebug(m_oldPass + " " + m_newPass);
		emit send("CHANGEPASS " + m_oldPass + " " + m_newPass + "\r\n");

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
	changepass->btnOK->setEnabled(changepass->leNewPass->hasAcceptableInput());
}


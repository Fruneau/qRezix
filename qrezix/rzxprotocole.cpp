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

#include "rzxprotocole.h"
#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "rzxwrongpassui.h"

const char * RzxProtocole::ServerFormat[] = {
	"^JOIN [0-9A-Fa-f]+ .* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ .*",
	"^REFRESH [0-9A-Fa-f]+ .* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ .*",
	"^SYSMSG .*",
	"^PING",
	"^PASS [0-9]+",
	"^PART [0-9A-Za-z]+",
	"^UPGRADE v[0-9]+,[0-9]+,[0-9]+ .*",
	"^FATAL",
	"^ICON [0-9A-Fa-f]+",
	"^WRONGPASS",
	0
};

const unsigned int RzxProtocole::ServerCounts[] = {
	7, 7, 1, 0, 1, 1, 2, 0, 1
};

RzxProtocole::RzxProtocole()
	: QObject(0, "Protocole") {
}
RzxProtocole::RzxProtocole(const char * name)
	: QObject(0, name) {
}
RzxProtocole::~RzxProtocole(){
}


/** Analyse les messages serveur -> client. Cette methode ne gere
pas les messages ICON du fait des donnees binaires qui arrivent. Elle est
reimplementer dans RzxServerListener */
void RzxProtocole::parse(const QString& msg){
	// on supprime l'en-tete du message
	QString msgClean = msg, msgParams;
	msgClean.stripWhiteSpace();
	int offset = msgClean.find(" ");
	if (offset >= 0)
		msgParams = msgClean.right(msgClean.length() - offset);
	msgParams = msgParams.stripWhiteSpace();
		
	bool ok=true; unsigned long val;
	
	QRegExp cmd;
	for (int i = 0; ServerFormat[i]; i++)
	{
		cmd.setPattern(ServerFormat[i]);
		if (cmd.match(msg) == -1)
			continue;
		
		switch (i)
		{
			case SERVER_JOIN:
			case SERVER_REFRESH:
				emit login(msgParams);
				break;
				
			case SERVER_SYSMSG:
				if (!msgParams.isEmpty())
				  emit sysmsg(msgParams);
				break;
				
			case SERVER_PING:
				emit ping();
				break;

			case SERVER_WRONGPASS:
				{
					RzxWrongPassUI wp;
					wp.exec();
					QString pwd = wp.ledPassword->text();
					if(pwd.length())
					{
						val = pwd.toInt(&ok, 16);
						if (ok) {
							sendAuth(val, RzxConfig::localHost());
							RzxConfig::globalConfig()->setPass(val);
						}
					}
				}
				break;

			case SERVER_FATAL:
				if (!msgParams.isEmpty())
					emit fatal(msgParams);
				break;
				
			case SERVER_PASS:
				emit sysmsg(QString(tr("Your XNet password  is  : %1\n"
					                   "This is an identification code used to authentificate your connection to the server and avoid IP-spoofing.\n\n"
									   "KEEP IT WELL because without it, you may not be able to connect to the server")).arg(msgParams));
				val = msgParams.toInt(&ok, 16);
				if (ok) {
					RzxConfig::globalConfig()->setPass(ok);
				}
				break;
				
			case SERVER_PART:
				val = msgParams.toULong(&ok, 16);
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

void RzxProtocole::sendAuth(int passcode, RzxComputer * thisComputer) {
	QString msg = "VERSION 3.9\r\n";
	msg = msg + "PASS " + QString::number(passcode, 16) + "\r\n";
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

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

#ifndef RZXPROTOCOLE_H
#define RZXPROTOCOLE_H

#include "rzxhostaddress.h"
#include <qobject.h>
#include <qstring.h>

/**
	*Gere le protocole Xnet. Attention, les messages SERVER_ICON sont definis mais
	* ne sont pas geres - pb de gestion des donnees binaires.
	* RzxServerListener gere lui les icones.
	* NB: on remarquera que pour la meme raison, aucune fonction n'est definie pour
	* envoyer des icones au serveur
  * @author Sylvain Joyeux
  */

class QStringList;
class RzxComputer;

class RzxProtocole : public QObject{
	Q_OBJECT
	
public: 
	RzxProtocole();
	RzxProtocole(const char * name);
	~RzxProtocole();
 	
	enum Icons {
		ICON_SIZE=12288,
		ICON_WIDTH=64,
		ICON_HEIGHT=64
	};
		
 	static const char * ServerFormat[];
 	static const unsigned int ServerCounts[];
 	enum ServerCommands {
 		SERVER_JOIN = 0,
		SERVER_REFRESH = 1,
		SERVER_SYSMSG = 2,
		SERVER_PING = 3,
		SERVER_PASS = 4,
		SERVER_PART = 5,
		SERVER_UPGRADE = 6,
		SERVER_FATAL = 7,
		SERVER_ICON = 8
	};

	static QStringList split(char sep, const QString& command, unsigned int count);
	virtual void parse(const QString& msg);

	/** Realise une sequence d'authentification aupres du serveur
	*@param thisComputer donnees concernant l'ordinateur local */	
	void sendAuth(int passcode, RzxComputer * thisComputer);
	/** Envoie une commande refresh
	*@param thisComputer donnees concernant l'ordinateur local */
	void sendRefresh(RzxComputer * thisComputer);
	/** Envoie le message de deconnection */
	void sendPart();
	
public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	void getIcon(const RzxHostAddress& ip);
	/** Envoie un pong */
	void sendPong();
		
signals: // Signals
	/** ping() est emit quand on passe une commande PING a @ref parse */
 	void ping();
	/** login() est emit quand on passe une commande JOIN ou REFRESH a @ref parse
	*C'est au slot de verifier si oui ou non newComputer a deja ete reference
	*@param newComputer donnees concernant l'ordinateur */
 	void login(const QString& newComputer);
	/** logout() est emit quand on passe une commande PART @ref parse
	*@param ip IP de l'ordinateur qui se deconnecte de rezix */
 	void logout(const RzxHostAddress& ip);
	/** sysmsg() est emit quand on passe une commande SYSMSG a @ref parse
	*@param msg message envoye par le serveur */
	void sysmsg(const QString& msg);
	/** fatal() est emit quand on passe une commande FATAL a @ref parse
	*@param msg message envoye par le serveur */
 	void fatal(const QString& msg);
 	
 	/** emit quand l'objet RzxProtocole a besoin d'envoyer une commande au serveur,
 	*typiquement lors d'appel aux fonctions @ref sendAuth, @ref sendRefresh,
 	*@ref sendPart, @ref sendPong et @ref getIcon */
	void send(const QString& msg);

};

#endif

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

#include <qobject.h>
#include <qstring.h>

#include "rzxhostaddress.h"
#include "ui_rzxchangepassui.h"

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
	
	QString m_oldPass;
	QString m_newPass;
	Ui::RzxChangePassUI changepassui;
	QDialog *changepass;
	
	///Patron de formatage des objets
	static const QString serialPattern;

public: 
	RzxProtocole();
	~RzxProtocole();
	
	enum Icons {
		ICON_SIZE= 64*64*4,
		ICON_WIDTH=64,
		ICON_HEIGHT=64
	};
		
 	static const char * ServerFormat[];
 	enum ServerCommands {
 		SERVER_JOIN = 0,
		SERVER_REFRESH = 1,
		SERVER_SYSMSG = 2,
		SERVER_PING = 3,
		SERVER_PASS = 4,
		SERVER_PART = 5,
		SERVER_UPGRADE = 6,
		SERVER_FATAL = 7,
		SERVER_ICON = 8,
		SERVER_WRONGPASS = 9,
		SERVER_CHANGEPASSOK = 10,
		SERVER_CHANGEPASSFAILED = 11,
		SERVER_UPLOAD = 12
	};

	virtual void parse(const QString& msg);

public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	void getIcon(const RzxHostAddress& ip);
	/** Envoie un pong */
	void sendPong();
	/** Realise une sequence d'authentification aupres du serveur
	*@param thisComputer donnees concernant l'ordinateur local */	
	void sendAuth(const QString& passcode);
	/** Envoie le message de deconnection */
	void sendPart();
	/** Envoie une commande refresh
	*@param thisComputer donnees concernant l'ordinateur local */
	void sendRefresh();
	
	/** Demande de changement de pass */
	void changePass(const QString& oldPass = QString::null);
	
protected slots:
	/** Validation d'un changement de pass */
	void validChangePass();
	/** Annulation d'un changement de pass */
	void cancelChangePass();
	/** Analyse du texte tapé pour le nouveau pass */
	void analyseNewPass();

signals: // Signals
	/** ping() est emit quand on passe une commande PING a @ref parse */
 	void ping();
	/** login() est emit quand on passe une commande JOIN ou REFRESH a @ref parse
	*C'est au slot de verifier si oui ou non newComputer a deja ete reference
	*@param newComputer donnees concernant l'ordinateur */
 	void login(const RzxHostAddress &ip, const QString& name, quint32 options, quint32 version, quint32 stamp, quint32 flags, const QString& comment);
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

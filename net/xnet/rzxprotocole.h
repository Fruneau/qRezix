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

#include <QObject>
#include <QString>

#include <RzxHostAddress>
#include <RzxNetwork>


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
namespace Ui { class RzxXNetPropUI; };

class RzxProtocole : public RzxNetwork
{
	Q_OBJECT
	
	QString m_oldPass;
	QString m_newPass;
	
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

protected slots:
	///envoie d'un message
 	/** Utilisé quand l'objet RzxProtocole a besoin d'envoyer une commande au serveur,
 	 * typiquement lors d'appel aux fonctions @ref sendAuth, @ref sendRefresh,
 	 * @ref sendPart, @ref sendPong et @ref getIcon
	 */
	virtual void send(const QString&) = 0;
	///Réception d'un ping
	virtual void pingReceived() = 0;

public slots:
	/** Demande un envoi d'icone
	*@param ip ip de l'hote dont on veut l'icone */
	virtual void getIcon(const RzxHostAddress& ip);
	/** Envoie un pong */
	void sendPong();
	/** Realise une sequence d'authentification aupres du serveur
	*@param thisComputer donnees concernant l'ordinateur local */	
	void sendAuth(const QString& passcode);
	/** Surchage sendAuth en envoyant avec le pass par défaut */
	void beginAuth();
	/** Envoie le message de deconnection */
	void sendPart();
	/** Envoie une commande refresh
	*@param thisComputer donnees concernant l'ordinateur local */
	void sendRefresh();
	virtual void refresh();
	
	/** Demande de changement de pass */
	virtual void wantChangePass();
	virtual void changePass(const QString&);
	virtual void usePass(const QString&);


//Gestion de propriétés du module
private:
	Ui::RzxXNetPropUI *ui;
	QWidget *propWidget;

public:
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();

public slots:
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();

 };

inline void RzxProtocole::refresh()
{
	sendRefresh();
}

#endif

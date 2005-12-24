/***************************************************************************
                          rzxnetwork  -  description
                             -------------------
    begin                : Sun Aug 21 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <RzxBaseModule>

#ifndef RZXNETWORK_H
#define RZXNETWORK_H

#include <QObject>
#include <RzxLoaderProp>

class RzxHostAddress;
class RzxComputer;

/**
 @author Florent Bruneau
 */

///Classe de base d'un module réseau
/** Cette classe implémente la base d'un module réseau. La construction
 * ne doit pas lancer les connexions. Celle-ci ne doivent ne doivent être lancées que
 * par start. L'appel à stop entraine la fermeture de la connexion.
 *
 * Une fois la connexion établie, le module communique via différents messages :
 * 	- login pour indiquer une nouvelle connexion, ou une mise à jour
 * 	- logout pour indiquer une déconnexion
 * 	- refresh pour envoyer les mises à jour des informations du client
 */
class RZX_CORE_EXPORT RzxNetwork : public QObject, public RzxBaseModule
{
	Q_OBJECT
	Q_PROPERTY(Type type READ type)
	Q_PROPERTY(bool isInitialised READ isInitialised)
	Q_ENUMS(TypeFlags)
	Q_FLAGS(Type)

	public:
		enum TypeFlags {
			TYP_NONE = 0,
			TYP_CHAT = 1,
			TYP_PROPERTIES = 2
		};
		Q_DECLARE_FLAGS(Type, TypeFlags)

	private:
		Type m_type;

	public:
		RzxNetwork(const QString&, const QString&, const Rzx::Version&);
		virtual ~RzxNetwork();

		const Type &type() const;
		virtual bool isInitialised() const;
		virtual bool isStarted() const = 0;

	protected slots:
		void setType(const Type&);

	public slots:
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void restart();
		virtual void refresh() = 0;
		virtual void getIcon(const RzxHostAddress&) = 0;
		virtual void usePass(const QString&) = 0;
		virtual void changePass(const QString&) = 0;
		virtual void wantChangePass() = 0;

		virtual void chat(RzxComputer*);
		virtual void properties(RzxComputer*);

		///Averti que le module a reçu un message de type Chat
		/** Ce message ne doit être envoyé que par les objets de type \ref MOD_CHAT
		 */
		virtual void sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

	signals:
		///Le module vient de connecter
		/** Ce message doit être émis une seule et unique fois à la connexion !!!
		 */
		void connected(RzxNetwork*);

		///Le module vient de se déconnecter
		/** Ce message doit être émis une seule et unique fois à la déconnexion !!!
		 */
		void disconnected(RzxNetwork*);

		///On a reçu l'adresse IP du client d'après les informations de connexion avec le serveur
		void receiveAddress(const RzxHostAddress&);
		
		///La machine a l'adresse indiquée vient de se connecter ou de mettre à jour les information la concernant
		void login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);

		///La machine à l'adresse indiquée s'est déconnectée
		void logout(const RzxHostAddress&);

		///Le protocole demande l'affichage d'un message d'erreur
		void fatal(RzxNetwork*, const QString&);

		///Le protocole demande l'affichage d'un message d'avertissement
		void warning(RzxNetwork*, const QString&);

		///Le protocole demande l'affichage d'un message d'information
		void info(RzxNetwork*, const QString&);

		///L'état de connexion a changé
		void status(RzxNetwork*, const QString&);

		///Indique qu'on a reçu l'icône pour l'adresse indiquée
		void receivedIcon(QImage*, const RzxHostAddress&);

		///Indique qu'on attribue une icône pour l'adresse indiquée
		void receivedIcon(const QPixmap&, const RzxHostAddress&);

		///Avertis de la réception de nouvelles propriétés pour un RzxComputer
		/** Ce signal indique que de nouvelles propriétés ont été obtenues pour une
		 * machine. Chacun devra alors utiliser ce signal pour stocker ou afficher les
		 * propriétés correspondante.
		 *
		 * Ce signal doit être émis après le stockage officiel des propriétés.
		 *
		 * \param displayed doit être mis à true si l'information a été interceptée
		 * et communiquée à l'utilisateur ou autre par un objet.
		 */
		void haveProperties(RzxComputer*);
};

///Le module est initialisé par défaut
inline bool RzxNetwork::isInitialised() const
{
	return true;
}

typedef RzxLoaderProp<RzxNetwork> RzxNetworkLoaderProp;

///Définition de la fonction d'export d'un module network
#define RZX_NETWORK_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getNetwork, getNetworkName, getNetworkVersion, RzxNetwork, MODULE)

#endif

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

///Classe de base d'un module r�seau
/** Cette classe impl�mente la base d'un module r�seau. La construction
 * ne doit pas lancer les connexions. Celle-ci ne doivent ne doivent �tre lanc�es que
 * par start. L'appel � stop entraine la fermeture de la connexion.
 *
 * Une fois la connexion �tablie, le module communique via diff�rents messages :
 * 	- login pour indiquer une nouvelle connexion, ou une mise � jour
 * 	- logout pour indiquer une d�connexion
 * 	- refresh pour envoyer les mises � jour des informations du client
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

		///Averti que le module a re�u un message de type Chat
		/** Ce message ne doit �tre envoy� que par les objets de type \ref MOD_CHAT
		 */
		virtual void sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

	signals:
		///Le module vient de connecter
		/** Ce message doit �tre �mis une seule et unique fois � la connexion !!!
		 */
		void connected(RzxNetwork*);

		///Le module vient de se d�connecter
		/** Ce message doit �tre �mis une seule et unique fois � la d�connexion !!!
		 */
		void disconnected(RzxNetwork*);

		///On a re�u l'adresse IP du client d'apr�s les informations de connexion avec le serveur
		void receiveAddress(const RzxHostAddress&);
		
		///La machine a l'adresse indiqu�e vient de se connecter ou de mettre � jour les information la concernant
		void login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);

		///La machine � l'adresse indiqu�e s'est d�connect�e
		void logout(const RzxHostAddress&);

		///Le protocole demande l'affichage d'un message d'erreur
		void fatal(RzxNetwork*, const QString&);

		///Le protocole demande l'affichage d'un message d'avertissement
		void warning(RzxNetwork*, const QString&);

		///Le protocole demande l'affichage d'un message d'information
		void info(RzxNetwork*, const QString&);

		///L'�tat de connexion a chang�
		void status(RzxNetwork*, const QString&);

		///Indique qu'on a re�u l'ic�ne pour l'adresse indiqu�e
		void receivedIcon(QImage*, const RzxHostAddress&);

		///Indique qu'on attribue une ic�ne pour l'adresse indiqu�e
		void receivedIcon(const QPixmap&, const RzxHostAddress&);

		///Avertis de la r�ception de nouvelles propri�t�s pour un RzxComputer
		/** Ce signal indique que de nouvelles propri�t�s ont �t� obtenues pour une
		 * machine. Chacun devra alors utiliser ce signal pour stocker ou afficher les
		 * propri�t�s correspondante.
		 *
		 * Ce signal doit �tre �mis apr�s le stockage officiel des propri�t�s.
		 *
		 * \param displayed doit �tre mis � true si l'information a �t� intercept�e
		 * et communiqu�e � l'utilisateur ou autre par un objet.
		 */
		void haveProperties(RzxComputer*);
};

///Le module est initialis� par d�faut
inline bool RzxNetwork::isInitialised() const
{
	return true;
}

typedef RzxLoaderProp<RzxNetwork> RzxNetworkLoaderProp;

///D�finition de la fonction d'export d'un module network
#define RZX_NETWORK_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getNetwork, getNetworkName, getNetworkVersion, RzxNetwork, MODULE)

#endif

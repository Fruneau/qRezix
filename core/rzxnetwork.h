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


class RzxHostAddress;

/**
 @author Florent Bruneau
 */

///Classe de base d'un module r�eau
/** Cette classe impl�ente la base d'un module r�eau. La construction
 * ne doit pas lancer les connexions. Celle-ci ne doivent ne doivent �re lanc�s que
 * par start. L'appel �stop entra�e la fermeture de la connexion.
 *
 * Une fois la connexion �ablie, le module communique via diff�ents messages :
 * 	- login pour indiquer une nouvelle connexion, ou une mise �jour
 * 	- logout pour indiquer une d�onnexion
 * 	- refresh pour envoyer les mises �jour des informations du client
 */
class RzxNetwork : public QObject, public RzxBaseModule
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
			TYP_PREFERENCES = 2
		};
		Q_DECLARE_FLAGS(Type, TypeFlags)

	private:
		Type m_type;

	public:
		RzxNetwork(const QString&, const QString&, int, int, int, const QString& = QString());
		RzxNetwork(const QString&, const QString&, const Rzx::Version&);
		RzxNetwork(const QString&, const QString&);
		virtual ~RzxNetwork();

		const Type &type() const;
		virtual bool isInitialised() const;
		virtual bool isStarted() const = 0;

	protected slots:
		void setType(const Type&);

	public slots:
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void refresh() = 0;
		virtual void getIcon(const RzxHostAddress&) = 0;
		virtual void usePass(const QString&) = 0;
		virtual void changePass(const QString&) = 0;
		virtual void wantChangePass() = 0;

	signals:
		void connected(RzxNetwork*);
		void disconnected(RzxNetwork*);
		void receiveAddress(const RzxHostAddress&);
		
		void login(RzxNetwork*, const RzxHostAddress&, const QString&, quint32, quint32, quint32, quint32, const QString&);
		void logout(const RzxHostAddress&);

		void fatal(const QString&);
		void warning(const QString&);
		void info(const QString&);
		void status(const QString&);

		void receivedIcon(QImage*, const RzxHostAddress&);
};

///Le module est initialis�par d�aut
inline bool RzxNetwork::isInitialised() const
{
	return true;
}

///D�inition de la fonction d'export d'un module network
#define RZX_NETWORK_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getNetwork, RzxNetwork, MODULE)

#endif

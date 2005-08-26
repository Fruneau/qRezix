/***************************************************************************
                          rzxhostaddress.h  -  description
                             -------------------
    begin                : Sat Jan 26 2002
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

#ifndef RZXHOSTADDRESS_H
#define RZXHOSTADDRESS_H

#include <QHostAddress>

#include <RzxGlobal>

/**
  *@author Sylvain Joyeux
  */

///Gestion des adresses IP
/** Fournit une extension légère apportant quelques utilitaires
 * nécessaires pour une interface simple avec la totalité des
 * composants réseaux du programme et avec en particulier les
 * particularités du protocole xNet
 */
class RzxHostAddress : public QHostAddress 
{
	///Chaînes de caractères représentant les différents sous-réseau
	static const char *rezalText[Rzx::RZL_NUMBER][2];

	public:
		RzxHostAddress();
		RzxHostAddress(const QHostAddress& host);
		RzxHostAddress(quint32 ip);
		~RzxHostAddress();

		static RzxHostAddress fromRezix(quint32 ip);
		quint32 toRezix() const;

		operator quint32() const;

		Rzx::RezalId rezal() const;
		QString rezalName(bool = true) const;
		static QString rezalName(Rzx::RezalId, bool = true);
		
		bool isSameGateway(const RzxHostAddress&) const;
		static bool isSameGateway(const RzxHostAddress&, const RzxHostAddress&);
};

///Constructeur
inline RzxHostAddress::RzxHostAddress() { }

///Constructeur par extension d'un QHostAddress
inline RzxHostAddress::RzxHostAddress(const QHostAddress& host)
	:QHostAddress(host) { }

///Contructeur à partir d'une ip sous la forme d'un entier
inline RzxHostAddress::RzxHostAddress(quint32 ip)
	:QHostAddress(ip) { }

///Destruction de l'objet
inline RzxHostAddress::~ RzxHostAddress() { }

///Indique si deux adresses sont sur le même sous réseau
inline bool RzxHostAddress::isSameGateway(const RzxHostAddress& a) const
{
	return rezal() == a.rezal();
}

///Surcharge pour simplifier l'utilisation
inline bool RzxHostAddress::isSameGateway(const RzxHostAddress& a, const RzxHostAddress& b)
{
	return a.rezal() == b.rezal();
}

#endif

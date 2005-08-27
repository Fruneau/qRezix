/***************************************************************************
                          rzxhostaddress.cpp  -  description
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
#include <QRegExp>

#include <RzxConfig>
#include <RzxHostAddress>


///Cr�ation d'un RzxHostAddress � partir de la donn�e fournie par le protocole xNet
/** Le protocole xNet a le malheur de transmettre les IPs � l'envers... :/, et donc
 * pour pouvoir construire un QHostAddress, il faut swapper l'int...
 */
RzxHostAddress RzxHostAddress::fromRezix(quint32 tempIP) {
	RzxHostAddress ret;
	quint32 ip;
	ip = 0;
	ip |= (tempIP >> 24) & 0xFF;
	ip |= ((tempIP >> 16) & 0xFF) << 8;
	ip |= ((tempIP >> 8) & 0xFF) << 16;
	ip |= (tempIP & 0xFF) << 24;
	ret.setAddress(ip);
	return ret;
}

///Convertie le RzxHostAddress dans le format utilisable par le protocole xNet
/** Pour le m�me raisons que \ref fromRezix, il faut retourner l'adresse
 */
quint32 RzxHostAddress::toRezix() const
{
	quint32 tempIP = toIPv4Address();
	quint32 ip;
	ip = 0;
	ip |= (tempIP >> 24) & 0xFF;
	ip |= ((tempIP >> 16) & 0xFF) << 8;
	ip |= ((tempIP >> 8) & 0xFF) << 16;
	ip |= (tempIP & 0xFF) << 24;
	return ip;
}

///Converti l'objet en entier
/** L'entier est l'adresse IPv4... � voir le jour o� on ajoute le support de l'IPv6
 */
RzxHostAddress::operator quint32() const
{
	return toIPv4Address();
}

///Permet de retrouver le 'nom' du sous-r�seau sur lequel se trouve la machine
/** Permet de donner un nom au sous-r�seau de la machine.
 */
int RzxHostAddress::rezal() const
{
	return RzxConfig::rezal(*this);
}

///Retourne la version texte du nom du sous-r�seau
/** Ne fait que r�aliser la conversion en cha�ne de caract�res du RezalId */
QString RzxHostAddress::rezalName(bool shortname) const
{
	return RzxConfig::rezalName((QHostAddress)*this, shortname);
}

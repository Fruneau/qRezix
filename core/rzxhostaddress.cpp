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

#include <RzxConfig>
#include <RzxHostAddress>
#include <RzxConnectionLister>

///Inverse les octes d'un objet de 32bits
quint32 swap(quint32 tempIP)
{
	quint32 ip;
	ip = 0;
	ip |= (tempIP >> 24) & 0xFF;
	ip |= ((tempIP >> 16) & 0xFF) << 8;
	ip |= ((tempIP >> 8) & 0xFF) << 16;
	ip |= (tempIP & 0xFF) << 24;
	return ip;
}

///Création d'un RzxHostAddress à partir de la donnée fournie par le protocole xNet
/** Le protocole xNet a le malheur de transmettre les IPs à l'envers... :/, et donc
 * pour pouvoir construire un QHostAddress, il faut swapper l'int...
 */
RzxHostAddress RzxHostAddress::fromRezix(quint32 tempIP)
{
	return RzxHostAddress(swap(tempIP));
}

///Convertie le RzxHostAddress dans le format utilisable par le protocole xNet
/** Pour le même raisons que \ref fromRezix, il faut retourner l'adresse
 */
quint32 RzxHostAddress::toRezix() const
{
	return swap(toIPv4Address());
}

///Converti l'objet en entier
/** L'entier est l'adresse IPv4... à voir le jour où on ajoute le support de l'IPv6
 */
RzxHostAddress::operator quint32() const
{
	return toIPv4Address();
}

///Retrouve l'ordinateur associé à l'adresse donnée
/** Cette fonction a pour but de simplifier les transitions entre les IP et machinie
 * qui peuvent parfois allourdir le code inutilement.
 */
RzxComputer *RzxHostAddress::computer() const
{
	return RzxConnectionLister::global()->getComputerByIP(*this);
}

///Converti l'objet en un RzxComputer*
/** Simple utilisation de RzxConnectionLister::global()->getComputerByIP()...
 * Cette fonction est surtout pratique pour simplifier éviter d'avoir à faire des
 * conversion à longueur de temps dans certains modules (le chat entre autre.
 */
RzxHostAddress::operator RzxComputer*() const
{
	return computer();
}

///Permet de retrouver le 'nom' du sous-réseau sur lequel se trouve la machine
/** Permet de donner un nom au sous-réseau de la machine.
 */
int RzxHostAddress::rezal() const
{
	return RzxConfig::rezal(*this);
}

///Retourne la version texte du nom du sous-réseau
/** Ne fait que réaliser la conversion en chaîne de caractères du RezalId */
QString RzxHostAddress::rezalName(bool shortname) const
{
	return RzxConfig::rezalName((QHostAddress)*this, shortname);
}

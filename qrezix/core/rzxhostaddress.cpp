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

#include "rzxhostaddress.h"

const char *RzxHostAddress::rezalText[Rzx::RZL_NUMBER][2] = {
	{ "World", "World" },
	{ "Binets & Kès", "Binets" },
	{ "BR", "BR" },
	{ "BEM Bat. A", "BEM.A" },
	{ "BEM Bat. D", "BEM.D" },
	{ "Foch 0", "Foch.0" },
	{ "Foch 1", "Foch.1" },
	{ "Foch 2", "Foch.2" },
	{ "Foch 3", "Foch.3" },
	{ "Fayolle 0", "Fay.0" },
	{ "Fayolle 1", "Fay.1" },
	{ "Fayolle 2", "Fay.2" },
	{ "Fayolle 3", "Fay.3" },
	{ "PEM", "PEM" },
	{ "Joffre 0", "Jof.0" },
	{ "Joffre 1", "Jof.1" },
	{ "Joffre 2", "Jof.2" },
	{ "Joffre 3", "Jof.3" },
	{ "Maunoury 0", "Mau.0" },
	{ "Maunoury 1", "Mau.1" },
	{ "Maunoury 2", "Mau.2" },
	{ "Maunoury 3", "Mau.3" },
	{ "Batiment 411", "Bat.411" },
	{ "Batiment 70", "Bat.70" },
	{ "Btiment 71", "Bat.71" },
	{ "Batiment 72", "Bat.72" },
	{ "Batiment 73", "Bat.73" },
	{ "Batiment 74", "Bat.74" },
	{ "Batiment 75", "Bat.75" },
	{ "Batiment 76", "Bat.76" },
	{ "Batiment 77", "Bat.77" },
	{ "Batiment 78", "Bat.78" },
	{ "Batiment 79", "Bat.79" },
	{ "Batiment 80", "Bat.80" },
	{ "Wifi", "Wifi" },
	{ "X", "X" }
};

///Création d'un RzxHostAddress à partir de la donnée fournie par le protocole xNet
/** Le protocole xNet a le malheur de transmettre les IPs à l'envers... :/, et donc
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
/** Pour le même raisons que \ref fromRezix, il faut retourner l'adresse
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
/** L'entier est l'adresse IPv4... à voir le jour où on ajoute le support de l'IPv6
 */
RzxHostAddress::operator quint32() const
{
	return toIPv4Address();
}

///Permet de retrouver le 'nom' du sous-réseau sur lequel se trouve la machine
/** Permet de donner un nom au sous-réseau de la machine.
 * A terme cette fonction lira les données à partir d'un fichier qui contiendra les correspondances
 */
Rzx::RezalId RzxHostAddress::rezal() const
{
	QRegExp mask("(\\d{1,3}\\.\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");

	if(mask.indexIn(toString()) == -1) return Rzx::RZL_WORLD;
	
	//Si l'ip n'est pas de l'X
	if(mask.cap(1) != "129.104") return Rzx::RZL_WORLD;
	
	int resal = mask.cap(2).toUInt();
	int adr = mask.cap(3).toUInt();
	//Kes et binet... avec un cas particulier pour le BR :))
	if(resal == 201)
	{
		if(adr >= 50 && adr <= 62) return Rzx::RZL_BR;
		return Rzx::RZL_BINETS;
	}
	
	//Cas des bar d'étages
	//Comment ça y'a pas de bar au premier étage ? faudra dire ça à NC :þ
	if(resal == 208 || resal == 212 || resal == 218 || resal == 222)
	{
		if(adr >= 97 && adr <= 100) resal -= 2;
		if(adr >= 101 && adr <= 104) resal--;
	}

	//Pour prendre en compte les nouveaux caserts...
	//En espérant tout de même qu'un jour ils porteronts des vrais nom :(
	if(resal >= 224 && resal <= 229)
	{
		int greaterMask = (resal<<1) + (adr>>7);
		switch(greaterMask)
		{
			case (224<<1)+0: return Rzx::RZL_BAT71;
			case (224<<1)+1: return Rzx::RZL_BAT70;
			case (225<<1)+0: return Rzx::RZL_BAT73;
			case (225<<1)+1: return Rzx::RZL_BAT74;
			case (226<<1)+0: return Rzx::RZL_BAT75;
			case (226<<1)+1: return Rzx::RZL_BAT80;
			case (227<<1)+0: return Rzx::RZL_BAT76;
			case (227<<1)+1: return Rzx::RZL_BAT77;
			case (228<<1)+0: return Rzx::RZL_BAT78;
			case (228<<1)+1: return Rzx::RZL_BAT72;
			case (229<<1)+0: return Rzx::RZL_BAT79;
		}
	}

	switch(resal)
	{
		//Pour le BEM
		case 203: return Rzx::RZL_BEMA;
		case 204: return Rzx::RZL_BEMD;

		//Pour le PEM & BAT411
		case 214: return Rzx::RZL_PEM;
		case 223: return Rzx::RZL_BAT411;

		//Pour Foch, Fayolle, Joffre et Maunoury
		case 205: return Rzx::RZL_FOCH0;
		case 206: return Rzx::RZL_FOCH1;
		case 207: return Rzx::RZL_FOCH2;
		case 208: return Rzx::RZL_FOCH3;
		case 209: return Rzx::RZL_FAYOLLE0;
		case 210: return Rzx::RZL_FAYOLLE1;
		case 211: return Rzx::RZL_FAYOLLE2;
		case 212: return Rzx::RZL_FAYOLLE3;
		case 215: return Rzx::RZL_JOFFRE0;
		case 216: return Rzx::RZL_JOFFRE1;
		case 217: return Rzx::RZL_JOFFRE2;
		case 218: return Rzx::RZL_JOFFRE3;
		case 219: return Rzx::RZL_MAUNOURY0;
		case 220: return Rzx::RZL_MAUNOURY1;
		case 221: return Rzx::RZL_MAUNOURY2;
		case 222: return Rzx::RZL_MAUNOURY3;

		//Pour le wifi
		case 230: return Rzx::RZL_WIFI;

		//Si y'a rien, on sait au moins que c'est à l'X...
		default: return Rzx::RZL_X;
	}
}

///Retourne la version texte du nom du sous-réseau
/** Ne fait que réaliser la conversion en chaîne de caractères du RezalId */
QString RzxHostAddress::rezalName(bool shortname) const
{ return rezalText[rezal()][shortname?1:0]; }

///Retourne le texte associé au rezalId
/** Uniquement fournit dans le but de simplifier la gestion par d'autre classes... à utiliser avec parcimonie */
QString RzxHostAddress::rezalName(Rzx::RezalId rezalId, bool shortname)
{
	if(rezalId < 0 || rezalId >= Rzx::RZL_NUMBER)
		rezalId = Rzx::RZL_WORLD;
	return rezalText[rezalId][shortname?1:0];
}

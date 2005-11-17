/***************************************************************************
                          rzxthemedicon.cpp  -  description
                             -------------------
    begin                : Thu Aug 11 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : fruneau@melix.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QString>

#include <RzxIconCollection>

#include <RzxThemedIcon>

///Construction d'une icône associé à l'identifiant
RzxThemedIcon::RzxThemedIcon(Rzx::Icon icon)
	:m_type(Id), m_icon(icon)
{
}

///Construction d'une icône correspondant au nom du fichier
RzxThemedIcon::RzxThemedIcon(const QString& name)
	:m_type(Name), m_name(name)
{
}

///Construction d'une icône correspondant au système d'exploitation
RzxThemedIcon::RzxThemedIcon(Rzx::SysEx sysex)
	:m_type(OS), m_sysex(sysex)
{
}

///Construction d'une icône correspondant à la promo
RzxThemedIcon::RzxThemedIcon(Rzx::Promal promo)
	:m_type(Promo), m_promo(promo)
{
}

///Construction d'une icône à partir d'un QIcon
RzxThemedIcon::RzxThemedIcon(const QIcon &icon)
	:m_type(Icon), m_qicon(icon)
{
}

///Construction d'une icône correspondant au type indiqué...
/** Si le type nécessite une information complémentaire, il faut utiliser
 * le constructeur adéquate.
 */
RzxThemedIcon::RzxThemedIcon(Type type)
	:m_type(type)
{
	if(type == Id || type == OS || type == Name)
		type = Invalid;
}

///Construction par recopie
RzxThemedIcon::RzxThemedIcon(const RzxThemedIcon& a)
	:QObject()
{
	*this = a;
}

///Destructeur...
RzxThemedIcon::~RzxThemedIcon()
{
}

///Indique si l'objet est valide.
/** Si l'object est invalide il sera casté en une icône nulle
 */
bool RzxThemedIcon::isValid() const
{
	return m_type != Invalid;
}

///Retourne le type d'icône
RzxThemedIcon::Type RzxThemedIcon::type() const
{
	return m_type;
}

///Pour la recopie de l'objet
const RzxThemedIcon &RzxThemedIcon::operator=(const RzxThemedIcon& a)
{
	m_type = a.m_type;
	switch(m_type)
	{
		case Id: m_icon = a.m_icon; break;
		case Name: m_name = a.m_name; break;
		case OS: m_sysex = a.m_sysex; break;
		case Promo: m_promo = a.m_promo; break;
		case Icon: m_qicon = a.m_qicon; break;
		default: break;
	}
	return *this;
}

///Retourne l'icône correspondante
QIcon RzxThemedIcon::icon() const
{
	switch(m_type)
	{
		case Name: return RzxIconCollection::getIcon(m_name);
		case Id: return RzxIconCollection::getIcon(m_icon);
		case OS: return RzxIconCollection::global()->osIcon(m_sysex);
		case Promo: return RzxIconCollection::global()->promoIcon(m_promo);
		case Responder: return RzxIconCollection::getResponderIcon();
		case Sound: return RzxIconCollection::getSoundIcon();
		case OnOff: return RzxIconCollection::getOnOffIcon();
		case Favorite: return RzxIconCollection::getFavoriteIcon();
		case Ban: return RzxIconCollection::getBanIcon();
		case Icon: return m_qicon;
		default: return QIcon();
	}
}

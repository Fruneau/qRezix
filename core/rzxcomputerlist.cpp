/***************************************************************************
          rzxcomputerlist  -  gestion d'une liste de machines
                             -------------------
    begin                : Wed Dec 14 2005
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
#include <QSettings>
#include <RzxComputer>
#include <RzxHostAddress>
#include <RzxConnectionLister>

#include <RzxComputerList>

///Construction
/** On défini le type une fois pour toute... ainsi que le QSettings lié
 * et la clé du QSettings à laquelle sont stockées les informations.
 */
RzxComputerList::RzxComputerList(Type m_type, QSettings *m_settings, const QString &m_key)
	:settings(m_settings), key(m_key), type(m_type)
{
	read();
}

///Destruction...
RzxComputerList::~RzxComputerList()
{
	write();
}

///Lecture des données de configuration et intégration de ces données à la liste
void RzxComputerList::read()
{
	if(!settings) return;

	QStringList confList = settings->value(key).toStringList();
	if(type == Name)
		nameList = confList;
	else if(type == Address)
	{
		nameList = settings->value(key + "_names").toStringList();
		for(int i = 0 ; i < confList.size() ; i++)
			addressList << confList[i];

		while(nameList.size() < addressList.size())
			nameList << QString();
	}
}

///Ecriture des données de configuration
void RzxComputerList::write()
{
	if(!settings) return;
	
	if(type == Name)
		settings->setValue(key, nameList);
	else if(type == Address)
	{
		settings->setValue(key + "_names", nameList);
		QStringList confList;
		for(int i = 0 ; i < addressList.size() ; i++)
			confList << addressList[i].toString();
		settings->setValue(key, confList);
	}
}

///Ajout d'un élément
/** Par son nom de machine
 */
void RzxComputerList::add(const QString& name)
{
	remove(name);
	if(type == Name)
		nameList << name;
	else if(type == Address)
	{
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
		if(!computer) return;
		addressList << computer->ip();
		nameList << computer->name();
	}
}

///Ajout d'un élément
/** Par son adresse
 */
void RzxComputerList::add(const RzxHostAddress& address)
{
	remove(address);
	const RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(address);
	if(type == Name)
	{
		if(!computer) return;
		nameList << computer->name();
	}
	else if(type == Address)
	{
		addressList << address;
		if(!computer) nameList << QString();
		else nameList << computer->name();
	}
}

///Ajout d'un élément
/** Par son ordinateur
 */
void RzxComputerList::add(const RzxComputer* computer)
{
	remove(computer);
	if(!computer) return;
	nameList << computer->name();
	if(type == Address)
		addressList << computer->ip();
}

///Suppression d'un élément
/** Par index dans la liste
 */
void RzxComputerList::remove(int i)
{
	if(nameList.size() >= i) nameList.removeAt(i);
	if(addressList.size() >= i) addressList.removeAt(i);
}

///Suppression d'un élément
/** Par nom
 */
void RzxComputerList::remove(const QString& name)
{
	int pos;
	while((pos = nameList.indexOf(name)) != -1)
		remove(pos);
}

///Suppression d'un élément
/** Par adresse
 */
void RzxComputerList::remove(const RzxHostAddress& address)
{
	int pos;
	while((pos = addressList.indexOf(address)) != -1)
		remove(pos);
}

///Suppression d'un élément
/** Par machine associée
 */
void RzxComputerList::remove(const RzxComputer* computer)
{
	if(!computer) return;
	if(type == Name)
		remove(computer->name());
	else if(type == Address)
		remove(computer->ip());
}

///Test si un objet est dans la liste
/** Par nom
 */
bool RzxComputerList::isIn(const QString& name) const
{
	return nameList.contains(name);
}

///Test si un objet est dans la liste
/** Par adresse
 */
bool RzxComputerList::isIn(const RzxHostAddress& address) const
{
	return addressList.contains(address);
}

///Test si un objet est dans la liste
/** Par machine associée
 */
bool RzxComputerList::isIn(const RzxComputer *computer) const
{
	if(!computer) return false;
	if(type == Name)
		return isIn(computer->name());
	else //pas de test type == Addres pour éviter les warning le compilation
		return isIn(computer->ip());
}

///Retourne la liste des noms
QStringList RzxComputerList::names() const
{
	return nameList;
}

///Retourne la liste des adresses
QList<RzxHostAddress> RzxComputerList::addresses() const
{
	if(type == Address)
		return addressList;
	else
	{
		QList<RzxHostAddress> addresses;
		for(int i = 0 ; i < nameList.size() ; i++)
		{
			const RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(nameList[i]);
			if(!computer) addresses << RzxHostAddress();
			else addresses << computer->ip();
		}
		return addresses;
	}
}

///Retourne la liste des ordinateurs
QList<RzxComputer*> RzxComputerList::computers() const
{
	QList<RzxComputer*> computers;
	for(int i = 0 ; i < nameList.size() ; i++)
	{
		RzxComputer *computer;
		if(type == Name)
			computer = RzxConnectionLister::global()->getComputerByName(nameList[i]);
		else
			computer = RzxConnectionLister::global()->getComputerByIP(addressList[i]);
		computers << computer;
	}
	return computers;
}

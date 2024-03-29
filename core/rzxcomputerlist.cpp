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
/** On d�fini le type une fois pour toute... ainsi que le QSettings li�
 * et la cl� du QSettings � laquelle sont stock�es les informations.
 */
RzxComputerList::RzxComputerList(Type m_type, QSettings *settings, const QString &m_key)
	:m_settings(settings), key(m_key), type(m_type)
{
	read();
}

///Destruction...
RzxComputerList::~RzxComputerList()
{
	write();
}

///Lecture des donn�es de configuration et int�gration de ces donn�es � la liste
void RzxComputerList::read()
{
	if(!m_settings) return;

	QStringList confList = m_settings->value(key).toStringList();
	if(confList.size() == 1 && confList[0].isEmpty()) return;

	if(type == Name)
		nameList = confList;
	else if(type == Address)
	{
		nameList = m_settings->value(key + "_names").toStringList();
		for(int i = 0 ; i < confList.size() ; i++)
			addressList << confList[i];

		while(nameList.size() < addressList.size())
			nameList << QString();
	}
}

///Ecriture des donn�es de configuration
void RzxComputerList::write()
{
	if(!m_settings) return;
	
	if(type == Name)
		m_settings->setValue(key, nameList);
	else if(type == Address)
	{
		m_settings->setValue(key + "_names", nameList);
		QStringList confList;
		for(int i = 0 ; i < addressList.size() ; i++)
			confList << addressList[i].toString();
		m_settings->setValue(key, confList);
	}
}

///Retourne le QSettings de stockage
QSettings *RzxComputerList::settings() const
{
	return m_settings;
}

///Ajout d'un �l�ment
/** Par son nom de machine
 */
void RzxComputerList::add(const QString& name)
{
	remove(name);
	if(type == Name)
	{
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(name);
		if(!computer)
			nameList << name;
		else
			nameList << computer->name();
	}
	else if(type == Address)
	{
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
		if(!computer) return;
		addressList << computer->ip();
		nameList << computer->name();
	}
	write();
}

///Ajout d'un �l�ment
/** Par son adresse
 */
void RzxComputerList::add(const RzxHostAddress& address)
{
	remove(address);
	const RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(address);
	if(type == Name)
	{
		if(!computer)
			nameList << address.toString();
		else
			nameList << computer->name();
	}
	else if(type == Address)
	{
		addressList << address;
		if(!computer) nameList << QString();
		else nameList << computer->name();
	}
	write();
}

///Ajout d'un �l�ment
/** Par son ordinateur
 */
void RzxComputerList::add(const RzxComputer* computer)
{
	remove(computer);
	if(!computer) return;
	nameList << computer->name();
	if(type == Address)
		addressList << computer->ip();
	write();
}

///Suppression d'un �l�ment
/** Par index dans la liste
 */
void RzxComputerList::remove(int i)
{
	if(nameList.size() >= i) nameList.removeAt(i);
	if(addressList.size() >= i) addressList.removeAt(i);
	write();
}

///Suppression d'un �l�ment
/** Par nom
 */
void RzxComputerList::remove(const QString& name)
{
	int pos;
	RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
	if(!computer)
		computer = RzxConnectionLister::global()->getComputerByIP(name);
	if(!computer)
	{
		while((pos = nameList.indexOf(name)) != -1)
			remove(pos);
	}
	else
	{
		while((pos = nameList.indexOf(computer->name())) != -1)
			remove(pos);
		while((pos = nameList.indexOf(computer->ip().toString())) != -1)
			remove(pos);
	}
	remove(RzxHostAddress(name));
}

///Suppression d'un �l�ment
/** Par adresse
 */
void RzxComputerList::remove(const RzxHostAddress& address)
{
	int pos;
	while((pos = addressList.indexOf(address)) != -1)
		remove(pos);
}

///Suppression d'un �l�ment
/** Par machine associ�e
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
bool RzxComputerList::contains(const QString& name) const
{
	return nameList.contains(name, Qt::CaseInsensitive);
}

///Test si un objet est dans la liste
/** Par adresse
 */
bool RzxComputerList::contains(const RzxHostAddress& address) const
{
	return addressList.contains(address);
}

///Test si un objet est dans la liste
/** Par machine associ�e
 */
bool RzxComputerList::contains(const RzxComputer *computer) const
{
	if(!computer) return false;
	if(type == Name)
		return contains(computer->name()) || contains(computer->ip().toString());
	else //pas de test type == Addres pour �viter les warning le compilation
		return contains(computer->ip());
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
		{
			computer = RzxConnectionLister::global()->getComputerByName(nameList[i]);
			if(!computer)
				computer = RzxConnectionLister::global()->getComputerByIP(nameList[i]);
		}
		else
			computer = RzxConnectionLister::global()->getComputerByIP(addressList[i]);
		computers << computer;
	}
	return computers;
}

///Retourne une liste de description des objets...
QStringList RzxComputerList::humanReadable() const
{
	QStringList desc;
	if(type == Name)
		desc = nameList;
	else
		for(int i = 0 ; i < addressList.size() ; i++)
			desc << addressList[i].toString();

	for(int i = 0 ; i < desc.size() ; i++)
	{
		const RzxComputer *computer = RzxConnectionLister::global()->getComputerByIP(desc[i]);
		if(computer)
			desc[i] = computer->name();
	}

	return desc;
}

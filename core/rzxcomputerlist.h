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
#ifndef RZXCOMPUTERLIST_H
#define RZXCOMPUTERLIST_H

#include <QList>
#include <QStringList>

#include <RzxHostAddress>

/**
 @author Florent Bruneau
 */

class QSettings;
class RzxComputer;

///Liste généraliste d'ordinateurs
/** Permet d'ajouter, de supprimer et d'identifier des ordinateurs ajoutés
 * à une liste... qu'ils soient ou non connectés, et ce avec différents moyens
 * de stockages :
 * 	- par nom
 * 	- par adresse
 *
 * Cette classe doit être dérivée pour permettre une utilisation correcte
 */
class RZX_CORE_EXPORT RzxComputerList
{
	QList<RzxHostAddress> addressList;
	QStringList nameList;

	QSettings *m_settings;
	QString key;

	protected:
		enum Type
		{
			Name = 0,
			Address = 1
		} type;

		RzxComputerList(Type T, QSettings*, const QString&);
		void read();
		void write();

	public:
		~RzxComputerList();

		void add(const QString&);
		void add(const RzxHostAddress&);
		void add(const RzxComputer*);

		RzxComputerList& operator<<(const QString&);
		RzxComputerList& operator<<(const RzxHostAddress&);
		RzxComputerList& operator<<(const RzxComputer*);

		void remove(int);
		void remove(const QString&);
		void remove(const RzxHostAddress&);
		void remove(const RzxComputer*);

		RzxComputerList& operator>>(const QString&);
		RzxComputerList& operator>>(const RzxHostAddress&);
		RzxComputerList& operator>>(const RzxComputer*);

		bool contains(const QString&) const;
		bool contains(const RzxHostAddress&) const;
		bool contains(const RzxComputer*) const;

		QSettings *settings() const;

		QStringList names() const;
		QList<RzxHostAddress> addresses() const;
		QList<RzxComputer*> computers() const;
		QStringList humanReadable(bool = false) const;

		operator QList<RzxComputer*>() const;
};

///Surcharge de <<, ajout d'un élément par son nom
inline RzxComputerList& RzxComputerList::operator<<(const QString& name)
{
	add(name);
	return *this;
}

///Surcharge de <<, ajout d'un élément par son adresse
inline RzxComputerList& RzxComputerList::operator<<(const RzxHostAddress& address)
{
	add(address);
	return *this;
}

///Surcharge de <<, ajout d'un élément par son ordinateur associé
inline RzxComputerList& RzxComputerList::operator<<(const RzxComputer* computer)
{
	add(computer);
	return *this;
}

///Surcharge de >>, suppression d'un élément par son nom
inline RzxComputerList& RzxComputerList::operator>>(const QString& name)
{
	remove(name);
	return *this;
}

///Surcharge de >>, suppression d'un élément par son adresse
inline RzxComputerList& RzxComputerList::operator>>(const RzxHostAddress& address)
{
	remove(address);
	return *this;
}

///Surcharge de >>, suppression d'un élément par son ordinateur associé
inline RzxComputerList& RzxComputerList::operator>>(const RzxComputer* computer)
{
	remove(computer);
	return *this;
}

///Surcharge de computers()
inline RzxComputerList::operator QList<RzxComputer*>() const
{
	return computers();
}

#endif

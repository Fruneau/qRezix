/***************************************************************************
           rzxusergroup  -  gestion d'une liste d'utilisateurs
                             -------------------
    begin                : Thu Jan 26 2006
    copyright            : (C) 2006 by Florent Bruneau
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
#ifndef RZXUSERGROUP_H
#define RZXUSERGROUP_H

#include <RzxComputerList>
#include <RzxThemedIcon>

#include "rzxmainuiglobal.h"

/**
 @author Florent Bruneau
 */

///Liste des favoris...
class RZX_MAINUI_EXPORT RzxUserGroup: public RzxComputerList
{
	static QList<RzxUserGroup*> groups;
	static QSettings *groupSettings;

	QString m_name;
	RzxThemedIcon m_icon;

	public:
		RzxUserGroup(const QString&, const QString&);
		~RzxUserGroup();

		QString name() const;
		QIcon icon() const;

		void setIcon(const QString&);

		//Gestion de l'ensemble des groups
		static void loadGroups(QSettings*);
		static void clearGroups();

		static RzxUserGroup *group(int);
		static void deleteGroup(int);
		static int groupId(const QString&);
		static int groupNumber();
};

#endif

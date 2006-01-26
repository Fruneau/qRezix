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
#include <QSettings>

#include "rzxusergroup.h"

///Pour stocker la liste des groupes disponibles
QList<RzxUserGroup*> RzxUserGroup::groups = QList<RzxUserGroup*>();

///Pour définir un QSettings utilisé par tous les groups
QSettings *RzxUserGroup::groupSettings = NULL;

///Création d'un groupe en lui définissant une icône
RzxUserGroup::RzxUserGroup(const QString& icon, const QString& name)
	:RzxComputerList(Name, groupSettings, "group-" + name), m_name(name)
{
	if(!icon.isEmpty())
		setIcon(icon);
	groups << this;
}

///Destruction du groupe
RzxUserGroup::~RzxUserGroup()
{
}

///Définition de l'icône du grouope
void RzxUserGroup::setIcon(const QString& c_icon)
{
	settings()->setValue("groupicon-"+name(), c_icon);
	m_icon = RzxThemedIcon(c_icon);
}

///Retourne le nom du groupe
QString RzxUserGroup::name() const
{
	return m_name;
}

///Retourne l'icône du groupe
QIcon RzxUserGroup::icon() const
{
	return m_icon;
}

///Charge l'ensemble des groupes disposibles sur le QSetting indiqué
void RzxUserGroup::loadGroups(QSettings *settings)
{
	groupSettings = settings;
	if(!settings) return;

	const QStringList keys = settings->childKeys();
	foreach(QString key, keys)
	{
		if(key.left(6) == "group-")
		{
			const QString name = key.mid(6);
			new RzxUserGroup(settings->value("groupicon-"+name).toString(), name);
		}
	}
}

///Libère la mémoire occupée par les groupes
void RzxUserGroup::clearGroups()
{
	qDeleteAll(groups);
	groups.clear();
	groupSettings = NULL;
}

///Retourne le ième group...
RzxUserGroup *RzxUserGroup::group(int i)
{
	if(i < 0 || i >= groups.size())
		return NULL;
	return groups[i];
}

///Retourne le nombre de groupes enregistrés
int RzxUserGroup::groupNumber()
{
	return groups.size();
}

///Supprime (et efface des préférences) le ième groupe
void RzxUserGroup::deleteGroup(int i)
{
	if(i < 0 || i >= groups.size())
		return;
	
	RzxUserGroup *group = groups[i];
	const QString name = group->name();
	groups.removeAt(i);
	delete group;
	if(groupSettings)
	{
		groupSettings->remove("group-" + name);
		groupSettings->remove("groupicon-" + name);
	}
}

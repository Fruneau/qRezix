/***************************************************************************
                      rzxpluginloader.cpp  -  description
                             -------------------
    begin                : Thu Jul 19 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qstringlist.h>
#include <qpopupmenu.h>
#include <qobject.h>
#include <qsettings.h>
#include <qvariant.h>

#include "rzxplugin.h"

/* Valeurs par d�faut des donn�es */
//Par d�faut, on a un plug-in sans nom ni description
//n'ayant ni menu pour le tray, ni menu pur les items
//ni menu d'action, ni fen�tre de propri�t�s
//ni donn�es de configuration � stocker
RzxPlugIn::RzxPlugIn() : QObject(0, 0)
{
	name = "";
	description = "";
	prop = false;
	tray = item = NULL;
	action = NULL;
	settings = NULL;
}

RzxPlugIn::RzxPlugIn(const QString& nm, const QString& desc) : QObject(0, 0)
{
	name = nm;
	description = desc;
	prop = false;
	tray = item = NULL;
	action = NULL;
	settings = NULL;
}

/* Fermeture du plugin */
RzxPlugIn::~RzxPlugIn()
{
	if(tray) delete tray;
	if(item) delete item;
	if(action) delete action;
	if(settings) delete settings;
}

/* Fonction qui retourne simplement une valeur */
//le nom du plugin
QString RzxPlugIn::getName()
{
	return name;
}

//la description du plugin
QString RzxPlugIn::getDescription()
{
	return description;
}

//est-ce que le plug-in a une fen�tre de propri�t� ou pas
bool RzxPlugIn::hasProp()
{
	return prop;
}

/** Fonction qui fournissent au concepteur de plug-in une interface
pratique pour le stockage de donn�es de configuration de mani�re
homog�ne pour l'ensemble des plugins */
//Bien que qrezix utilise le m�me fichier pour stocker ses donn�es
//il faut bien se rendre compte que pour acc�der aux donn�es, il vaut
//mieux passer par queryData qui fournit une m�thode plus sur, et non
//d�pendante de l'impl�mentation du stockage des donn�es

// Ouverture du stockage
void RzxPlugIn::initSettings(const QString& path)
{
	if(!settings)
		settings = new QSettings();
	//Pour les syst�mes unix, on va dans le r�pertoire de stockage
	settings->insertSearchPath(QSettings::Unix, path);
}

//Les fonctions d'entr�e sortie sont juste fournies par souci de 'propret�'
//toujours dans le but d'�crire avec une structure claire les donn�es
//c'est � dire dans le fichier qrezixrc, dans le groupe du plugin
// Lecture d'une entr�e.
QString RzxPlugIn::readEntry(const QString& keyname, const QString& def)
{
	return settings->readEntry("/qRezix/" + name + "/" + keyname, def);
}

// Ecriture d'une entr�e
bool RzxPlugIn::writeEntry(const QString& keyname, const QString& value)
{
	return settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/** Demande d'envoi d'un message au serveur */
//Cette ce slot permet en particulier de fournir un relais pour l'envoi
//d'un message au serveur entre le plug-in lui m�me et l'interface.
//A UTILISER SEULEMENT SI ON SAIT CE QU'ON FAIT
void RzxPlugIn::sender(const QString& msg)
{
	emit send(msg);
}

/** Demande de donn�es � qRezix */
//Envoi simplement un message � qrezix comme quoi le plug-in
//voudrait obtenir la valeur de la donn�e data
void RzxPlugIn::querySender(Data data)
{
	emit queryData(data, this);
}

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

/// Construction avec des valeurs par par défaut
/** Par défaut, on a un plug-in sans nom ni description
 * n'ayant ni menu pour le tray, ni menu pur les items
 * ni menu d'action, ni fenêtre de propriétés
 * ni données de configuration à stocker
 */
RzxPlugIn::RzxPlugIn() : QObject(0, 0)
{
	name = "";
	description = "";
	prop = false;
	tray = item = NULL;
	action = NULL;
	settings = NULL;
	chat = NULL;
	icon = NULL;
	version = PLUGIN_VERSION;
	features = 0;
}

/// Construction d'un plug-in
/** On construit un plug-in vierge avec uniquement un nom et une description
 * \param nm fournit le nom du plug-in : un chaîne courte qui n'est qu'un nom simple
 * \param desc fournit la description du plug-in : chaîne qui donne une description plus complète du plug-in
 */
RzxPlugIn::RzxPlugIn(const QString& nm, const QString& desc) : QObject(0, 0)
{
	name = nm;
	description = desc;
	prop = false;
	tray = item = NULL;
	action = NULL;
	settings = NULL;
	chat = NULL;
	icon = NULL;
	version = PLUGIN_VERSION;
	features = 0;
}

/// Fermeture du plugin
RzxPlugIn::~RzxPlugIn()
{
	if(tray) delete tray;
	if(item) delete item;
	if(action) delete action;
	if(chat) delete chat;
	if(icon) delete icon;
}

/// Retourne le nom du plug-in
QString RzxPlugIn::getName()
{
	return name;
}


/// Retourne la description du plug-in
QString RzxPlugIn::getDescription()
{
	return description;
}

/// Indique s'il y a une boîte de dialogue pour régler les propriétés
/** Cette méthode ne fait que retourner <i><b>prop</b></i> */
bool RzxPlugIn::hasProp()
{
	return prop;
}

/// Retourne l'icon du plug-in
/** L'utilisation d'une icône pour le plug-in est facultative, mais c'est le genre d'artifice qui sont toujours sympathique pour personnaliser le programme.
 * \return NULL si il n'y a pas d'icône pour le plug-in
 */
QPixmap *RzxPlugIn::getIcon()
{
	return icon;
}

/// Retourne la version du support plug-in utilisé
/** Etant donné que le support de plug-in (c'est à dire cette classe) évolue avec qRezix, il est nécessaire de maintenir une fonction qui donne la version utilisée.
 *<br><b>IMPORTANT : pour les dével de qRezix : </b> Le structure de plug-in doit être suffisamment constante pour pouvoir tester les données de base (version et nom) de toutes les versions de RzxPlugIn avec les même fonctions. Il faut donc absolument laissé dans l'ordre en début de plug-in les données et les méthodes correspondantes.
 */
int RzxPlugIn::getVersion()
{
	return version;
}

/* Fonction qui fournissent au concepteur de plug-in une interface
pratique pour le stockage de données de configuration de manière
homogène pour l'ensemble des plugins */
//Bien que qrezix utilise le même fichier pour stocker ses données
//il faut bien se rendre compte que pour accéder aux données, il vaut
//mieux passer par queryData qui fournit une méthode plus sur, et non
//dépendante de l'implémentation du stockage des données

//Les fonctions d'entrée sortie sont juste fournies par souci de 'propreté'
//toujours dans le but d'écrire avec une structure claire les données
//c'est à dire dans le fichier qrezixrc, dans le groupe du plugin
// Lecture d'une entrée.

///Changement de la classe settings
void RzxPlugIn::setSettings(QSettings *m_settings)
{
	settings = m_settings;
}

/// Lecture d'une donnée de configuration
/** Lit une entrée dans les données de configuration. La valeur retournée est une chaine de caractère
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param def valeur pas défaut du champ
 */
QString RzxPlugIn::readEntry(const QString& keyname, const QString& def)
{
	if(!settings) return def;
	return settings->readEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donnée numérique de configuration
/** Lit une entrée dans les données de configuration. La valeur retournée est un entier
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param def valeur pas défaut du champ
 */
int RzxPlugIn::readNumEntry(const QString& keyname, int def)
{
	if(!settings) return def;
	return settings->readNumEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donnée booléenne de configuration
/** Lit une entrée dans les données de configuration. La valeur retournée est un booléen
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param def valeur pas défaut du champ
 */
bool RzxPlugIn::readBoolEntry(const QString& keyname, bool def)
{
	if(!settings) return def;
	return settings->readBoolEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donnée sous forme de liste de chaîne de configuration
/** Lit une entrée dans les données de configuration. La valeur retournée est un booléen
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 */
QStringList RzxPlugIn::readListEntry(const QString& keyname)
{
	if(!settings) return QStringList();
	return settings->readListEntry("/qRezix/" + name + "/" + keyname);
}


/// Enregistrement d'une entrée dans la configuration
/** Ajoute ou modifie la valeur d'une entrée dans les configuration. La valeur est une chaine de caractère
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, const QString& value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entrée numérique dans la configuration
/** Ajoute ou modifie la valeur d'une entrée dans les configuration. La valeur est un entier
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, int value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entrée booléenne dans la configuration
/** Ajoute ou modifie la valeur d'une entrée dans les configuration. La valeur est un booléen
 * Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, bool value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entrée sous forme de liste de chaîne dans la configuration
/** Ajoute ou modifie la valeur d'une entrée dans les configuration. La valeur est un booléen
 * Cette fonction surcharge celle de QSettings pour une question de commodité quand à la hiérarchie des chemins de stockage
 * \param keyname nom de la clef à lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, const QStringList& value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}


/// Demande d'envoi d'un message au serveur
/** Cette ce slot permet en particulier de fournir un relais pour l'envoi
 * d'un message au serveur entre le plug-in lui même et l'interface.
 * <br><b>A UTILISER SEULEMENT SI ON SAIT CE QU'ON FAIT</b>
 * \param msg contient le message complet envoyé au serveur
 */
void RzxPlugIn::sender(const QString& msg)
{
	emit send(msg);
}

/// Demande de données à qRezix
/** Envoi simplement un message à qrezix comme quoi le plug-in voudrait obtenir la valeur de la donnée data.
 * \param data indique ce qu'on recherche comme donnée
 */
void RzxPlugIn::querySender(Data data)
{
	emit queryData(data, this);
}

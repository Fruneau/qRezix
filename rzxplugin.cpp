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

/// Construction avec des valeurs par par d�faut
/** Par d�faut, on a un plug-in sans nom ni description
 * n'ayant ni menu pour le tray, ni menu pur les items
 * ni menu d'action, ni fen�tre de propri�t�s
 * ni donn�es de configuration � stocker
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
 * \param nm fournit le nom du plug-in : un cha�ne courte qui n'est qu'un nom simple
 * \param desc fournit la description du plug-in : cha�ne qui donne une description plus compl�te du plug-in
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

/// Indique s'il y a une bo�te de dialogue pour r�gler les propri�t�s
/** Cette m�thode ne fait que retourner <i><b>prop</b></i> */
bool RzxPlugIn::hasProp()
{
	return prop;
}

/// Retourne l'icon du plug-in
/** L'utilisation d'une ic�ne pour le plug-in est facultative, mais c'est le genre d'artifice qui sont toujours sympathique pour personnaliser le programme.
 * \return NULL si il n'y a pas d'ic�ne pour le plug-in
 */
QPixmap *RzxPlugIn::getIcon()
{
	return icon;
}

/// Retourne la version du support plug-in utilis�
/** Etant donn� que le support de plug-in (c'est � dire cette classe) �volue avec qRezix, il est n�cessaire de maintenir une fonction qui donne la version utilis�e.
 *<br><b>IMPORTANT : pour les d�vel de qRezix : </b> Le structure de plug-in doit �tre suffisamment constante pour pouvoir tester les donn�es de base (version et nom) de toutes les versions de RzxPlugIn avec les m�me fonctions. Il faut donc absolument laiss� dans l'ordre en d�but de plug-in les donn�es et les m�thodes correspondantes.
 */
int RzxPlugIn::getVersion()
{
	return version;
}

/* Fonction qui fournissent au concepteur de plug-in une interface
pratique pour le stockage de donn�es de configuration de mani�re
homog�ne pour l'ensemble des plugins */
//Bien que qrezix utilise le m�me fichier pour stocker ses donn�es
//il faut bien se rendre compte que pour acc�der aux donn�es, il vaut
//mieux passer par queryData qui fournit une m�thode plus sur, et non
//d�pendante de l'impl�mentation du stockage des donn�es

//Les fonctions d'entr�e sortie sont juste fournies par souci de 'propret�'
//toujours dans le but d'�crire avec une structure claire les donn�es
//c'est � dire dans le fichier qrezixrc, dans le groupe du plugin
// Lecture d'une entr�e.

///Changement de la classe settings
void RzxPlugIn::setSettings(QSettings *m_settings)
{
	settings = m_settings;
}

/// Lecture d'une donn�e de configuration
/** Lit une entr�e dans les donn�es de configuration. La valeur retourn�e est une chaine de caract�re
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param def valeur pas d�faut du champ
 */
QString RzxPlugIn::readEntry(const QString& keyname, const QString& def)
{
	if(!settings) return def;
	return settings->readEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donn�e num�rique de configuration
/** Lit une entr�e dans les donn�es de configuration. La valeur retourn�e est un entier
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param def valeur pas d�faut du champ
 */
int RzxPlugIn::readNumEntry(const QString& keyname, int def)
{
	if(!settings) return def;
	return settings->readNumEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donn�e bool�enne de configuration
/** Lit une entr�e dans les donn�es de configuration. La valeur retourn�e est un bool�en
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param def valeur pas d�faut du champ
 */
bool RzxPlugIn::readBoolEntry(const QString& keyname, bool def)
{
	if(!settings) return def;
	return settings->readBoolEntry("/qRezix/" + name + "/" + keyname, def);
}

/// Lecture d'une donn�e sous forme de liste de cha�ne de configuration
/** Lit une entr�e dans les donn�es de configuration. La valeur retourn�e est un bool�en
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 */
QStringList RzxPlugIn::readListEntry(const QString& keyname)
{
	if(!settings) return QStringList();
	return settings->readListEntry("/qRezix/" + name + "/" + keyname);
}


/// Enregistrement d'une entr�e dans la configuration
/** Ajoute ou modifie la valeur d'une entr�e dans les configuration. La valeur est une chaine de caract�re
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, const QString& value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entr�e num�rique dans la configuration
/** Ajoute ou modifie la valeur d'une entr�e dans les configuration. La valeur est un entier
 * <br>Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, int value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entr�e bool�enne dans la configuration
/** Ajoute ou modifie la valeur d'une entr�e dans les configuration. La valeur est un bool�en
 * Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, bool value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}

/// Enregistrement d'une entr�e sous forme de liste de cha�ne dans la configuration
/** Ajoute ou modifie la valeur d'une entr�e dans les configuration. La valeur est un bool�en
 * Cette fonction surcharge celle de QSettings pour une question de commodit� quand � la hi�rarchie des chemins de stockage
 * \param keyname nom de la clef � lire
 * \param value nouvelle valeur du champ
 */
void RzxPlugIn::writeEntry(const QString& keyname, const QStringList& value)
{
	if(!settings) return;
	settings->writeEntry("/qRezix/" + name + "/" + keyname, value);
}


/// Demande d'envoi d'un message au serveur
/** Cette ce slot permet en particulier de fournir un relais pour l'envoi
 * d'un message au serveur entre le plug-in lui m�me et l'interface.
 * <br><b>A UTILISER SEULEMENT SI ON SAIT CE QU'ON FAIT</b>
 * \param msg contient le message complet envoy� au serveur
 */
void RzxPlugIn::sender(const QString& msg)
{
	emit send(msg);
}

/// Demande de donn�es � qRezix
/** Envoi simplement un message � qrezix comme quoi le plug-in voudrait obtenir la valeur de la donn�e data.
 * \param data indique ce qu'on recherche comme donn�e
 */
void RzxPlugIn::querySender(Data data)
{
	emit queryData(data, this);
}

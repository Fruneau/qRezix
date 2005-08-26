/***************************************************************************
                          rzxrezal.cpp  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include "rzxrezal.h"

///Construction du rezal à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
 */
RzxRezal::RzxRezal(const QString& name, const QString& description, int major, int minor, int build, const QString& tag)
	:RzxBaseModule(name, description, major, minor, build, tag)
{
}

///Construction d'un rezal
/** Fonction surchargée
 */
RzxRezal::RzxRezal(const QString& name, const QString& description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version)
{
}

///Construction d'un rezal à partir d'une chaîne décrivant son nom version
/** Cette fonction surchargée présente l'intérêt de permettre la construction d'un module simplemente à partir
 * de 2 chaînes de caractères. La première décrivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce même module.
 *
 * Le nom est donné de la forme :
 * "nom major.minor.build-tag"
 */
RzxRezal::RzxRezal(const QString& name, const QString& description)
	:RzxBaseModule(name, description)
{
}

///Destruction du module
RzxRezal::~RzxRezal()
{
}

///On dit que le module est initialisé correctement par défaut...
/** Ce n'est peut-être pas le plus intelligent, mais au moins
 * c'est pas trop fatiguant...
 */
bool RzxRezal::isInitialised() const
{
	return true;
}

///Ajout d'un flag aux type du module
void RzxRezal::setType(const Type& flag)
{
	m_type |= flag;
}

///Récupération du type du module
const RzxRezal::Type& RzxRezal::type() const
{
	return m_type;
}

///Met à jour l'affichage
/** L'implémentation par défaut ne fait rien
 */
void RzxRezal::updateLayout()
{
}

/***************************************************************************
                          rzxnetwork  -  description
                             -------------------
    begin                : Sun Aug 21 2005
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
#include <RzxNetwork>

///Construction du network � partir des diff�rentes informations
/** Les informations � fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit �tre le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin � l'utilisateur
 * 	- num�ro de version major.minor.build-tag
 */
RzxNetwork::RzxNetwork(const QString& name, const QString& description, int major, int minor, int build, const QString& tag)
	:RzxBaseModule(name, description, major, minor, build, tag)
{
}

///Construction d'un network
/** Fonction surcharg�e
 */
RzxNetwork::RzxNetwork(const QString& name, const QString& description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version)
{
}

///Construction d'un network � partir d'une cha�ne d�crivant son nom version
/** Cette fonction surcharg�e pr�sente l'int�r�t de permettre la construction d'un module simplemente � partir
 * de 2 cha�nes de caract�res. La premi�re d�crivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce m�me module.
 *
 * Le nom est donn� de la forme :
 * "nom major.minor.build-tag"
 */
RzxNetwork::RzxNetwork(const QString& name, const QString& description)
	:RzxBaseModule(name, description)
{
}

///Destruction du module
RzxNetwork::~RzxNetwork()
{
}

///Ajout d'un flag aux type du module
void RzxNetwork::setType(const Type& flag)
{
	m_type |= flag;
}

///R�cup�ration du type du module
const RzxNetwork::Type& RzxNetwork::type() const
{
	return m_type;
}

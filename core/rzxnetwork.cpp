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

///Construction du network à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
 */
RzxNetwork::RzxNetwork(const QString& name, const QString& description, int major, int minor, int build, const QString& tag)
	:RzxBaseModule(name, description, major, minor, build, tag)
{
}

///Construction d'un network
/** Fonction surchargée
 */
RzxNetwork::RzxNetwork(const QString& name, const QString& description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version)
{
}

///Construction d'un network à partir d'une chaîne décrivant son nom version
/** Cette fonction surchargée présente l'intérêt de permettre la construction d'un module simplemente à partir
 * de 2 chaînes de caractères. La première décrivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce même module.
 *
 * Le nom est donné de la forme :
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

///Récupération du type du module
const RzxNetwork::Type& RzxNetwork::type() const
{
	return m_type;
}


///Lance le chat avec l'ordinateur distant
/** Cette fonction est appelée dans différents cas :
 * - pour un MOD_CHATUI, lorsque l'utilisateur ou le protocole CHAT indique le début d'un chat
 * - pour un MOD_CHAT, lorsque l'utilisateur demande un chat
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxNetwork::chat(RzxComputer*)
{
}

///Lance la demande de propriétés de l'ordinateur distant
/** Cette fonction est appelée lorsque l'utilisateur demande que les propriétés
 * de l'ordinateur soient checkées et que MOD_PROPERTIES est déclaré
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxNetwork::properties(RzxComputer*)
{
}

///Envoie un message de chat à la machine indiquée
/** Cette fonction est émise lors de l'appelle à RzxComputer::sendChat. Et suppose que la
 * machine indiquée est bien sur le réseau actuel.
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxNetwork::sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

///Relance la connexion
/** L'implémentation par défaut fait un stop - start
 */
void RzxNetwork::restart()
{
	stop();
	start();
}

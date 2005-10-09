/***************************************************************************
                               rzxmodule.cpp
        Interface à implémenter pour développer un module pour qRezix
                             -------------------
    begin                : Sat Aug 6 2005
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
#include <QWidget>
#include <RzxModule>

///Construction du module à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
 */
RzxModule::RzxModule(const QString& name, const QString& description, int major, int minor, int build, const QString& tag)
	:RzxBaseModule(name, description, major, minor, build, tag)
{
}

///Construction d'un module
/** Fonction surchargée
 */
RzxModule::RzxModule(const QString& name, const QString& description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version)
{
}

///Construction d'un module à partir d'une chaîne décrivant son nom version
/** Cette fonction surchargée présente l'intérêt de permettre la construction d'un module simplemente à partir
 * de 2 chaînes de caractères. La première décrivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce même module.
 *
 * Le nom est donné de la forme :
 * "nom major.minor.build-tag"
 */
RzxModule::RzxModule(const QString& name, const QString& description)
	:RzxBaseModule(name, description)
{
}

///Destruction du module
RzxModule::~RzxModule()
{
}

///Ajout d'un flag aux type du module
void RzxModule::setType(const Type& flag)
{
	m_type |= flag;
}

///Retourne le type de module
const RzxModule::Type &RzxModule::type() const
{
	return m_type;
}

///Inverse l'état d'affichage de l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantToggleVisible.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::toggleVisible()
{
}

///Affichage de l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantShow.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::show()
{
}

///Cache l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantHide.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::hide()
{
}

///Retourne la fenêtre principale
/** Cette fonction permet de récupérer la fenêtre principale du module
 * son intérêt de de fournir une fenêtre principale pour le programme
 * pour tous les autres modules. Il est donc fortement conseillé de
 * réimplémenter cette fonction pour tous les modules de type \ref MOD_MAINUI.
 *
 * L'implémentation par défault renvoie NULL
 */
QWidget *RzxModule::mainWindow() const
{
	return NULL;
}


///Lance le chat avec l'ordinateur distant
/** Pour un MOD_CHATUI, lorsque l'utilisateur ou le protocole CHAT indique le début d'un chat
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::chat(RzxComputer*)
{
}

///Lance la demande de propriétés de l'ordinateur distant
/** Cette fonction est appelée lorsque l'utilisateur demande que les propriétés
 * de l'ordinateur soient checkées et que MOD_PROPERTIES est déclaré
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::properties(RzxComputer*)
{
}

///Lance l'affichage de l'historique des communication avec l'ordinateur distant
/** Cette fonction n'a besoin d'être réimplémentée que dans les MOD_CHATUI
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::history(RzxComputer*)
{
}

///Force l'affichage des propriétés par le module en question
/** Cette fonction est n'a besoin d'être réimplémentée que dans les MOD_PROPERTIESUI
 *
 * L'implémentation par défaut ne faire rien
 */
void RzxModule::showProperties(RzxComputer*)
{
}

void RzxModule::sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

void RzxModule::receiveChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

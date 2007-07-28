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
RzxModule::RzxModule(const QString& name, const char* description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version)
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
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::showProperties(RzxComputer*)
{
}

///Envoie un message à l'ordinateur distant
/** Ce message est identifié par la machine de destination, le type de message et son
 * contenu
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

///Réception d'un message provenant de l'ordinateur distant
/** Ce message est identifié par la machine qui l'a envoyé, le type de message et
 * son contenu
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::receiveChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

///Affichage d'un message via la Tray Icon
/** Ce message contient les informations à afficher (texte du message)
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::showTrayMessage(const QString&, const QString&)
{
}

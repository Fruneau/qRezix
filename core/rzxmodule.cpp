/***************************************************************************
                               rzxmodule.cpp
        Interface � impl�menter pour d�velopper un module pour qRezix
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

///Construction du module � partir des diff�rentes informations
/** Les informations � fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit �tre le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin � l'utilisateur
 * 	- num�ro de version major.minor.build-tag
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

///Inverse l'�tat d'affichage de l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantToggleVisible.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::toggleVisible()
{
}

///Affichage de l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantShow.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::show()
{
}

///Cache l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantHide.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::hide()
{
}

///Retourne la fen�tre principale
/** Cette fonction permet de r�cup�rer la fen�tre principale du module
 * son int�r�t de de fournir une fen�tre principale pour le programme
 * pour tous les autres modules. Il est donc fortement conseill� de
 * r�impl�menter cette fonction pour tous les modules de type \ref MOD_MAINUI.
 *
 * L'impl�mentation par d�fault renvoie NULL
 */
QWidget *RzxModule::mainWindow() const
{
	return NULL;
}


///Lance le chat avec l'ordinateur distant
/** Pour un MOD_CHATUI, lorsque l'utilisateur ou le protocole CHAT indique le d�but d'un chat
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::chat(RzxComputer*)
{
}

///Lance la demande de propri�t�s de l'ordinateur distant
/** Cette fonction est appel�e lorsque l'utilisateur demande que les propri�t�s
 * de l'ordinateur soient check�es et que MOD_PROPERTIES est d�clar�
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::properties(RzxComputer*)
{
}

///Lance l'affichage de l'historique des communication avec l'ordinateur distant
/** Cette fonction n'a besoin d'�tre r�impl�ment�e que dans les MOD_CHATUI
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::history(RzxComputer*)
{
}

///Force l'affichage des propri�t�s par le module en question
/** Cette fonction est n'a besoin d'�tre r�impl�ment�e que dans les MOD_PROPERTIESUI
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::showProperties(RzxComputer*)
{
}

///Envoie un message � l'ordinateur distant
/** Ce message est identifi� par la machine de destination, le type de message et son
 * contenu
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

///R�ception d'un message provenant de l'ordinateur distant
/** Ce message est identifi� par la machine qui l'a envoy�, le type de message et
 * son contenu
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::receiveChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString&)
{
}

///Affichage d'un message via la Tray Icon
/** Ce message contient les informations � afficher (texte du message)
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::showTrayMessage(const QString&, const QString&)
{
}

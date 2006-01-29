/***************************************************************************
                          rzxrezalaction  -  description
                               -------------------
    begin                : Sun Dec 26 2005
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
#include <RzxComputer>
#include <RzxApplication>
#include <RzxIconCollection>

#include "rzxrezalaction.h"
#include "rzxmainuiconfig.h"

///Recherche l'action effective associ�e � l'ordinateur donn�
/** On proc�de de la mani�re suivante :
 * 	- on teste si l'action par d�faut est possible...
 * 	- on parcours toutes les autres actions � la recherche d'une valide
 */
RzxRezalAction::Action RzxRezalAction::action(const RzxComputer *computer)
{
	if(!computer) return None;

	Action act = (Action)RzxMainUIConfig::doubleClicRole();
	do
	{
		//Lancement des actions en fonction de la demande
		switch(act)
		{
			case Chat:
				if(computer->canBeChatted())
					return Chat;

			case Ftp:
				if(computer->hasFtpServer())
					return Ftp;

			case Http:
				if(computer->hasHttpServer())
					return Http;

			case News:
				if(computer->hasNewsServer())
					return News;

			case Samba:
				if(computer->hasSambaServer())
					return Samba;

			case Mail:
				if(computer->hasEmailAddress())
					return Mail;

			case Prop:
				if(computer->canBeChecked())
					return Prop;

			case History:
				if(RzxApplication::chatUiModule())
					return History;

			default:
				if(act == Chat) return None;
				act = Chat;
		}
		//On relance un unique deuxi�me tour en partant de Chat pour �tre s�r
		//de tester toutes les actions possibles
	}
	while(true);
}


///Lance l'action par d�faut associ�e au double clic
void RzxRezalAction::run(RzxComputer *computer)
{
	if(!computer) return;

	//Lancement des actions en fonction de la demande
	switch(action(computer))
	{
		case Chat:
			computer->chat();
			return;

		case Ftp:
			computer->ftp();
			return;

		case Http:
			computer->http();
			return;

		case News:
			computer->news();
			return;

		case Samba:
			computer->samba();
			return;

		case Mail:
			computer->mail();
			return;

		case Prop:
			computer->checkProperties();

		case History:
			computer->history();

		default: //pour �viter les warnings de compilation
			return;
	}
}

///Lance l'action demand�e sur l'ordinateur en question
void RzxRezalAction::run(Action action, RzxComputer *computer)
{
	if(!computer) return;
	switch(action)
	{
		case Chat:
			if(computer->canBeChatted())
				computer->chat();
			return;

		case Ftp:
			if(computer->hasFtpServer())
				computer->ftp();
			return;

		case Http:
			if(computer->hasHttpServer())
				computer->http();
			return;

		case News:
			if(computer->hasNewsServer())
				computer->news();
			return;

		case Samba:
			if(computer->hasSambaServer())
				computer->samba();
			return;

		case Mail:
			if(computer->hasEmailAddress())
				computer->mail();
			return;

		case Prop:
			if(computer->canBeChecked())
				computer->checkProperties();
			return;

		case History:
			if(RzxApplication::chatUiModule())
				computer->history();
			return;

		default: //pour �viter les warnings de compilation
			return;
	}
}

///Construit un curseur pour l'action par d�faut
/** C'est � dire un curseur repr�sentant l'action r�alisable actuellement
 * o� une fl�che normale si on peut rien faire...
 */
QCursor RzxRezalAction::cursor(const RzxComputer *computer)
{
	return cursor(action(computer), computer);
}

///Construit un curseur pour l'action donn�e...
/** Si un ordinateur est sp�cifi�, le curseau n'est construit que si l'action
 * en question est applicable � l'ordinateur
 */
QCursor RzxRezalAction::cursor(Action action, const RzxComputer *computer, bool force)
{
	switch(action)
	{
		case Chat:
			if(!computer || computer->canBeChatted())
				return RzxIconCollection::getPixmap(Rzx::ICON_CHAT);
			break;

		case Ftp:
			if(!computer || computer->hasFtpServer())
				return RzxIconCollection::getPixmap(Rzx::ICON_FTP);
			break;

		case Http:
			if(!computer || computer->hasHttpServer())
				return RzxIconCollection::getPixmap(Rzx::ICON_HTTP);
			break;

		case News:
			if(!computer || computer->hasNewsServer())
				return RzxIconCollection::getPixmap(Rzx::ICON_NEWS);
			break;

		case Samba:
			if(!computer || computer->hasSambaServer())
				return RzxIconCollection::getPixmap(Rzx::ICON_SAMBA);
			break;

		case Mail:
			if(!computer || computer->hasEmailAddress())
				return RzxIconCollection::getPixmap(Rzx::ICON_MAIL);
			break;

		case Prop:
			if(!computer || computer->canBeChecked())
				return RzxIconCollection::getPixmap(Rzx::ICON_PROPRIETES);
			break;

		case History:
			if(RzxApplication::chatUiModule())
				return RzxIconCollection::getPixmap(Rzx::ICON_HISTORIQUE);

		default:
			return QCursor();
	}

	if(force && computer)
		return cursor(computer);
	return QCursor();
}

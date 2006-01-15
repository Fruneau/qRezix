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

#include "rzxrezalaction.h"
#include "rzxmainuiconfig.h"

///Lance l'action par défaut associée au double clic
/** On procède de la manière suivante :
 * 	- on teste si l'action par défaut est possible...
 * 	- on parcours toutes les autres actions à la recherche d'une valide
 */
void RzxRezalAction::run(RzxComputer *computer)
{
	if(!computer) return;

	RzxMainUIConfig::DoubleClicAction action = (RzxMainUIConfig::DoubleClicAction)RzxMainUIConfig::doubleClicRole();
	do
	{
		//Lancement des actions en fonction de la demande
		switch(RzxMainUIConfig::doubleClicRole())
		{
			case RzxMainUIConfig::Chat:
				if(computer->canBeChatted())
				{
					computer->chat();
					return;
				}

			case RzxMainUIConfig::Ftp:
				if(computer->hasFtpServer())
				{
					computer->ftp();
					return;
				}

			case RzxMainUIConfig::Http:
				if(computer->hasHttpServer())
				{
					computer->http();
					return;
				}

			case RzxMainUIConfig::News:
				if(computer->hasNewsServer())
				{
					computer->news();
					return;
				}

			case RzxMainUIConfig::Samba:
				if(computer->hasSambaServer())
				{
					computer->samba();
					return;
				}

			case RzxMainUIConfig::Mail:
				if(computer->hasEmailAddress())
				{
					computer->mail();
					return;
				}
		}
		//On relance un unique deuxième tour en partant de Chat pour être sûr
		//de tester toutes les actions possibles
		if(action == RzxMainUIConfig::Chat) return;
		action = RzxMainUIConfig::Chat;
	}
	while(true);
}

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
#ifndef RZXREZALACTION_H
#define RZXREZALACTION_H

#include <QCursor>

#include "rzxmainuiglobal.h"

/**
 @author Florent Bruneau
 */

///Permet de lancer des actions sur les ordinateurs
namespace RzxRezalAction
{
	///Enumère les actions pour le double clic
	enum Action
	{
		None = -1,
		Chat = 0,
		Ftp = 1,
		Http = 2,
		News = 3,
		Samba = 4,
		Mail = 5
	};

	///Recherche l'action effective actuelle
	RZX_MAINUI_EXPORT Action action(const RzxComputer *);

	///Lance l'action par défaut associée au double clic
	RZX_MAINUI_EXPORT void run(RzxComputer *);

	///Lance l'action demandée sur l'ordinateur
	RZX_MAINUI_EXPORT void run(Action, RzxComputer *);

	///Retourne un curseur pour l'action par défaut
	RZX_MAINUI_EXPORT QCursor cursor(const RzxComputer *);

	///Retourne un curseur pour l'action donnée
	RZX_MAINUI_EXPORT QCursor cursor(Action, const RzxComputer * = NULL, bool force = false);
}

#endif

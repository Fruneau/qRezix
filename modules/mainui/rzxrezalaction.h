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

#include "rzxmainuiglobal.h"

/**
 @author Florent Bruneau
 */

///Permet de lancer des actions sur les ordinateurs
namespace RzxRezalAction
{
	///Lance l'action par défaut associée au double clic
	RZX_MAINUI_EXPORT void run(RzxComputer *);
}

#endif

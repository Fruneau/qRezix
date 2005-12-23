/***************************************************************************
                     rzxrezalviewconfig.h  -  description
                             -------------------
    begin                : Fri Dec 23 2005
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
#ifndef RZXREZALVIEWCONFIG_H
#define RZXREZALVIEWCONFIG_H

#include <RzxAbstractConfig>

/**
 @author Florent Bruneau
 */

///Stockage des données de configuration du module.
class RzxRezalViewConfig: public RzxAbstractConfig
{
	RZX_CONFIG(RzxRezalViewConfig)

	public:
		RZX_UINTPROP("colonnes", colonnes, setColonnes, 0x37b)
		RZX_LISTPROP(int, "columnpositions", columnPositions, setColumnPositions)
};

#endif

/***************************************************************************
           rzxfavoritelist  -  gestion d'une liste de favoris
                             -------------------
    begin                : Wed Dec 14 2005
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
#ifndef RZXFAVORITELIST_H
#define RZXFAVORITELIST_H

#include <RzxComputerList>

/**
 @author Florent Bruneau
 */

///Liste des favoris...
class RZX_CORE_EXPORT RzxFavoriteList: public RzxComputerList
{
	RZX_GLOBAL(RzxFavoriteList)

	public:
		RzxFavoriteList();
		~RzxFavoriteList();
};

#endif

/***************************************************************************
               rzxbanlist  -  gestion d'une liste d'ignorance
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
#include <RzxConfig>

#include <RzxBanList>

RZX_GLOBAL_INIT(RzxBanList);

///Construction
RzxBanList::RzxBanList()
	:RzxComputerList(Address, RzxConfig::global(), "ignoreList")
{
}

///Destruction
RzxBanList::~RzxBanList()
{
	RZX_GLOBAL_CLOSE
}

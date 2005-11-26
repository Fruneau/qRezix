/***************************************************************************
                          rzxmainuiglobal  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXMAINUIGLOBAL_H
#define RZXMAINUIGLOBAL_H

#include <QtGlobal>

#ifndef RZX_MAINUI_EXPORT
#	ifdef RZX_BUILD_MAINUI
#		define RZX_MAINUI_EXPORT Q_DECL_EXPORT
#	else
#		define RZX_MAINUI_EXPORT
#	endif
#endif

#endif

/***************************************************************************
                      rzxrezaldetailconfig  -  description
                             -------------------
    begin                : Wed Jan 11 2006
    copyright            : (C) 2006 Florent Bruneau
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
#ifndef RZXREZALDETAILCONFIG_H
#define RZXREZALDETAILCONFIG_H

#include <RzxAbstractConfig>

/**
 @author Florent Bruneau
 */

///Propriétés de la fenêtre de détails
class RzxRezalDetailConfig: public RzxAbstractConfig
{
	Q_OBJECT
	RZX_CONFIG(RzxRezalDetailConfig)

	public:
		RZX_INTPROP("detailSize", detailSize, setDetailSize, 150)
		RZX_INTPROP("propsSize", propsSize, setPropsSize, 150)
};

#endif

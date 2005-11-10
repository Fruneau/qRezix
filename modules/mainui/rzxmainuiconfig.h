/***************************************************************************
                     rzxmainuiconfig.h  -  description
                             -------------------
    begin                : Fri Aug 12 2005
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
#ifndef RZXMAINUICONFIG_H
#define RZXMAINUICONFIG_H

#include <RzxAbstractConfig>

#define DEFAULT_WINDOWSIZE "100000000"

/**
 @author Florent Bruneau
 */

///Stockage des données de configuration du module.
class RzxMainUIConfig: public RzxAbstractConfig
{
	RZX_CONFIG(RzxMainUIConfig)

	public:
		/** Renvoie la taille des icones des utilsiateurs
		 * 0 pour 32x32
		 * 1 pour 64x64
		 */
		RZX_INTPROP("iconsize", computerIconSize, setComputerIconSize, 0)
		RZX_INTPROP("doubleClic", doubleClicRole, setDoubleClicRole, 0)
		RZX_INTPROP("defaultTab", defaultTab, setDefaultTab, 0)
		RZX_BOOLPROP("iconHighlight", computerIconHighlight, setComputerIconHighlight, true)
		RZX_BOOLPROP("useSearch", useSearch, setUseSearch, true)
		RZX_BOOLPROP("showQuit", showQuit, setShowQuit, true)
		RZX_INTPROP("quitmode", quitMode, setQuitMode, 1)
		RZX_UINTPROP("colonnes", colonnes, setColonnes, 0x37b)
		RZX_UINTPROP("tooltip", tooltip, setTooltip, 0)
		RZX_ENUMPROP(Qt::SortOrder, "sortOrder", sortOrder, setSortOrder, Qt::AscendingOrder)
		RZX_INTPROP("sortColumn", sortColumn, setSortColumn, 0)
		RZX_LISTPROP(int, "columnpositions", columnPositions, setColumnPositions)
	
		RZX_WIDGETPROP("main", restoreMainWidget, saveMainWidget, QPoint(2,24), QSize(400, 300))
};

#endif

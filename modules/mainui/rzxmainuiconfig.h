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
#include "rzxmainuiglobal.h"
#include "rzxrezalaction.h"
#include "rzxrezalsearch.h"

#define DEFAULT_WINDOWSIZE "100000000"
#define DEFAULT_REZAL "List"

/**
 @author Florent Bruneau
 */

///Stockage des donn�es de configuration du module.
class RZX_MAINUI_EXPORT RzxMainUIConfig: public RzxAbstractConfig
{
	RZX_CONFIG(RzxMainUIConfig)

	public:
		///Enum�re les tabs accessibles pour l'ouverture
		enum Tab
		{
			Favorites = 0,
			Promo = 1,
			Subnet = 2,
			Everybody = 3,
			FirstGroup = 4
		};

		RZX_INTPROP("doubleClic", doubleClicRole, setDoubleClicRole, RzxRezalAction::Chat)
		RZX_INTPROP("defaultTab", defaultTab, setDefaultTab, Favorites)
		RZX_STRINGPROP("defaultTabGroup", defaultTabGroup, setDefaultTabGroup, QString());
		RZX_BOOLPROP("useSearch", useSearch, setUseSearch, true)
		RZX_BOOLPROP("showQuit", showQuit, setShowQuit, true)
		RZX_INTPROP("quitmode", quitMode, setQuitMode, 1)
		RZX_UINTPROP("tooltip", tooltip, setTooltip, 0)
		RZX_ENUMPROP(Qt::SortOrder, "sortOrder", sortOrder, setSortOrder, Qt::AscendingOrder)
		RZX_INTPROP("sortColumn", sortColumn, setSortColumn, 0)
		RZX_STRINGPROP("centralrezal", centralRezal, setCentralRezal, DEFAULT_REZAL)
		RZX_ENUMPROP(RzxRezalSearch::Mode, "searchMode", searchMode, setSearchMode, RzxRezalSearch::Lite)

		RZX_INTPROP("topLeftCorner", topLeftCorner, setTopLeftCorner, Qt::LeftDockWidgetArea)
		RZX_INTPROP("bottomLeftCorner", bottomLeftCorner, setBottomLeftCorner, Qt::LeftDockWidgetArea)
		RZX_INTPROP("bottomRightCorner", bottomRightCorner, setBottomRightCorner, Qt::RightDockWidgetArea)
		RZX_INTPROP("topRightCorner", topRightCorner, setTopRightCorner, Qt::RightDockWidgetArea)

		RZX_WIDGETPROP("main", restoreMainWidget, saveMainWidget, QPoint(2,24), QSize(400, 300))
		RZX_PROP(QByteArray, "mainuiState", restoreState, saveState, QByteArray())

};

#endif

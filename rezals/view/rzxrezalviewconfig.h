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
#include <RzxRezalModel>

/**
 @author Florent Bruneau
 */

///Stockage des données de configuration du module.
class RzxRezalViewConfig: public RzxAbstractConfig
{
	RZX_CONFIG(RzxRezalViewConfig)

	public:
		RZX_UINTPROP("colonnes", colonnes, setColonnes, RzxRezalModel::ColNomFlag | RzxRezalModel::ColRemarqueFlag |
			RzxRezalModel::ColFTPFlag | RzxRezalModel::ColHTTPFlag | RzxRezalModel::ColNewsFlag | RzxRezalModel::ColPrinterFlag |
			RzxRezalModel::ColPromoFlag | RzxRezalModel::ColRezalFlag)

		RZX_LISTPROP(int, "columnpositions", columnPositions, setColumnPositions)
};

#endif

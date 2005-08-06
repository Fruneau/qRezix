/***************************************************************************
                          rzxnotifier  -  description
                             -------------------
    begin                : Sun Jul 31 2005
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
#ifndef RZXNOTIFIER_H
#define RZXNOTIFIER_H

#include <QObject>

#include "../core/rzxmodule.h"

class RzxComputer;

/**
@author Florent Bruneau
*/

///Notification de connexion des favoris
class RzxNotifier : public RzxModule
{
	Q_OBJECT

	bool favoriteWarn;

	public:
		RzxNotifier();
		~RzxNotifier();
		virtual bool isInitialised() const;

	public slots:
		void favoriteUpdated(RzxComputer *);
		void login(RzxComputer *);
		void loginEnd();
};

#endif

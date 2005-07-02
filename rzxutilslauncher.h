/***************************************************************************
                          rzxutilslauncher.h  -  description
                             -------------------
    begin                : Fri Sep 10 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXUTILSLAUNCHER_H
#define RZXUTILSLAUNCHER_H

#include <QString>

#include "rzxconnectionlister.h"

/**
  *@author Florent Bruneau
  */
  
class RzxConnectionLister;

///Lanceur d'application externes
/** Cette classe à pour but de lancer les applications externes liées à qRezix, comme le client ftp, le client http... */
class RzxUtilsLauncher
{
	static RzxUtilsLauncher *object;
	RzxConnectionLister *lister;

	public:
		RzxUtilsLauncher();
		~RzxUtilsLauncher();
		
		static RzxUtilsLauncher *global();
	
		void ftp(const QString& login) const;
		void http(const QString& login) const;
		void news(const QString& login) const;
		void samba(const QString& login) const;
};

inline RzxUtilsLauncher *RzxUtilsLauncher::global()
{
	if(!object)
		object = new RzxUtilsLauncher();
	return object;
}

#endif

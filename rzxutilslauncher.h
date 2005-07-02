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

/**
  *@author Florent Bruneau
  */
  
class RzxRezal;

///Lanceur d'application externes
/** Cette classe à pour but de lancer les applications externes liées à qRezix, comme le client ftp, le client http... */
class RzxUtilsLauncher
{
	static RzxUtilsLauncher *object;
	RzxRezal *rezal;

	public:
		RzxUtilsLauncher(RzxRezal *m_rezal);
		~RzxUtilsLauncher();
		
		inline static RzxUtilsLauncher *global() { return object; };
	
		static void ftp(const QString& login);
		static void http(const QString& login);
		static void news(const QString& login);
		static void samba(const QString& login);
};

#endif

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

class RzxHostAddress;

/**
 @author Florent Bruneau
 */

///Lanceur d'application externes
/** Cette classe à pour but de lancer les applications externes liées à qRezix, comme le client ftp, le client http... */
namespace RzxUtilsLauncher
{
	void ftp(const RzxHostAddress&, const QString& path = QString());
	void http(const RzxHostAddress&, const QString& path = QString());
	void news(const RzxHostAddress&, const QString& path = QString());
	void samba(const RzxHostAddress&, const QString& path = QString());
};

#endif

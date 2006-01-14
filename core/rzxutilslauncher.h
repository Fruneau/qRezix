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

#include <QStringList>

#include <RzxGlobal>

class RzxHostAddress;
class QProcess;
class QUrl;

/**
 @author Florent Bruneau
 */

///Lanceur d'application externes
/** Cette classe à pour but de lancer les applications externes liées à qRezix,
 * comme le client ftp, le client http...
 */
namespace RzxUtilsLauncher
{
	RZX_CORE_EXPORT void run(const QUrl&);
	RZX_CORE_EXPORT void ftp(const RzxHostAddress&, const QString& path = QString());
	RZX_CORE_EXPORT void ftp(const QString&, const QString& path = QString());
	RZX_CORE_EXPORT void http(const RzxHostAddress&, const QString& path = QString());
	RZX_CORE_EXPORT void http(const QString&, const QString& path = QString());
	RZX_CORE_EXPORT void news(const RzxHostAddress&, const QString& path = QString());
	RZX_CORE_EXPORT void news(const QString&, const QString& path = QString());
	RZX_CORE_EXPORT void samba(const RzxHostAddress&, const QString& path = QString());
	RZX_CORE_EXPORT void samba(const QString&, const QString& path = QString());
	RZX_CORE_EXPORT void mail(const QString&);
	RZX_CORE_EXPORT QProcess *launchCommand(const QString&, const QStringList& = QStringList(), const QString& path = QString());
};

#endif

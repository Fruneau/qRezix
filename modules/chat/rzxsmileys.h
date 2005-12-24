/***************************************************************************
                          rzxsmiley -  description
                             -------------------
    begin                : Sat Dec 24 2005
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
#ifndef RZXSMILEYS_H
#define RZXSMILEYS_H

#include <QHash>
#include <QPixmap>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

class QDir;

///Gestion des smileys...
/** Cette classe a pour but de centraliser la gestion des smileys
 */
class RzxSmileys: public QObject
{
	Q_OBJECT
	RZX_GLOBAL(RzxSmileys)

	QHash<QString, QDir*> smileyDir;
	QHash<QString, QString> smileys;
	QStringList baseSmileys;
	void loadSmileysList();
	void loadSmileys();

	RzxSmileys();

	public:
		~RzxSmileys();
		static void setTheme(const QString&);
		static QString theme();

		static QStringList themeList();
		static QStringList baseSmileyList();

		static QString smiley(const QString&);
		static QPixmap pixmap(const QString&);
		static void replace(QString &);
};

#endif

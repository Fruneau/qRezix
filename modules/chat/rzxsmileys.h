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

	///Données décrivant un thème de smileys
	struct SmileyTheme
	{
		///Association masque - url de l'image
		QHash<QString, QString> smileys;
		///Liste des smileys disponibles
		QStringList baseSmileys;
	};

	QHash<QString, SmileyTheme*> themes;
	SmileyTheme *currentTheme;

	void loadSmileysList();
	void loadSmileys(SmileyTheme *, QDir*);

	SmileyTheme *smileyTheme(const QString& theme);

	RzxSmileys();

	public:
		~RzxSmileys();
		static void setTheme(const QString&);
		static QString theme();

		static QStringList themeList();
		static QStringList baseSmileyList(const QString& thm = QString());

		static QString smiley(const QString&, const QString& thm = QString());
		static QPixmap pixmap(const QString&, const QString& thm = QString());
		static void replace(QString &, const QString& thm = QString());

		static bool isValid(const QString& thm = QString());

		static bool connect(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoCompatConnection);
		static bool disconnect(const QObject * receiver);

		static QList<QPixmap> preview(const QString& thm = QString());

	signals:
		///Ce signal est émis lorsque le theme change
		/** \sa connect, disconnect */
		void themeChanged(const QString&);
};

///Retourne un thème correspondant à celui indiqué ou NULL
/** Si theme est vide, retourne le thème courant
 */
inline RzxSmileys::SmileyTheme *RzxSmileys::smileyTheme(const QString& theme)
{
	if(theme.isEmpty())
		return currentTheme;

	if(!themes.keys().contains(theme))
		return NULL;

	return themes[theme];
}

///Connexion pour le changement de traduction
inline bool RzxSmileys::connect(const QObject *receiver, const char *method, Qt::ConnectionType type)
{
	return QObject::connect(global(), SIGNAL(themeChanged(const QString&)), receiver, method, type);
}

///Déconnecte un objet du message de changement de traduction
inline bool RzxSmileys::disconnect(const QObject *receiver)
{
	return global()->QObject::disconnect(SIGNAL(themeChanged(const QString&)), receiver);
}

#endif

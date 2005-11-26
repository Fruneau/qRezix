/***************************************************************************
                          rzxtranslator  -  description
                             -------------------
    begin                : Sat Nov 5 2005
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
#ifndef RZXTRANSLATOR_H
#define RZXTRANSLATOR_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QList>

#include <RzxGlobal>

class QDir;
class QTranslator;

/**
 @author Florent Bruneau
 */

///Gestion des traductions
/** Cette classe regroupe les fonctions qui ont un lien avec les traductions. En particulier :
 * 	- chargement des traductions
 * 	- changement de traduction courrante
 *
 * La plupart des classes n'ont juste besoin que de connaître connect et disconnect qui permettent
 * de simplifier la connexion au languageChanged.
 */
class RZX_CORE_EXPORT RzxTranslator:public QObject
{
	Q_OBJECT
	RZX_GLOBAL(RzxTranslator)

	QHash<QString, QString> languageNames;
	QHash<QString, QList<QTranslator*> > translations;
	QString lang;

	public:
		RzxTranslator();
		~RzxTranslator();

	private:
		void loadTranslators();
		void loadTranslatorsInDir(const QDir &rep);

	public:
		static void setLanguage(const QString&);
		static QString language();
		static QStringList translationsList();
		static QString translation();

		static bool connect(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoCompatConnection);
		static bool disconnect(const QObject * receiver);

	signals:
		///Indique que le language actuel a changé
		void languageChanged(const QString&);
};

///Connexion pour le changement de traduction
inline bool RzxTranslator::connect(const QObject *receiver, const char *method, Qt::ConnectionType type)
{
	return QObject::connect(global(), SIGNAL(languageChanged(const QString&)), receiver, method, type);
}

///Déconnecte un objet du message de changement de traduction
inline bool RzxTranslator::disconnect(const QObject *receiver)
{
	return global()->QObject::disconnect(SIGNAL(languageChanged(const QString&)), receiver);
}

#endif

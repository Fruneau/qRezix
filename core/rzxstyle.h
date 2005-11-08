/***************************************************************************
                          rzxstyle  -  description
                             -------------------
    begin                : Sun Nov 6 2005
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
#ifndef RZXSTYLE_H
#define RZXSTYLE_H

#include <QStyleFactory>
#include <QObject>
#include <QList>
#include <QPointer>
#include <QWidget>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

///Gestion des styles
/** Cette classe g�re les th�mes appliqu�s aux fen�tres.
 * qRezix permet de choisir entre un application globale des th�mes ou juste fen�tre par fen�tre.
 * Dans ce deuxi�me cas, seules les fen�tres qui ont �t� d�clar�es comme 'skinnable'
 * via \ref RzxStyle::useStyleOnWindows seront mise � jour lors des changements de th�me.
 *
 * La classe utilise �galementune API proche de \ref RzxTranslator qui permet de connecter simplement
 * un objet � l'envoie du message styleChanged. Mais a priori ces fonctions sont peu utilses.
 */
class RzxStyle:public QObject
{
	Q_OBJECT
	RZX_GLOBAL(RzxStyle)

	QStringList styles;
	QString currentName;

	QList< QPointer<QWidget> > styledWidgets;

	QStyle *current() const;

	public:
		RzxStyle();
		~RzxStyle();

		static QStringList styleList();
		static void setStyle(const QString&);
		static QString style();
		static void useStyleOnWindow(QWidget*);
		static void freeStyleOnWindow(QWidget*);

		static bool connect(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoCompatConnection);
		static bool disconnect(const QObject * receiver);

	protected:
		void applyStyle();
		void applyStyle(QWidget *);

	signals:
		void styleChanged(const QString&);
};


///G�n�re un objet du style apropri�
inline QStyle *RzxStyle::current() const
{
	if(currentName == "default" || currentName == "Mac Metal")
		return NULL;
	else
		return QStyleFactory::create(currentName);
}

///Connexion pour le changement de traduction
inline bool RzxStyle::connect(const QObject *receiver, const char *method, Qt::ConnectionType type)
{
	return QObject::connect(global(), SIGNAL(styleChanged(const QString&)), receiver, method, type);
}

///D�connecte un objet du message de changement de traduction
inline bool RzxStyle::disconnect(const QObject *receiver)
{
	return global()->QObject::disconnect(SIGNAL(styleChanged(const QString&)), receiver);
}

#endif

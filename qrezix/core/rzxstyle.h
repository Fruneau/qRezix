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

#include <QObject>
#include <QList>
#include <QPointer>
#include <QWidget>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

///Gestion des styles
class RzxStyle:public QObject
{
	Q_OBJECT
	RZX_GLOBAL(RzxStyle)

	QStringList styles;
	QString currentName;
	QStyle *current;

	QList< QPointer<QWidget> > styledWidgets;

	public:
		RzxStyle();
		~RzxStyle();

		static QStringList styleList();
		static void setStyle(const QString&);
		static QString style();
		static void useStyleOnWindow(QWidget*);

		static bool connect(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoCompatConnection);
		static bool disconnect(const QObject * receiver);

	protected:
		void applyStyle();
		void applyStyle(QWidget *);

	signals:
		void styleChanged(const QString&);
};

///Connexion pour le changement de traduction
inline bool RzxStyle::connect(const QObject *receiver, const char *method, Qt::ConnectionType type)
{
	return QObject::connect(global(), SIGNAL(styleChanged(const QString&)), receiver, method, type);
}

///Déconnecte un objet du message de changement de traduction
inline bool RzxStyle::disconnect(const QObject *receiver)
{
	return global()->QObject::disconnect(SIGNAL(styleChanged(const QString&)), receiver);
}

#endif

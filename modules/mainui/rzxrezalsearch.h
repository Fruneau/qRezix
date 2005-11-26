/***************************************************************************
                          rzxrezalsearch  -  description
                             -------------------
    begin                : Mon Jul 25 2005
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
#ifndef RZXREZALSEARCH_H
#define RZXREZALSEARCH_H

#include <QObject>
#include <QString>
#include <QTime>

#include "rzxmainuiglobal.h"

class QAbstractItemView;
class RzxRezalModel;
class QModelIndex;

/**
@author Florent Bruneau
*/

///Classe fournissant une API simple pour permettre des recherches
class RZX_MAINUI_EXPORT RzxRezalSearch:public QObject
{
	Q_OBJECT
	Q_PROPERTY(QAbstractItemView* view READ view)
	Q_PROPERTY(RzxRezalModel* model READ model)
	Q_PROPERTY(QString pattern READ pattern WRITE setPattern RESET resetPattern);
	Q_PROPERTY(int timeout READ timeout WRITE setTimeout)

	QString searchPattern;
	QTime searchTimeout;
	int timeLimit;

	public:
		RzxRezalSearch(QAbstractItemView *, int timeout = 5000, bool connected = true);
		~RzxRezalSearch();

		QAbstractItemView *view() const;
		RzxRezalModel *model() const;
		const QString &pattern() const;
		int timeout() const;

	public slots:
		void setPattern(const QString&);
		void addToPattern(const QString&);
		void reducePattern(int size = 1);
		void resetPattern();
		void setTimeout(int);

	protected:
		void testTimeout();

	signals:
		void searchPatternChanged(const QString&);
		void findItem(const QModelIndex&);
};

#endif

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
#include <QAbstractItemView>

#include "rzxrezalsearch.h"

#include "rzxrezalmodel.h"

///Construction
RzxRezalSearch::RzxRezalSearch(QAbstractItemView *view, int timeout)
	:QObject(view), timeLimit(timeout)
{ }

///Destruction... rien à faire
RzxRezalSearch::~RzxRezalSearch()
{ }

QAbstractItemView *RzxRezalSearch::view() const
{ return qobject_cast<QAbstractItemView*>(parent()); }

RzxRezalModel *RzxRezalSearch::model() const
{ return qobject_cast<RzxRezalModel*>(view()->model()); }

const QString &RzxRezalSearch::pattern() const
{ return searchPattern; }

int RzxRezalSearch::timeout() const
{ return timeLimit; }

///Met à jour le filtre
void RzxRezalSearch::setPattern(const QString& pattern)
{
	if(pattern == searchPattern) return;
	searchPattern = pattern;
	emit searchPatternChanged(searchPattern);
	searchTimeout.start();

	RzxComputer **item;
	const RzxDict<QString, RzxComputer*> *itemByName = model()->childrenByName(view()->rootIndex());
	QString lower, higher;
	if(!itemByName->find_nearest(searchPattern, lower, higher ))
	{
		bool lmatch, hmatch;
		lmatch = lower.left(searchPattern.length() ) == searchPattern;
		hmatch = higher.left( searchPattern.length() ) == searchPattern;
		if ( ( !lmatch ) && ( !hmatch ) )
		{
			int i;
			for ( i = 0, lmatch = true, hmatch = true; lmatch && hmatch && ( i < searchPattern.length() ); i++ )
			{
				lmatch = lower[i] == searchPattern[i];
				hmatch = higher[i] == searchPattern[i];
			}
			i--;
			if ( (!hmatch) && (!lmatch) )
			{
				char c = searchPattern[i].toLatin1(),
					lc = lower[i].toLatin1(),
					hc = higher[i].toLatin1();
				if ( qAbs( c - hc ) < qAbs( c - lc ) )
					hmatch = true;
			}
			searchPattern = searchPattern.left( searchPattern.length() - 1 );
		}
		if ( hmatch && ( !lmatch ) )
			lower = higher;
	}
	itemByName->find(lower, item);
	emit findItem(model()->index(*item, view()->rootIndex()));
}

void RzxRezalSearch::resetPattern()
{
	setPattern(QString());
}

void RzxRezalSearch::addToPattern(const QString& pattern)
{
	testTimeout();
	setPattern(searchPattern + pattern);
}

void RzxRezalSearch::reducePattern(int size)
{
	testTimeout();
	int length = searchPattern.length() - size;
	if(length <= 0)
		resetPattern();
	else
		setPattern(searchPattern.left(length));
}

void RzxRezalSearch::setTimeout(int time)
{
	timeLimit = time;
}

void RzxRezalSearch::testTimeout()
{
	if(searchTimeout.elapsed() > 5000)
	{
		searchPattern = QString();
		emit searchPatternChanged(searchPattern);
	}
}

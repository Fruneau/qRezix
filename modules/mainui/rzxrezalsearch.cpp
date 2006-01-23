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
#include <RzxComputer>

#include "rzxrezalsearch.h"

#include "rzxrezalmodel.h"
#include "qrezix.h"

///Construction
RzxRezalSearch::RzxRezalSearch(QAbstractItemView *view, int timeout, bool connected)
	:QObject(view), timeLimit(timeout)
{
	if(connected)
	{
		connect(this, SIGNAL(searchPatternChanged(const QString&)), QRezix::global(), SLOT(setSearchPattern(const QString&)));
		connect(QRezix::global(), SIGNAL(searchPatternChanged(const QString&)), this, SLOT(setPattern(const QString&)));
	}
}

///Destruction... rien à faire
RzxRezalSearch::~RzxRezalSearch()
{ }

///Retourne le QAbstractItemView auquel est rattaché l'objet actuel
QAbstractItemView *RzxRezalSearch::view() const
{ return qobject_cast<QAbstractItemView*>(parent()); }

///Retourne le RzxRezalModel
/** Ceci implique donc que le QAbstractItemView doit utiliser un RzxRezalModel
 * comme modèle...
 */
RzxRezalModel *RzxRezalSearch::model() const
{ return qobject_cast<RzxRezalModel*>(view()->model()); }

///Retourne le filtre actuel
const QString &RzxRezalSearch::pattern() const
{ return searchPattern; }

///Retourne le temps de timeout du filtre
/** Un filtre de recherche timeout automatiquement au bout d'un certain temps...
 * Ceci évite entre autre de continuer une rechercher oubliée au lieu d'en recommencer
 * une nouvelle 
 */
int RzxRezalSearch::timeout() const
{ return timeLimit; }

///Met à jour le filtre
/** La mise à jour s'accompagne de la recherche de l'élément le plus proche dans
 * le RzxDict des objets
 */
void RzxRezalSearch::setPattern(const QString& pattern)
{
	if(pattern == searchPattern) return;
	searchPattern = pattern;
	searchTimeout.start();

	//Implémentation de la recherche par Ey pour qRezix 1.6.2
	// le but étant d'utiliser un arbre binaire de recherche pour avoir
	// une recherche vraiment rapide
	const RzxRezalSearchTree *itemByName = model()->childrenByName(view()->rootIndex());
	if(!itemByName) return;

	QPointer<RzxComputer> *item = NULL;
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
	emit searchPatternChanged(searchPattern);
	if(item && !searchPattern.isEmpty())
		emit findItem(model()->index(*item, view()->rootIndex()));
}

///Réinitialise le filtre de la rechercher
void RzxRezalSearch::resetPattern()
{
	setPattern(QString());
}

///Complète le filtre en ajoutant
void RzxRezalSearch::addToPattern(const QString& pattern)
{
	testTimeout();
	setPattern(searchPattern + pattern);
}

///Réduit le filtre en supprimant ses derniers caractères
void RzxRezalSearch::reducePattern(int size)
{
	testTimeout();
	int length = searchPattern.length() - size;
	if(length <= 0)
		resetPattern();
	else
		setPattern(searchPattern.left(length));
}

///Défini le temps de timeout
void RzxRezalSearch::setTimeout(int time)
{
	timeLimit = time;
}

///Fait timeouter le filtre
/** Test si le filtre est trop ancien, et le fait timeouté
 * si nécessaire
 */
void RzxRezalSearch::testTimeout()
{
	if(searchTimeout.elapsed() > timeLimit)
	{
		searchPattern = QString();
		emit searchPatternChanged(searchPattern);
	}
}

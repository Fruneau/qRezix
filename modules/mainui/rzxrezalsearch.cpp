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
#include <RzxComputer>
#include <RzxConnectionLister>

#include "rzxrezalsearch.h"

#include "rzxrezalmodel.h"
#include "rzxmainuiconfig.h"
#include "qrezix.h"

///Construction
RzxRezalSearch::RzxRezalSearch(QAbstractItemView *view, int timeout, bool connected)
	:QObject(view), timeLimit(timeout), searchMode(Config)
{
	if(connected)
	{
		connect(this, SIGNAL(searchPatternChanged(const QString&)), QRezix::global(), SLOT(setSearchPattern(const QString&)));
		connect(QRezix::global(), SIGNAL(searchPatternChanged(const QString&)), this, SLOT(setPattern(const QString&)));
		connect(QRezix::global(), SIGNAL(searchModeChanged(RzxRezalSearch::Mode)), this, SLOT(setMode(RzxRezalSearch::Mode)));
	}
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer*)),
			this, SLOT(filterView()));
}

///Destruction... rien � faire
RzxRezalSearch::~RzxRezalSearch()
{ }

///Retourne le QAbstractItemView auquel est rattach� l'objet actuel
QAbstractItemView *RzxRezalSearch::view() const
{ return qobject_cast<QAbstractItemView*>(parent()); }

///Retourne le RzxRezalModel
/** Ceci implique donc que le QAbstractItemView doit utiliser un RzxRezalModel
 * comme mod�le...
 */
RzxRezalModel *RzxRezalSearch::model() const
{ return qobject_cast<RzxRezalModel*>(view()->model()); }

///Retourne le filtre actuel
const QString &RzxRezalSearch::pattern() const
{ return searchPattern; }

///Retourne le mode de fonctionnement actuel du filtre
/** Si le filtre est en mode Config, alors retourne le mode indiqu� par
* le RzxMainuiConfig
*/
RzxRezalSearch::Mode RzxRezalSearch::mode() const
{
	if(searchMode == Lite || searchMode == Full)
		return searchMode;
	return (Mode)RzxMainUIConfig::searchMode();
}

///D�finie le mode de fonctionnement
void RzxRezalSearch::setMode(Mode mode)
{
	resetPattern();
	searchMode = mode;
}

/***************** Pour le mode de fonctionnement Full **************/
/* Ce mode effectue une recherche en plain text dans un grand nombre de
 * champs de donn�es et peut donc �tre tr�s lent.
 */

///Effectue le parcours des �l�ments pour cacher ceux ne correspondant pas
void RzxRezalSearch::filterView()
{
	if(mode() != Full)
		return;

	QAbstractItemView *v = view();
	QListView *list = qobject_cast<QListView*>(v);
	QTreeView *tree = qobject_cast<QTreeView*>(v);
	if(list)
		applyFilter<QListView>(v->rootIndex(), model(), list);
	else if(tree)
		applyFilter<QTreeView>(v->rootIndex(), model(), tree);
	else
		return;

	// Force refresh
	const QRect rect = v->visualRect(v->currentIndex());
	const bool viewCurrent = rect.bottom() >= 0 && rect.top() < v->viewport()->height();
	v->setRootIndex(v->rootIndex());
	if(viewCurrent)
		v->scrollTo(v->currentIndex());
	
	emit searchPatternChanged(searchPattern);
}

///V�rifie si un ordinateur correspond aux r�gles de filtrage actuelle
bool RzxRezalSearch::matches(const QModelIndex& index, RzxRezalModel *model) const
{
	if(mode() != Full || searchPattern.isEmpty() || !model->isComputer(index))
		return true;

	QVariant value = model->data(index, Qt::UserRole);
	const RzxComputer *computer = value.value<RzxComputer*>();
	if(computer == NULL)
		return false;
	
	QString searched = computer->name() + " "
		+ computer->client() + " "
		+ computer->sysExText() + " "
		+ computer->remarque();
	const RzxHostAddress &ip = computer->ip();
	searched += ip.toString() + " ";
	QStringList cache = RzxConfig::rawCache(ip);
	for(int i = 1 ; i < cache.size() ; i+=2)
	{
		searched += cache[i] + " ";
		if(cache[i-1] == "Sport")
			searched += QApplication::translate("RzxProperty", cache[i].toAscii().constData()) + " ";
	}

	QStringList words = searchPattern.split(" ");
	foreach (const QString &word, words)
	{
		if(!searched.contains(word, Qt::CaseInsensitive))
			return false;
	}

	return true;
}


/***************** Pour le mode de fonctionnement Lite **************/
/* Ce mode est plus complexe a implementer, mais est plus rapide a
 * ex�cuter. Il se limite � une recherche dans un arbre binaire �quilibr�
 * ce qui le rend extr�mement rapide
 */

///Retourne le temps de timeout du filtre
/** Un filtre de recherche timeout automatiquement au bout d'un certain temps...
 * Ceci �vite entre autre de continuer une rechercher oubli�e au lieu d'en recommencer
 * une nouvelle 
 */
int RzxRezalSearch::timeout() const
{ return timeLimit; }

///Met � jour le filtre
/** La mise � jour s'accompagne de la recherche de l'�l�ment le plus proche dans
 * le RzxDict des objets
 */
void RzxRezalSearch::setPattern(const QString& pattern)
{
	if(pattern == searchPattern) return;
	
	searchPattern = pattern;

	if(mode() == Full)
	{
		filterView();
		return;
	}
		
	searchTimeout.start();

	const RzxRezalSearchTree *itemByName = model()->childrenByName(view()->rootIndex());
	if(!itemByName) return;

	//Impl�mentation de la recherche par Ey pour qRezix 2.1
	RzxRezalSearchTree::const_iterator i_higher, i_lower;

	i_higher = itemByName->upperBound(searchPattern);
	if( i_higher != itemByName->begin() )
		i_lower  = i_higher-1;
	else
		i_lower = i_higher;

	if( ( i_lower.key() != searchPattern ) && ( i_higher != itemByName->end() ) )
	{
		QString higher   = i_higher.key(),
			lower    = i_lower.key();
		
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
			i_lower = i_higher;
	}

	emit searchPatternChanged(searchPattern);
	if( ( i_lower != itemByName->end() ) && i_lower.value() && !searchPattern.isEmpty() )
		emit findItem(model()->index(i_lower.value(), view()->rootIndex()));
}

///R�initialise le filtre de la rechercher
void RzxRezalSearch::resetPattern()
{
	setPattern(QString());
}

///Compl�te le filtre en ajoutant
void RzxRezalSearch::addToPattern(const QString& pattern)
{
	if(mode() == Full)
		return;
	testTimeout();
	setPattern(searchPattern + pattern);
}

///R�duit le filtre en supprimant ses derniers caract�res
void RzxRezalSearch::reducePattern(int size)
{
	testTimeout();
	int length = searchPattern.length() - size;
	if(length <= 0)
		resetPattern();
	else
		setPattern(searchPattern.left(length));
}

///D�fini le temps de timeout
void RzxRezalSearch::setTimeout(int time)
{
	timeLimit = time;
}

///Fait timeouter le filtre
/** Test si le filtre est trop ancien, et le fait timeout�
 * si n�cessaire
 */
void RzxRezalSearch::testTimeout()
{
	if(searchTimeout.elapsed() > timeLimit)
	{
		searchPattern = QString();
		emit searchPatternChanged(searchPattern);
	}
}

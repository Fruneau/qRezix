/***************************************************************************
                 rzxrezalmodel  -  Modèle de base de l'affichage
                             -------------------
    begin                : Sun Jul 17 2005
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
#include <QVariant>
#include <QString>
#include <QIcon>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxConnectionLister>

#include "rzxrezalmodel.h"
#include "rzxmainuiconfig.h"


RZX_GLOBAL_INIT(RzxRezalModel)

const char *RzxRezalModel::colNames[RzxRezalModel::numColonnes] = {
			QT_TR_NOOP("Computer name"),
			QT_TR_NOOP("Comment"),
			QT_TR_NOOP("Samba"),
			QT_TR_NOOP("FTP"),
			QT_TR_NOOP("Web"), 
			QT_TR_NOOP("News"),
			QT_TR_NOOP("Printer"),
			QT_TR_NOOP("OS"),
			QT_TR_NOOP("Gateway"),
			QT_TR_NOOP("Promo"),
			QT_TR_NOOP("Place"),
			QT_TR_NOOP("IP"),
			QT_TR_NOOP("Client")
};

///Pour le tri des RzxComputer
/** Cette fonction permet de comparer 2 RzxComputer selon l'ordre indiqué dans le RzxRezalModel */
bool sortComputer(RzxComputer *c1, RzxComputer *c2)
{
	RzxRezalModel::NumColonne order = RzxRezalModel::global()->order;
	if(RzxRezalModel::global()->sens == Qt::AscendingOrder)
		qSwap(c1, c2);
	if(!c1) return false;
	if(!c2) return true;
	const bool nameTest = c1->name().toLower() < c2->name().toLower();
	bool test;
	bool testEq;
	switch(order)
	{
		case RzxRezalModel::ColNom:
			return nameTest;
		case RzxRezalModel::ColRemarque:
			return c1->remarque().toLower() < c2->remarque().toLower();
		case RzxRezalModel::ColSamba: 
			testEq = c1->hasSambaServer() == c2->hasSambaServer();
			test = c1->hasSambaServer() && !c2->hasSambaServer(); break;
		case RzxRezalModel::ColFTP:
			testEq = c1->hasFtpServer() == c2->hasFtpServer();
			test = c1->hasFtpServer() && !c2->hasFtpServer(); break;
		case RzxRezalModel::ColHTTP:
			testEq = c1->hasHttpServer() == c2->hasHttpServer();
			test = c1->hasHttpServer() && !c2->hasHttpServer(); break;
		case RzxRezalModel::ColNews:
			testEq = c1->hasNewsServer() == c2->hasNewsServer();
			test = c1->hasNewsServer() && !c2->hasNewsServer(); break;
		case RzxRezalModel::ColOS:
			testEq = c1->sysEx() == c2->sysEx();
			test = c1->sysEx() < c2->sysEx(); break;
		case RzxRezalModel::ColGateway:
			testEq = test = c1->isSameGateway(c2); break;
		case RzxRezalModel::ColPromo:
			testEq = c1->promo() == c2->promo();
			test = c1->promo() < c2->promo(); break;
		case RzxRezalModel::ColRezal:
			testEq = c1->rezal() == c2->rezal();
			test = c1->rezal() < c2->rezal(); break;
		case RzxRezalModel::ColIP:
			testEq = c1->ip() == c2->ip();
			test = (qint32)c1->ip() < (qint32)c2->ip(); break;
		case RzxRezalModel::ColClient:
			testEq = c1->client() == c2->client();
			test = c1->client() < c2->client(); break;
		default: return false;
	}
	return test || (testEq && nameTest);
}

///Construction du Model
RzxRezalModel::RzxRezalModel()
		:everybodyGroup(numColonnes), favoritesGroup(numColonnes), favoriteIndex(numColonnes), ignoredIndex(numColonnes), neutralIndex(numColonnes), promoGroup(numColonnes), joneIndex(numColonnes), roujeIndex(numColonnes), oranjeIndex(numColonnes), rezalGroup(numColonnes)
{
	rezalIndex = new QVector<QPersistentModelIndex>[RzxConfig::rezalNumber()];
	rezals = new RzxRezalSearchList[RzxConfig::rezalNumber()];
	rezalsByName = new RzxRezalSearchTree[RzxConfig::rezalNumber()];

	for(int i = 0 ; i < numColonnes ; i++)
	{
		//Base de l'arbre
		if(!i) insertRows(0, 4);
		everybodyGroup[i] = QAbstractItemModel::createIndex(TREE_BASE_EVERYBODY, i, (int)TREE_FLAG_BASE);
		favoritesGroup[i] = QAbstractItemModel::createIndex(TREE_BASE_FAVORITE, i, (int)TREE_FLAG_BASE);
		promoGroup[i] = QAbstractItemModel::createIndex(TREE_BASE_PROMO, i, (int)TREE_FLAG_BASE);
		rezalGroup[i] = QAbstractItemModel::createIndex(TREE_BASE_REZAL, i, (int)TREE_FLAG_BASE);

		//Arborescence favoris/ignoré
		if(!i) insertRows(0, 3, favoritesGroup[0]);
		favoriteIndex[i] = QAbstractItemModel::createIndex(TREE_FAVORITE_FAVORITE, i, (int)TREE_FLAG_FAVORITE);
		ignoredIndex[i] = QAbstractItemModel::createIndex(TREE_FAVORITE_IGNORED, i, (int)TREE_FLAG_FAVORITE);
		neutralIndex[i] = QAbstractItemModel::createIndex(TREE_FAVORITE_NEUTRAL, i, (int)TREE_FLAG_FAVORITE);

		//Arborescence par promo
		if(!i) insertRows(0, 4, promoGroup[0]);
		joneIndex[i] = QAbstractItemModel::createIndex(TREE_PROMO_JONE, i, (int)TREE_FLAG_PROMO);
		roujeIndex[i] = QAbstractItemModel::createIndex(TREE_PROMO_ROUJE, i, (int)TREE_FLAG_PROMO);
		oranjeIndex[i] = QAbstractItemModel::createIndex(TREE_PROMO_ORANJE, i, (int)TREE_FLAG_PROMO);
	
		//Arborescence par rezal
		if(!i) insertRows(0, RzxConfig::rezalNumber(), rezalGroup[0]);
		for(uint j = 0 ; j < RzxConfig::rezalNumber() ; j++)
		{
			if(!i) rezalIndex[j].resize(numColonnes);
			rezalIndex[j][i] = QAbstractItemModel::createIndex(j, i, (int)TREE_FLAG_REZAL);
		}
	}

	//Initialisation de l'ordre de tri
	order = (NumColonne)RzxMainUIConfig::sortColumn();
	sens = RzxMainUIConfig::sortOrder();

	object = this;
	QList<RzxComputer*> computers = RzxConnectionLister::global()->computerList();
	foreach(RzxComputer *computer, computers)
		login(computer);

	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(update(RzxComputer* )), this, SLOT(update(RzxComputer* )));
}

///Détruit le modèle
RzxRezalModel::~RzxRezalModel()
{
	RZX_GLOBAL_CLOSE
	RzxMainUIConfig::setSortColumn(order);
	RzxMainUIConfig::setSortOrder(sens);
}

///Retourne l'index correspondant à l'objet indiqué
QModelIndex RzxRezalModel::index(int row, int column, const QModelIndex& parent) const
{
	if(column < 0 || column >= numColonnes) return QModelIndex();

	//Le parent est invalide, c'est donc la racine...
	//Ses fils sont donc les début de branche :
	//	- Tout le monde... (BASE_EVERYBODY)
	//	- Favoris/Ignored (BASE_FAVORITE)
	//	- Par promo (BASE_PROMO)
	//	- Bâtiments (BASE_REZAL)
	if(!parent.isValid())
		switch(row)
		{
			case TREE_BASE_EVERYBODY: return everybodyGroup[column]; break;
			case TREE_BASE_FAVORITE: return favoritesGroup[column]; break;
			case TREE_BASE_PROMO: return promoGroup[column]; break;
			case TREE_BASE_REZAL: return rezalGroup[column]; break;
			default: return QModelIndex();
		}

	//Si le parent est valide, on filtre sur la catégorie
	//Comme dans la plupart des autres fonctions de cette classe, le seul cas
	//vraiment particulier est le filtrage sur les rezal
	const uint category = parent.internalId();
	const uint value = parent.row();
	switch(category)
	{
		//Le parent est un élément de la base
		//Ces éléments sont donc identifié par la valeur
		case TREE_FLAG_BASE:
			switch(value)
			{
				//Le père est EveryBody
				// - ... (FLAG_EVERYBODY | row)
				case TREE_BASE_EVERYBODY: return createIndex(row, column, TREE_FLAG_EVERYBODY, everybody);

				//Le père est Favorites
				// - Favoris (FLAG_FAVORITE | FAVORITE_FAVORITE)
				// - Ignored (FLAG_FAVORITE | FAVORITE_IGNORED)
				// - Autres (FLAG_FAVORITE | FAVORITE_NEUTRAL)
				case TREE_BASE_FAVORITE:
					switch(row)
					{
						case TREE_FAVORITE_FAVORITE: return favoriteIndex[column]; break;
						case TREE_FAVORITE_IGNORED: return ignoredIndex[column]; break;
						case TREE_FAVORITE_NEUTRAL: return neutralIndex[column]; break;
					}
					break;

				//Le père est Promo
				// - Jônes (FLAG_PROMO | PROMO_JONE)
				// - Roujes (FLAG_PROMO | PROMO_ROUJE)
				// - Oranjes (FLAG_PROMO | PROMO_ORANJE)
				// - Binets (FLAG_PROMO | PROMO_BINET)
				case TREE_BASE_PROMO:
					switch(row)
					{
						case TREE_PROMO_JONE: return joneIndex[column]; break;
						case TREE_PROMO_ROUJE: return roujeIndex[column]; break;
						case TREE_PROMO_ORANJE: return oranjeIndex[column]; break;
					}
					break;

				//Le père est Rezal, donc row est le rezalId
				//- Rezal (FLAG_REZAL | rezalId)
				case TREE_BASE_REZAL:
					if(row < (int)RzxConfig::rezalNumber())
						return rezalIndex[row][column];
					break;
			}
			break;

		//Le père est un élément du groupe Favoris...
		//Ce père est identifié par value.
		// - ... (TREE_FLAG_FAVORITE_$SSGROUP | row)
		case TREE_FLAG_FAVORITE:
			switch(value)
			{
				case TREE_FAVORITE_FAVORITE: return createIndex(row, column, TREE_FLAG_FAVORITE_FAVORITE, favorites);
				case TREE_FAVORITE_IGNORED: return createIndex(row, column, TREE_FLAG_FAVORITE_IGNORED, ignored);
				case TREE_FAVORITE_NEUTRAL: return createIndex(row, column, TREE_FLAG_FAVORITE_NEUTRAL, neutral);
			}
			break;

		//Le père est un élément du groupe Promo
		// - ... (TREE_FLAG_PROMO_$SSGROUP | row)
		case TREE_FLAG_PROMO:
			switch(value)
			{
				case TREE_PROMO_JONE: return createIndex(row, column, TREE_FLAG_PROMO_JONE, jone);
				case TREE_PROMO_ROUJE: return createIndex(row, column, TREE_FLAG_PROMO_ROUJE, rouje);
				case TREE_PROMO_ORANJE: return createIndex(row, column, TREE_FLAG_PROMO_ORANJE, oranje);
			}
			break;

		//Le père est un élément du group Rezal
		// - ...  (FLAG_REZAL | ((rezalId + 1) << 16) | row)
		case TREE_FLAG_REZAL:
			if(value < RzxConfig::rezalNumber())
				return createIndex(row, column, GET_ID_FROM_REZAL(value), rezals[value]);
			break;
	}

	return QModelIndex();
}

///Retourne l'index du RzxComputer dans le parent indiqué
QModelIndex RzxRezalModel::index(RzxComputer *computer, const QModelIndex& parent) const
{
	if(!parent.isValid()) return QModelIndex();
	const RzxRezalSearchList *list = NULL;
	switch(parent.internalId())
	{
		case TREE_FLAG_BASE:
			if(parent.row() == TREE_BASE_EVERYBODY) list = &everybody;
			break;
			
		case TREE_FLAG_FAVORITE:
			switch(parent.row())
			{
				case TREE_FAVORITE_FAVORITE: list = &favorites; break;
				case TREE_FAVORITE_IGNORED: list = &ignored; break;
				case TREE_FAVORITE_NEUTRAL: list = &neutral; break;
			}
			break;
			
		case TREE_FLAG_PROMO:
			switch(parent.row())
			{
				case TREE_PROMO_JONE: list = &jone; break;
				case TREE_PROMO_ROUJE: list = &rouje; break;
				case TREE_PROMO_ORANJE: list = &oranje; break;
			}
			break;
			
		case TREE_FLAG_REZAL:
			list = &rezals[parent.row()];
			break;
			
		default:
			return QModelIndex();
	}

	if(list)
		return index(list->indexOf(computer), 0, parent);
	else
		return QModelIndex();
}

///Recherche l'index associé au subnet donné
QModelIndex RzxRezalModel::index(const RzxSubnet& subnet) const
{
	int i = RzxConfig::rezal(subnet);
	if(i == -1) return QModelIndex();
	return rezalIndex[i][0];
}

///Retourne l'arbre des fils
const RzxRezalSearchTree *RzxRezalModel::childrenByName(const QModelIndex& index) const
{
	if(!index.isValid()) return NULL;
	switch(index.internalId())
	{
		case TREE_FLAG_BASE:
			if(index.row() == TREE_BASE_EVERYBODY) return &everybodyByName;
			break;
			
		case TREE_FLAG_FAVORITE:
			switch(index.row())
			{
				case TREE_FAVORITE_FAVORITE: return &favoritesByName;
				case TREE_FAVORITE_IGNORED: return &ignoredByName;
				case TREE_FAVORITE_NEUTRAL: return &neutralByName;
			}
			break;
			
		case TREE_FLAG_PROMO:
			switch(index.row())
			{
				case TREE_PROMO_JONE: return &joneByName;
				case TREE_PROMO_ROUJE: return &roujeByName;
				case TREE_PROMO_ORANJE: return &oranjeByName;
			}
			break;
			
		case TREE_FLAG_REZAL:
			return &rezalsByName[index.row()];
	}
	return NULL;
}

///Retourne le parent
QModelIndex RzxRezalModel::parent(const QModelIndex& index) const
{
	if(!index.isValid()) return QModelIndex();
	switch(index.internalId())
	{
		case TREE_FLAG_BASE: return QModelIndex();
		case TREE_FLAG_EVERYBODY: return everybodyGroup[0];
		case TREE_FLAG_FAVORITE: return favoritesGroup[0];
		case TREE_FLAG_PROMO: return promoGroup[0];
		case TREE_FLAG_REZAL: return rezalGroup[0];
		case TREE_FLAG_FAVORITE_FAVORITE: return favoriteIndex[0];
		case TREE_FLAG_FAVORITE_IGNORED: return ignoredIndex[0];
		case TREE_FLAG_FAVORITE_NEUTRAL: return neutralIndex[0];
		case TREE_FLAG_PROMO_JONE: return joneIndex[0];
		case TREE_FLAG_PROMO_ROUJE: return roujeIndex[0];
		case TREE_FLAG_PROMO_ORANJE: return oranjeIndex[0];
	}

	//Cas Particulier : les rezal
	if((index.internalId() & TREE_FLAG_HARDMASK) == TREE_FLAG_REZAL)
	{
		uint rezal = GET_REZAL_FROM_ID(index.internalId());
		if(rezal < RzxConfig::rezalNumber())
			return rezalIndex[rezal][0];
	}

	return QModelIndex();
}

///Indique si le QModelIndex est un objet d'index de l'arbre
/** C'est à dire que ce n'est pas un élément contenant des RzxComputer ou
 * un élément correspondant à un RzxComputer
 */
bool RzxRezalModel::isIndex(const QModelIndex& index) const
{
	if(!index.isValid()) return true;
	
	const int category = index.internalId();
	const int value = index.row();
	if(category == TREE_FLAG_BASE && value != TREE_BASE_EVERYBODY)
		return true;
	return false;
}

///Indique si le QModelIndex représente un ordinateur...
/** C'est à dire si on peut en extraire un RzxComputer valide
 */
bool RzxRezalModel::isComputer(const QModelIndex& index) const
{
	if(!index.isValid()) return false;

	if(data(index, Qt::UserRole).canConvert<RzxComputer*>())
		return true;
	return false;
}

///Indique si le QModelIndex est un index d'un rezal
/** C'est à dire que son contenu est tous les RzxComputer d'un subnet
 */
int RzxRezalModel::rezal(const QModelIndex& index) const
{
	if(!index.isValid()) return -1;
	
	const int category = index.internalId();
	const int value = index.row();
	if(category == TREE_FLAG_REZAL && value < (int)RzxConfig::rezalNumber())
		return value;
	return -1;
}

///Indique si l'index a des fils
bool RzxRezalModel::hasChildren(const QModelIndex& index) const
{
	if(rowCount(index))
		return true;
	return false;
}

///Retourne le nombre de fils de l'index
int RzxRezalModel::rowCount(const QModelIndex& index) const
{
	//Les fils de la racine sont BASE_NUMBER
	if(!index.isValid())
		return TREE_BASE_NUMBER;

	//On extrait de l'identifiant l'indicateur d'arborescence
	const uint category = index.internalId();
	const uint value = index.row();
	switch(category)
	{
		case TREE_FLAG_BASE:
			switch(value)
			{
				case TREE_BASE_EVERYBODY: return everybody.count();
				case TREE_BASE_FAVORITE: return TREE_FAVORITE_NUMBER;
				case TREE_BASE_PROMO: return TREE_PROMO_NUMBER;
				case TREE_BASE_REZAL: return RzxConfig::rezalNumber();
			}
			break;

		case TREE_FLAG_FAVORITE:
			switch(value)
			{
				case TREE_FAVORITE_FAVORITE: return favorites.count();
				case TREE_FAVORITE_IGNORED: return ignored.count();
				case TREE_FAVORITE_NEUTRAL: return neutral.count();
			}
			break;

		case TREE_FLAG_PROMO:
			switch(value)
			{
				case TREE_PROMO_JONE: return jone.count();
				case TREE_PROMO_ROUJE: return rouje.count();
				case TREE_PROMO_ORANJE: return oranje.count();
			}
			break;

		case TREE_FLAG_REZAL:
			if(value < RzxConfig::rezalNumber())
				return rezals[value].count();
	}

	//Sinon pas de children ;)
	return 0;
}


///Retourne le nombre de colonnes
/** Le nombre de colonnes est constamment 1... */
int RzxRezalModel::columnCount(const QModelIndex&) const
{
	return numColonnes;
}

///Retourne les données associées à l'objet
QVariant RzxRezalModel::data(const QModelIndex& index, int role) const
{
	//Fils de la racine
	if(!index.isValid())
		return QVariant();

	const uint column = index.column();
	const uint value = index.row();
	const uint category = index.internalId();
	int child = rowCount(index);
			
	switch(category)
	{
		case TREE_FLAG_BASE:
			if(column) return QVariant();
			switch(value)
			{
				case TREE_BASE_EVERYBODY: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Everybody"));
				case TREE_BASE_FAVORITE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Category"));
				case TREE_BASE_PROMO: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("Promo"));
				case TREE_BASE_REZAL: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), tr("Subnet"));
			}
			break;

		case TREE_FLAG_EVERYBODY: return getComputer(role, everybody, value, column);
		case TREE_FLAG_FAVORITE:
			if(column) return QVariant();
			switch(value)
			{
				case TREE_FAVORITE_FAVORITE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Favorites"));
				case TREE_FAVORITE_IGNORED: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_BAN), tr("Banned"));
				case TREE_FAVORITE_NEUTRAL: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Others..."));
			}
			break;

		case TREE_FLAG_PROMO:
			if(column) return QVariant();
			switch(value)
			{
				case TREE_PROMO_JONE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_JONE), tr("Jones"));
				case TREE_PROMO_ROUJE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_ROUJE), tr("Roujes"));
				case TREE_PROMO_ORANJE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("Oranjes"));
			}
			break;

		case TREE_FLAG_REZAL:
			if(column) return QVariant();
			return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), RzxConfig::rezalName(value, false));

		case TREE_FLAG_FAVORITE_FAVORITE: return getComputer(role, favorites, value, column);
		case TREE_FLAG_FAVORITE_IGNORED: return getComputer(role, ignored, value, column);
		case TREE_FLAG_FAVORITE_NEUTRAL: return getComputer(role, neutral, value, column);

		case TREE_FLAG_PROMO_JONE: return getComputer(role, jone, value, column);
		case TREE_FLAG_PROMO_ROUJE: return getComputer(role, rouje, value, column);
		case TREE_FLAG_PROMO_ORANJE: return getComputer(role, oranje, value, column);
	}

	if((category & TREE_FLAG_HARDMASK) == TREE_FLAG_REZAL)
	{
		const uint rezalId = index.parent().row(); //GET_REZAL_FROM_ID(category);
		if(rezalId < RzxConfig::rezalNumber())
			return getComputer(role, rezals[rezalId], value, column);
	}

	return QString::number(category, 16);
}

///Extraction du computer
/** L'extraction se fait après vérification des indexes... c'est d'ailleurs le seul intérêt de cette fonction ;)
 */
QVariant RzxRezalModel::getComputer(int role, const RzxRezalSearchList& list, int pos, int column) const
{
	
	if(pos < 0 || pos >= list.count())
		return QVariant();
	RzxComputer *computer = list[pos];
	if(!computer)
		return QVariant();
	switch(role)
	{
		case Qt::DisplayRole:
			switch(column)
			{
				case ColNom: return computer->name();
				case ColRemarque: return computer->remarque();
				case ColRezal: return computer->rezalName();
				case ColIP: return computer->ip().toString();
				case ColClient: return computer->client();
				default: return QVariant();
			}

		case Qt::DecorationRole:
			switch(column)
			{
				case ColNom: return QIcon(computer->icon());
				case ColSamba: return RzxIconCollection::getIcon(computer->hasSambaServer()?Rzx::ICON_SAMBA:Rzx::ICON_NOSAMBA);
				case ColFTP: return RzxIconCollection::getIcon(computer->hasFtpServer()?Rzx::ICON_FTP:Rzx::ICON_NOFTP);
				case ColHTTP: return RzxIconCollection::getIcon(computer->hasHttpServer()?Rzx::ICON_HTTP:Rzx::ICON_NOHTTP);
				case ColNews: return RzxIconCollection::getIcon(computer->hasNewsServer()?Rzx::ICON_NEWS:Rzx::ICON_NONEWS);
				case ColPrinter: return RzxIconCollection::getIcon(computer->hasPrinter()?Rzx::ICON_PRINTER:Rzx::ICON_NOPRINTER);
				case ColGateway: return RzxIconCollection::getIcon(computer->isSameGateway()?Rzx::ICON_SAMEGATEWAY:Rzx::ICON_OTHERGATEWAY);
				case ColPromo: return RzxIconCollection::global()->promoIcon(computer->promo());
				case ColOS: return RzxIconCollection::global()->osIcon(computer->sysEx());
				default: return QVariant();
			}

		case Qt::ToolTipRole: return tooltip(computer);

		case Qt::TextAlignmentRole:
			if(column == ColRemarque) return (int)Qt::AlignLeft | Qt::AlignVCenter;
			return (int)Qt::AlignCenter;

		case Qt::BackgroundColorRole:
			if(computer->state() == Rzx::STATE_AWAY || computer->state() == Rzx::STATE_REFUSE)
				return QColor(RzxConfig::repondeurBase());
			return QVariant();

		case Qt::UserRole: return QVariant::fromValue<RzxComputer*>(computer);
		default: return QVariant();
	}
}


///Génère un tooltip formaté correspondant à l'objet
/** Le tooltip généré selon les préférences exprimées par l'utilisateur, avec les informations qui constitue le RzxComputer :
 * 	- NOM
 * 	- Informations :
 * 		- serveurs
 * 		- promo
 * 		- ip/rezal
 * 		- client/modules
 * 	- Propriétés (dernière propriétés en cache pour ce client)
 */
QString RzxRezalModel::tooltip(const RzxComputer *computer) const
{
	const int tooltipFlags = RzxMainUIConfig::tooltip();
	if(!(tooltipFlags & (int)RzxRezalModel::TipEnable) || tooltipFlags==(int)RzxRezalModel::TipEnable) return "";
	
	QString tooltip = "<b>"+ computer->name() + " </b>";
	if(tooltipFlags & (int)RzxRezalModel::TipPromo)
		tooltip += "<i>(" + computer->promoText() + ")</i>";
	tooltip += "<br/><br/>";
 	tooltip += "<b><i>" + tr("Informations:") + " </b></i><br/>";
	
	if(computer->hasFtpServer() && (tooltipFlags & (int)RzxRezalModel::TipFtp))
		tooltip += "<b>-></b>&nbsp;" + tr("ftp server:") + " " + tr("<b>on</b>") + "<br/>";
	if(computer->hasHttpServer() && (tooltipFlags & (int)RzxRezalModel::TipHttp))
		tooltip += "<b>-></b>&nbsp;" + tr("web server:") + " " + tr("<b>on</b>") + "<br/>";
	if(computer->hasNewsServer() && (tooltipFlags & (int)RzxRezalModel::TipNews))
		tooltip += "<b>-></b>&nbsp;" + tr("news server:") + " " + tr("<b>on</b>") + "<br/>";
	if(computer->hasSambaServer() && (tooltipFlags & (int)RzxRezalModel::TipSamba))
		tooltip += "<b>-></b>&nbsp;" + tr("samba server:") + " " + tr("<b>on</b>") + "<br/>";
	if(computer->hasPrinter() && (tooltipFlags & (int)RzxRezalModel::TipPrinter))
		tooltip += "<b>-></b>&nbsp;" + tr("printer:") + " " + tr("<b>yes</b>") + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipOS)
		tooltip += "<b>-></b>&nbsp;os : " + computer->sysExText() + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipClient)
		tooltip += "<b>-></b>&nbsp;" + computer->client() + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipFeatures)
	{
		if(computer->can(Rzx::CAP_ON))
		{
			int nb = 0;
			tooltip += "<b>-></b>&nbsp;" + tr("features:");
			if(computer->can(Rzx::CAP_CHAT))
			{
				tooltip += tr("chat");
				nb++;
			}
			if(computer->can(Rzx::CAP_XPLO))
			{
				tooltip += QString(nb?", ":"") + "Xplo";
				nb++;
			}
			if(!nb) tooltip += tr("none");
			tooltip += "<br/>";
		}
	}
	if(tooltipFlags & (int)RzxRezalModel::TipIP)
		tooltip += "<b>-></b>&nbsp;ip : <i>" + computer->ip().toString() + "</i><br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipResal)
		tooltip += "<b>-></b>&nbsp;" + tr("Location:") + computer->rezalName(false) + "<br/>";
	
	if(tooltipFlags & (int)RzxRezalModel::TipProperties)
	{
		tooltip += "<br/>";
		QString msg = RzxConfig::cache(computer->ip());
		if(msg.isNull())
		{
			tooltip += "<i>" + tr("No properties cached") + "</i>";
		}
		else
		{
			QString date = RzxConfig::getCacheDate(computer->ip());
			tooltip += "<b><i>" + tr("Properties checked on ")  + date + " :</i></b><br/>";
			QStringList list = msg.split("|");
			for(int i = 0 ; i < list.size() - 1 ; i+=2)
				tooltip += "<b>-></b>&nbsp;" + list[i] + " : " + list[i+1] + "<br/>";
		}
	}

	return tooltip;
}

///Extraction de l'objet pour le menu
/** Contrairement aux items pour lesquels le RzxComputer contient la totalité des informations importantes,
 * les menus doivent être définis à la main par l'utilisateur... */
QVariant RzxRezalModel::getMenuItem(int role, int children, const QIcon& icon, const QString& name, const QString& desc) const
{
	const QString description = (desc.isNull()?name:desc);

	switch(role)
	{
		case Qt::DisplayRole: return name;
		case Qt::DecorationRole: return icon;
		case Qt::ToolTipRole: return description + " (" + QString::number(children) + ")";
		default: return QVariant();
	}
}

///Récupération des en-têtes de colonne
QVariant RzxRezalModel::headerData(int column, Qt::Orientation orientation, int role) const
{
	if(orientation != Qt::Horizontal)
		return QVariant();

	switch(role)
	{
		case Qt::DisplayRole: 
			switch(column)
			{
				case ColSamba: case ColFTP: case ColHTTP:
				case ColNews: case ColGateway: case ColPromo:
				case ColPrinter:
					return QVariant();
				default:
					return tr(colNames[column]);
			}

		case Qt::DecorationRole:
			switch(column)
			{
				case ColSamba: return RzxIconCollection::getIcon(Rzx::ICON_SAMBA);
				case ColFTP: return RzxIconCollection::getIcon(Rzx::ICON_FTP);
				case ColHTTP: return RzxIconCollection::getIcon(Rzx::ICON_HTTP);
				case ColNews: return RzxIconCollection::getIcon(Rzx::ICON_NEWS);
				case ColPrinter: return RzxIconCollection::getIcon(Rzx::ICON_PRINTER);
				case ColGateway: return RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY);
				case ColPromo: return RzxIconCollection::global()->promoIcon(Rzx::PROMAL_ORANGE);
				default: return QVariant();
			}

		case Qt::ToolTipRole:
			return columnName((NumColonne)column);

		default: return QVariant();
	}
}

///Enregistrement de l'arrivée d'un nouvel élément
void RzxRezalModel::login(RzxComputer *computer)
{
	if(!computer) return;

	connect(computer, SIGNAL(update(RzxComputer* )), this, SLOT(update(RzxComputer* )));
	if(everybody.contains(computer))
	{
		update(computer);
		return;
	}

	//Un nouvel objet... donc on le stocke
	insertObject(everybodyGroup[0], everybody, everybodyByName, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
		insertObject(favoriteIndex[0], favorites, favoritesByName, computer);
	else if(computer->isIgnored())
		insertObject(ignoredIndex[0], ignored, ignoredByName, computer);
	else
		insertObject(neutralIndex[0], neutral, neutralByName, computer);

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			insertObject(oranjeIndex[0], oranje, oranjeByName, computer);
			break;
		case Rzx::PROMAL_JONE:
			insertObject(joneIndex[0], jone, joneByName, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			insertObject(roujeIndex[0], rouje, roujeByName, computer);
			break;
	}

	//Rangement en rezal
	int rezalId = computer->rezal();
	if(rezalId >= 0 && rezalId < (int)RzxConfig::rezalNumber())
		insertObject(rezalIndex[rezalId][0], rezals[rezalId], rezalsByName[rezalId], computer);
}

///Un computer vient de se déconnecter...
void RzxRezalModel::logout(RzxComputer *computer)
{
	if(!computer) return;

	//Un objet est supprimé :/
	removeObject(everybodyGroup[0], everybody, everybodyByName, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
		removeObject(favoriteIndex[0], favorites, favoritesByName, computer);
	else if(computer->isIgnored())
		removeObject(ignoredIndex[0], ignored, ignoredByName, computer);
	else
		removeObject(neutralIndex[0], neutral, neutralByName, computer);

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			removeObject(oranjeIndex[0], oranje, oranjeByName, computer);
			break;
		case Rzx::PROMAL_JONE:
			removeObject(joneIndex[0], jone, joneByName, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			removeObject(roujeIndex[0], rouje, roujeByName, computer);
			break;
	}

	//Rangement en rezal
	int rezalId = computer->rezal();
	if(rezalId >= 0 && rezalId < (int)RzxConfig::rezalNumber())
		removeObject(rezalIndex[rezalId][0], rezals[rezalId], rezalsByName[rezalId], computer);
}

///Un computer vient de mettre à jours les informations qui le concerne
void RzxRezalModel::update(RzxComputer *computer)
{
	if(!computer) return;

	updateObject(everybodyGroup[0], everybody, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
	{
		if(!favorites.contains(computer))
		{
			removeObject(neutralIndex[0], neutral, neutralByName, computer);
			insertObject(favoriteIndex[0], favorites, favoritesByName, computer);
			removeObject(ignoredIndex[0], ignored, ignoredByName, computer);
		}
		else
			updateObject(favoriteIndex[0], favorites, computer);
	}
	else if(computer->isIgnored())
	{
		if(!ignored.contains(computer))
		{
			removeObject(neutralIndex[0], neutral, neutralByName, computer);
			removeObject(favoriteIndex[0], favorites, favoritesByName, computer);
			insertObject(ignoredIndex[0], ignored, ignoredByName, computer);
		}
		else
			updateObject(ignoredIndex[0], ignored, computer);
	}
	else
	{
		if(!neutral.contains(computer))
		{
			insertObject(neutralIndex[0], neutral, neutralByName, computer);
			removeObject(favoriteIndex[0], favorites, favoritesByName, computer);
			removeObject(ignoredIndex[0], ignored, ignoredByName, computer);
		}
		else
			updateObject(neutralIndex[0], neutral, computer);
	}

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			if(!oranje.contains(computer))
			{
				removeObject(roujeIndex[0], rouje, roujeByName, computer);
				insertObject(oranjeIndex[0], oranje, oranjeByName, computer);
				removeObject(joneIndex[0], jone, joneByName, computer);
			}
			else
				updateObject(oranjeIndex[0], oranje, computer);
			break;
		case Rzx::PROMAL_JONE:
			if(!jone.contains(computer))
			{
				removeObject(roujeIndex[0], rouje, roujeByName, computer);
				removeObject(oranjeIndex[0], oranje, oranjeByName, computer);
				insertObject(joneIndex[0], jone, joneByName, computer);
			}
			else
				updateObject(joneIndex[0], jone, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			if(!rouje.contains(computer))
			{
				insertObject(roujeIndex[0], rouje, roujeByName, computer);
				removeObject(oranjeIndex[0], oranje, oranjeByName, computer);
				removeObject(joneIndex[0], jone, joneByName, computer);
			}
			else
				updateObject(roujeIndex[0], rouje, computer);
			break;
	}

	//Rangement en rezal
	uint rezalId = computer->rezal();
	if(rezalId < RzxConfig::rezalNumber() && !rezals[rezalId].contains(computer))
	{
		for(uint i = 0 ; i < RzxConfig::rezalNumber() ; i++)
			if(i == rezalId)
				insertObject(rezalIndex[i][0], rezals[i], rezalsByName[i], computer);
			else
				removeObject(rezalIndex[i][0], rezals[i], rezalsByName[i], computer);
	}
	else if(rezalId < RzxConfig::rezalNumber())
		updateObject(rezalIndex[rezalId][0], rezals[rezalId], computer);
	sort(order, sens);
}

///Vide complètement la liste des objets connus
void RzxRezalModel::clear()
{
#define deleteGroup(list, model) \
	{ \
		beginRemoveRows(model, 0, list.count() - 1); \
		list.clear(); \
		endRemoveRows(); \
	}
	deleteGroup(everybody, everybodyGroup[0]);
	deleteGroup(favorites, favoriteIndex[0]);
	deleteGroup(ignored, ignoredIndex[0]);
	deleteGroup(neutral, neutralIndex[0]);
	deleteGroup(jone, joneIndex[0]);
	deleteGroup(rouje, roujeIndex[0]);
	deleteGroup(oranje, oranjeIndex[0]);
	for(uint i = 0 ; i < RzxConfig::rezalNumber() ; i++)
		deleteGroup(rezals[i], rezalIndex[i][0]);
}


///Insertion d'un objet dans la liste et le groupe correspondant
void RzxRezalModel::insertObject(const QModelIndex& parent, RzxRezalSearchList& list, RzxRezalSearchTree& tree, RzxComputer *computer)
{
	int row = 0;
	while(row < list.count() && sortComputer(list[row], computer))
		row++;
	beginInsertRows(parent, row, row);
	list.insert(row, computer);
	tree.insert(computer->name().toLower(), new QPointer<RzxComputer>(computer));
	endInsertRows();
}

///Suppression d'un objet de la liste et du groupe correspondant
void RzxRezalModel::removeObject(const QModelIndex& parent, RzxRezalSearchList& list, RzxRezalSearchTree& tree, RzxComputer *computer)
{
	int row;
	//Nettoyage de la liste
	while((row = list.indexOf(NULL)) != -1)
	{
		beginRemoveRows(parent, row, row);
		list.removeAt(row);
		endRemoveRows();
	}
	//Suppression de l'ordinateur demandé
	row = list.indexOf(computer);
	if(row == -1) return;
	beginRemoveRows(parent, row, row);
	list.removeAll(computer);
	tree.remove(computer->name().toLower());
	endRemoveRows();
}

///Mise à jours des données concernant un object
void RzxRezalModel::updateObject(const QModelIndex& parent, const RzxRezalSearchList& list, RzxComputer *computer)
{
	const int row = list.indexOf(computer);
	if(row == -1) return;
	QModelIndex baseIndex = index(row, 0, parent);
	QModelIndex endIndex = index(row, numColonnes-1, parent);
	emit dataChanged(baseIndex, endIndex);
}

///Retourne la liste des items correspondants à la selection
/** Etant donné que chaque RzxComputer est représenté plusieurs fois dans les données
 * lorsqu'on sélection un item, on sélectionne pas seulement le RzxComputer correspondant...
 * le but de cette fonction est donc de rassembler dans la sélection tous les items qui ont le même
 * RzxComputer.
 */
QModelIndexList RzxRezalModel::selected(const QModelIndex& ref) const
{
#define insert(list, model) \
	{ \
		int offset = list.indexOf(computer); \
		if(offset != -1) \
			indexList << index(offset, 0, model); \
	}

	const QVariant value = data(ref, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
		return QModelIndexList() << ref;

	QModelIndexList indexList;
	RzxComputer *computer = value.value<RzxComputer*>();
	insert(everybody, everybodyGroup[0]);
	insert(favorites, favoriteIndex[0]);
	insert(ignored, ignoredIndex[0]);
	insert(neutral, neutralIndex[0]);
	insert(jone, joneIndex[0]);
	insert(rouje, roujeIndex[0]);
	insert(oranje, oranjeIndex[0]);
	for(uint i = 0 ; i<RzxConfig::rezalNumber() ; i++)
		insert(rezals[i], rezalIndex[i][0]);
	return indexList;
}

///Trie toutes les listes dans l'ordre demandé
void RzxRezalModel::sort(int column, Qt::SortOrder sortSens)
{
#define sortList(list, model) \
	if(list.count()) \
	{ \
		QModelIndex begin = index(0, 0, model); \
		QModelIndex end = index(rowCount(model) - 1, numColonnes - 1, model); \
		emit dataChanged(begin, end); \
		qSort(list.begin(), list.end(), sortComputer); \
	}

	order = (RzxRezalModel::NumColonne)column;
	sens = sortSens;

	sortList(everybody, everybodyGroup[0]);
	sortList(favorites, favoriteIndex[0]);
	sortList(ignored, ignoredIndex[0]);
	sortList(neutral, neutralIndex[0]);
	sortList(jone, joneIndex[0]);
	sortList(rouje, roujeIndex[0]);
	sortList(oranje, oranjeIndex[0]);
	for(uint i=0 ; i < RzxConfig::rezalNumber() ; i++)
		sortList(rezals[i], rezalIndex[i][0]);
#undef sortList
}

///Retourne le nom de la colonne indiquée
QString RzxRezalModel::columnName(NumColonne column) const
{
	return tr(colNames[column]);
}

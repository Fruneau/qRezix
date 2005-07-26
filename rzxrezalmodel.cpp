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

#include "rzxrezalmodel.h"

#include "rzxcomputer.h"
#include "rzxiconcollection.h"
#include "rzxconnectionlister.h"

RzxRezalModel *RzxRezalModel::object = NULL;

const char *RzxRezalModel::colNames[RzxRezalModel::numColonnes] = {
			QT_TR_NOOP("Computer name"),
			QT_TR_NOOP("Comment"),
			QT_TR_NOOP("Samba"),
			QT_TR_NOOP("FTP"),
			QT_TR_NOOP("Web"), 
			QT_TR_NOOP("News"),
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
	switch(order)
	{
		case RzxRezalModel::ColNom: return c1->name().toLower() < c2->name().toLower();
		case RzxRezalModel::ColRemarque: return c1->remarque().toLower() < c2->remarque().toLower();
		case RzxRezalModel::ColSamba: return c1->hasSambaServer() && !c2->hasSambaServer();
		case RzxRezalModel::ColFTP: return c1->hasFtpServer() && !c2->hasFtpServer();
		case RzxRezalModel::ColHTTP: return c1->hasHttpServer() && !c2->hasHttpServer();
		case RzxRezalModel::ColNews: return c1->hasNewsServer() && !c2->hasNewsServer();
		case RzxRezalModel::ColOS: return c1->sysEx() < c2->sysEx();
		case RzxRezalModel::ColGateway: return c1->rezalId() == c2->rezalId();
		case RzxRezalModel::ColPromo: return c1->promo() < c2->promo();
		case RzxRezalModel::ColRezal: return c1->rezalId() < c2->rezalId();
		case RzxRezalModel::ColIP: return c1->ip() < c2->ip();
		case RzxRezalModel::ColClient: return c1->client() < c2->client();
		default: return false;
	}
}

///Construction du Model
RzxRezalModel::RzxRezalModel()
{
	qRegisterMetaType<RzxComputer*>("RzxComputer*");

	//Base de l'arbre
	insertRows(0, 4);
	everybodyGroup = QAbstractItemModel::createIndex(TREE_BASE_EVERYBODY, 0, (int)TREE_FLAG_BASE | TREE_BASE_EVERYBODY);
	favoritesGroup = QAbstractItemModel::createIndex(TREE_BASE_FAVORITE, 0, (int)TREE_FLAG_BASE | TREE_BASE_FAVORITE);
	promoGroup = QAbstractItemModel::createIndex(TREE_BASE_PROMO, 0, (int)TREE_FLAG_BASE | TREE_BASE_PROMO);
	rezalGroup = QAbstractItemModel::createIndex(TREE_BASE_REZAL, 0, (int)TREE_FLAG_BASE | TREE_BASE_REZAL);

	//Arborescence favoris/ignoré
	favoriteIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_FAVORITE, 0, (int)TREE_FLAG_FAVORITE | TREE_FAVORITE_FAVORITE);
	ignoredIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_IGNORED, 0, (int)TREE_FLAG_FAVORITE | TREE_FAVORITE_IGNORED);
	neutralIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_NEUTRAL, 0, (int)TREE_FLAG_FAVORITE | TREE_FAVORITE_NEUTRAL);

	//Arborescence par promo
	insertRows(0, 4, promoGroup);
	joneIndex = QAbstractItemModel::createIndex(TREE_PROMO_JONE, 0, (int)TREE_FLAG_PROMO | TREE_PROMO_JONE);
	roujeIndex = QAbstractItemModel::createIndex(TREE_PROMO_ROUJE, 0, (int)TREE_FLAG_PROMO | TREE_PROMO_ROUJE);
	oranjeIndex = QAbstractItemModel::createIndex(TREE_PROMO_ORANJE, 0, (int)TREE_FLAG_PROMO | TREE_PROMO_ORANJE);
	binetIndex = QAbstractItemModel::createIndex(TREE_PROMO_BINET, 0, (int)TREE_FLAG_PROMO | TREE_PROMO_BINET);

	//Arborescence par rezal
	insertRows(0, Rzx::RZL_NUMBER, rezalGroup);
	rezalIndex = new QPersistentModelIndex[Rzx::RZL_NUMBER];
	rezals = new QList<RzxComputer*>[Rzx::RZL_NUMBER];
	rezalsByName = new RzxDict<QString, RzxComputer*>[Rzx::RZL_NUMBER];
	for(int i = 0 ; i < Rzx::RZL_NUMBER ; i++)
		rezalIndex[i] = QAbstractItemModel::createIndex(i, 0, (int)TREE_FLAG_REZAL | i);

	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(update(RzxComputer* )), this, SLOT(update(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(clear()), this, SLOT(clear()));
}

///Détruit le modèle
RzxRezalModel::~RzxRezalModel()
{
}

///Retourne l'index correspondant à l'objet indiqué
QModelIndex RzxRezalModel::index(int row, int column, const QModelIndex& parent) const
{
	//Le parent est invalide, c'est donc la racine...
	//Ses fils sont donc les début de branche :
	//	- Tout le monde... (BASE_EVERYBODY)
	//	- Favoris/Ignored (BASE_FAVORITE)
	//	- Par promo (BASE_PROMO)
	//	- Bâtiments (BASE_REZAL)
	if(!parent.isValid())
		switch(row)
		{
			case TREE_BASE_EVERYBODY: if(!column) return everybodyGroup;
			case TREE_BASE_FAVORITE: if(!column) return favoritesGroup;
			case TREE_BASE_PROMO: if(!column) return promoGroup;
			case TREE_BASE_REZAL: if(!column) return rezalGroup;
			default: return QModelIndex();
		}

	//Si le parent est valide, on filtre sur la catégorie
	//Comme dans la plupart des autres fonctions de cette classe, le seul cas
	//vraiment particulier est le filtrage sur les rezal
	int parentId = parent.internalId();
	int category = parentId & TREE_FLAG_MASK;
	int value = parentId & TREE_FLAG_VALUE;
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
						case TREE_FAVORITE_FAVORITE: if(!column) return favoriteIndex;
						case TREE_FAVORITE_IGNORED: if(!column) return ignoredIndex;
						case TREE_FAVORITE_NEUTRAL: if(!column) return neutralIndex;
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
						case TREE_PROMO_JONE: if(!column) return joneIndex;
						case TREE_PROMO_ROUJE: if(!column) return roujeIndex;
						case TREE_PROMO_ORANJE: if(!column) return oranjeIndex;
						case TREE_PROMO_BINET: if(!column) return binetIndex;
					}
					break;

				//Le père est Rezal, donc row est le rezalId
				//- Rezal (FLAG_REZAL | rezalId)
				case TREE_BASE_REZAL:
					if(row < Rzx::RZL_NUMBER)
						if(!column) return rezalIndex[row];
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
				case TREE_PROMO_BINET: return createIndex(row, column, TREE_FLAG_PROMO_BINET, binet);
			}
			break;

		//Le père est un élément du group Rezal
		// - ...  (FLAG_REZAL | ((rezalId + 1) << 16) | row)
		case TREE_FLAG_REZAL:
		{
			if(value >= 0 && value < Rzx::RZL_NUMBER)
				return createIndex(row, column, GET_ID_FROM_REZAL(value), rezals[value]);
		}
	}

	return QModelIndex();
}

///Retourne l'index du RzxComputer dans le parent indiqué
QModelIndex RzxRezalModel::index(RzxComputer *computer, const QModelIndex& parent) const
{
	if(!parent.isValid()) return QModelIndex();
	const QList<RzxComputer*> *list = NULL;
	switch(parent.internalId())
	{
		case TREE_FLAG_BASE | TREE_BASE_EVERYBODY: list = &everybody; break;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_FAVORITE: list = &favorites; break;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_IGNORED: list = &ignored; break;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_NEUTRAL: list = &neutral; break;
		case TREE_FLAG_PROMO | TREE_PROMO_JONE: list = &jone; break;
		case TREE_FLAG_PROMO | TREE_PROMO_ROUJE: list = &rouje; break;
		case TREE_FLAG_PROMO | TREE_PROMO_ORANJE: list = &oranje; break;
		case TREE_FLAG_PROMO | TREE_PROMO_BINET: list = &binet; break;
		default:
			if((parent.internalId() & TREE_FLAG_MASK) != TREE_FLAG_REZAL) return QModelIndex();
	}
	if((parent.internalId() & TREE_FLAG_MASK) == TREE_FLAG_REZAL)
	{
		int value = parent.internalId() & TREE_FLAG_VALUE;
		list = &rezals[value];
	}
	return index(list->indexOf(computer), 0, parent);
}

///Retourne l'arbre des fils
const RzxDict<QString, RzxComputer*> *RzxRezalModel::childrenByName(const QModelIndex& parent) const
{
	if(!parent.isValid()) return NULL;
	switch(parent.internalId())
	{
		case TREE_FLAG_BASE | TREE_BASE_EVERYBODY: return &everybodyByName;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_FAVORITE: return &favoritesByName;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_IGNORED: return &ignoredByName;
		case TREE_FLAG_FAVORITE | TREE_FAVORITE_NEUTRAL: return &neutralByName;
		case TREE_FLAG_PROMO | TREE_PROMO_JONE: return &joneByName;
		case TREE_FLAG_PROMO | TREE_PROMO_ROUJE: return &roujeByName;
		case TREE_FLAG_PROMO | TREE_PROMO_ORANJE: return &oranjeByName;
		case TREE_FLAG_PROMO | TREE_PROMO_BINET: return &binetByName;
		default:
			if((parent.internalId() & TREE_FLAG_MASK) != TREE_FLAG_REZAL) return NULL;
	}
	if((parent.internalId() & TREE_FLAG_MASK) == TREE_FLAG_REZAL)
	{
		int value = parent.internalId() & TREE_FLAG_VALUE;
		return &rezalsByName[value];
	}
	return NULL;
}

///Retourne le parent
QModelIndex RzxRezalModel::parent(const QModelIndex& parent) const
{
	if(!parent.isValid()) return QModelIndex();
	switch(parent.internalId() & TREE_FLAG_MASK)
	{
		case TREE_FLAG_BASE: return QModelIndex();
		case TREE_FLAG_EVERYBODY: return everybodyGroup;
		case TREE_FLAG_FAVORITE: return favoritesGroup;
		case TREE_FLAG_PROMO: return promoGroup;
		case TREE_FLAG_REZAL: return rezalGroup;
		case TREE_FLAG_FAVORITE_FAVORITE: return favoriteIndex;
		case TREE_FLAG_FAVORITE_IGNORED: return ignoredIndex;
		case TREE_FLAG_FAVORITE_NEUTRAL: return neutralIndex;
		case TREE_FLAG_PROMO_JONE: return joneIndex;
		case TREE_FLAG_PROMO_ROUJE: return roujeIndex;
		case TREE_FLAG_PROMO_ORANJE: return oranjeIndex;
		case TREE_FLAG_PROMO_BINET: return binetIndex;
	}

	//Cas Particulier : les rezal
	if(parent.internalId() & TREE_FLAG_MASK != TREE_FLAG_REZAL)
		return QModelIndex();

	uint rezal = GET_REZAL_FROM_ID(parent.internalId());
	if(rezal < Rzx::RZL_NUMBER)
		return rezalIndex[rezal];

	return QModelIndex();
}

///Indique si l'index a des fils
bool RzxRezalModel::hasChildren(const QModelIndex& parent) const
{
	if(rowCount(parent))
		return true;
	return false;
}

///Retourne le nombre de fils de l'index
int RzxRezalModel::rowCount(const QModelIndex& parent) const
{
	//Les fils de la racine sont BASE_NUMBER
	if(!parent.isValid())
		return TREE_BASE_NUMBER;

	//On extrait de l'identifiant l'indicateur d'arborescence
	int parentId = parent.internalId();
	int category = parentId & TREE_FLAG_MASK;
	int value = parentId & TREE_FLAG_VALUE;
	switch(category)
	{
		case TREE_FLAG_BASE:
			switch(value)
			{
				case TREE_BASE_EVERYBODY: return everybody.count();
				case TREE_BASE_FAVORITE: return TREE_FAVORITE_NUMBER;
				case TREE_BASE_PROMO: return TREE_PROMO_NUMBER;
				case TREE_BASE_REZAL: return Rzx::RZL_NUMBER;
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
				case TREE_PROMO_BINET: return binet.count();
			}
			break;

		case TREE_FLAG_REZAL:
			if(value >= 0 && value < Rzx::RZL_NUMBER)
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

	int parentId = index.internalId();
	int value = parentId & TREE_FLAG_VALUE;
	int column = index.column();
	int category = parentId & TREE_FLAG_MASK;
	
	switch(category)
	{
		case TREE_FLAG_BASE:
			switch(value)
			{
				case TREE_BASE_EVERYBODY: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Everybody"));;
				case TREE_BASE_FAVORITE: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Category"));
				case TREE_BASE_PROMO: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("Promo"));
				case TREE_BASE_REZAL: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), tr("Subnet"));
			}
			break;

		case TREE_FLAG_EVERYBODY: return getComputer(role, everybody, value, column);
		case TREE_FLAG_FAVORITE:
			switch(value)
			{
				case TREE_FAVORITE_FAVORITE: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Favorites"));
				case TREE_FAVORITE_IGNORED: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_BAN), tr("Banned"));
				case TREE_FAVORITE_NEUTRAL: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Others..."));
			}
			break;

		case TREE_FLAG_PROMO:
			switch(value)
			{
				case TREE_PROMO_JONE: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_JONE), tr("Jones"));
				case TREE_PROMO_ROUJE: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_ROUJE), tr("Roujes"));
				case TREE_PROMO_ORANJE: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("Oranjes"));
				case TREE_PROMO_BINET: return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Binet"));
			}
			break;

		case TREE_FLAG_REZAL:
			return getMenuItem(role, RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), RzxComputer::rezalFromId((Rzx::RezalId)value, false));

		case TREE_FLAG_FAVORITE_FAVORITE: return getComputer(role, favorites, value, column);
		case TREE_FLAG_FAVORITE_IGNORED: return getComputer(role, ignored, value, column);
		case TREE_FLAG_FAVORITE_NEUTRAL: return getComputer(role, neutral, value, column);

		case TREE_FLAG_PROMO_JONE: return getComputer(role, jone, value, column);
		case TREE_FLAG_PROMO_ROUJE: return getComputer(role, rouje, value, column);
		case TREE_FLAG_PROMO_ORANJE: return getComputer(role, oranje, value, column);
		case TREE_FLAG_PROMO_BINET: return getComputer(role, binet, value, column);
	}

	if((category & TREE_FLAG_HARDMASK) == TREE_FLAG_REZAL)
	{
		int rezalId = GET_REZAL_FROM_ID(parentId);
		if(rezalId >= 0 && rezalId < Rzx::RZL_NUMBER)
			return getComputer(role, rezals[rezalId], value, column);
	}

	return QString::number(parentId, 16);
}

///Extraction du computer
/** L'extraction se fait après vérification des indexes... c'est d'ailleurs le seul intérêt de cette fonction ;)
 */
QVariant RzxRezalModel::getComputer(int role, const QList<RzxComputer*>& list, int pos, int column) const
{
	
	if(pos < 0 || pos >= list.count())
		return QVariant();
	RzxComputer *computer = list[pos];
	switch(role)
	{
		case Qt::DisplayRole:
			switch(column)
			{
				case ColNom: return computer->name();
				case ColRemarque: return computer->remarque();
				case ColRezal: return computer->rezal();
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
				case ColGateway: return RzxIconCollection::getIcon(computer->isSameGateway()?Rzx::ICON_SAMEGATEWAY:Rzx::ICON_OTHERGATEWAY);
				case ColPromo: return RzxIconCollection::global()->promoIcon(computer->promo());
				case ColOS: return RzxIconCollection::global()->osIcon(computer->sysEx());
				default: return QVariant();
			}

		case Qt::ToolTipRole: return computer->tooltipText();

		case Qt::TextAlignmentRole:
			if(column == ColRemarque) return (int)Qt::AlignLeft | Qt::AlignVCenter;
			return (int)Qt::AlignCenter;

		case Qt::BackgroundColorRole:
			if(computer->state() == Rzx::STATE_AWAY || computer->state() == Rzx::STATE_REFUSE)
				return QColor(RzxConfig::repondeurBase());
			return QVariant();

		case Qt::UserRole: return QVariant::fromValue(computer);
		default: return QVariant();
	}
}

///Extraction de l'objet pour le menu
/** Contrairement aux items pour lesquels le RzxComputer contient la totalité des informations importantes,
 * les menus doivent être définis à la main par l'utilisateur... */
QVariant RzxRezalModel::getMenuItem(int role, const QIcon& icon, const QString& name, const QString& desc) const
{
	QString description = (desc.isNull()?name:desc);

	switch(role)
	{
		case Qt::DisplayRole: return name;
		case Qt::DecorationRole: return icon;
		case Qt::ToolTipRole: return description;
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
				case ColGateway: return RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY);
				case ColPromo: return RzxIconCollection::global()->promoIcon(Rzx::PROMAL_ORANGE);
				default: return QVariant();
			}
		default: return QVariant();
	}
}

///Enregistrement de l'arrivée d'un nouvel élément
void RzxRezalModel::login(RzxComputer *computer)
{
	connect(computer, SIGNAL(update(RzxComputer* )), this, SLOT(update(RzxComputer* )));
	if(everybody.contains(computer))
	{
		update(computer);
		return;
	}

	//Un nouvel objet... donc on le stocke
	insertObject(everybodyGroup, everybody, everybodyByName, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
		insertObject(favoriteIndex, favorites, favoritesByName, computer);
	else if(computer->isIgnored())
		insertObject(ignoredIndex, ignored, ignoredByName, computer);
	else
		insertObject(neutralIndex, neutral, neutralByName, computer);

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			if(computer->rezalId() == Rzx::RZL_BINETS || computer->rezalId() == Rzx::RZL_BR)
				insertObject(binetIndex, binet, binetByName, computer);
			else
				insertObject(oranjeIndex, oranje, oranjeByName, computer);
			break;
		case Rzx::PROMAL_JONE:
			insertObject(joneIndex, jone, joneByName, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			insertObject(roujeIndex, rouje, roujeByName, computer);
			break;
	}

	//Rangement en rezal
	Rzx::RezalId rezalId = computer->rezalId();
	insertObject(rezalIndex[rezalId], rezals[rezalId], rezalsByName[rezalId], computer);
}

///Un computer vient de se déconnecter...
void RzxRezalModel::logout(RzxComputer *computer)
{
	//Un objet est supprimé :/
	removeObject(everybodyGroup, everybody, everybodyByName, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
		removeObject(favoriteIndex, favorites, favoritesByName, computer);
	else if(computer->isIgnored())
		removeObject(ignoredIndex, ignored, ignoredByName, computer);
	else
		removeObject(neutralIndex, neutral, neutralByName, computer);

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			if(computer->rezalId() == Rzx::RZL_BINETS || computer->rezalId() == Rzx::RZL_BR)
				removeObject(binetIndex, binet, binetByName, computer);
			else
				removeObject(oranjeIndex, oranje, oranjeByName, computer);
			break;
		case Rzx::PROMAL_JONE:
			removeObject(joneIndex, jone, joneByName, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			removeObject(roujeIndex, rouje, roujeByName, computer);
			break;
	}

	//Rangement en rezal
	Rzx::RezalId rezalId = computer->rezalId();
	removeObject(rezalIndex[rezalId], rezals[rezalId], rezalsByName[rezalId], computer);
}

///Un computer vient de mettre à jours les informations qui le concerne
void RzxRezalModel::update(RzxComputer *computer)
{
	updateObject(everybodyGroup, everybody, computer);

	//Tri entre favoris, ban...
	if(computer->isFavorite())
	{
		if(!favorites.contains(computer))
		{
			removeObject(neutralIndex, neutral, neutralByName, computer);
			insertObject(favoriteIndex, favorites, favoritesByName, computer);
			removeObject(ignoredIndex, ignored, ignoredByName, computer);
		}
		else
			updateObject(favoriteIndex, favorites, computer);
	}
	else if(computer->isIgnored())
	{
		if(!ignored.contains(computer))
		{
			removeObject(neutralIndex, neutral, neutralByName, computer);
			removeObject(favoriteIndex, favorites, favoritesByName, computer);
			insertObject(ignoredIndex, ignored, ignoredByName, computer);
		}
		else
			updateObject(ignoredIndex, ignored, computer);
	}
	else
	{
		if(!neutral.contains(computer))
		{
			insertObject(neutralIndex, neutral, neutralByName, computer);
			removeObject(favoriteIndex, favorites, favoritesByName, computer);
			removeObject(ignoredIndex, ignored, ignoredByName, computer);
		}
		else
			updateObject(neutralIndex, neutral, computer);
	}

	//Promo
	switch(computer->promo())
	{
		case Rzx::PROMAL_UNK: case Rzx::PROMAL_ORANGE:
			if(computer->rezalId() == Rzx::RZL_BINETS || computer->rezalId() == Rzx::RZL_BR)
			{
				if(!binet.contains(computer))
				{
					removeObject(roujeIndex, rouje, roujeByName, computer);
					insertObject(binetIndex, binet, binetByName, computer);
					removeObject(oranjeIndex, oranje, oranjeByName, computer);
					removeObject(joneIndex, jone, joneByName, computer);
				}
				else
					updateObject(binetIndex, binet, computer);
			}
			else
			{
				if(!oranje.contains(computer))
				{
					removeObject(roujeIndex, rouje, roujeByName, computer);
					removeObject(binetIndex, binet, binetByName, computer);
					insertObject(oranjeIndex, oranje, oranjeByName, computer);
					removeObject(joneIndex, jone, joneByName, computer);
				}
				else
					updateObject(oranjeIndex, oranje, computer);
			}
			break;
		case Rzx::PROMAL_JONE:
			if(!jone.contains(computer))
			{
				removeObject(roujeIndex, rouje, roujeByName, computer);
				removeObject(binetIndex, binet, binetByName, computer);
				removeObject(oranjeIndex, oranje, oranjeByName, computer);
				insertObject(joneIndex, jone, joneByName, computer);
			}
			else
				updateObject(joneIndex, jone, computer);
			break;
		case Rzx::PROMAL_ROUJE:
			if(!rouje.contains(computer))
			{
				insertObject(roujeIndex, rouje, roujeByName, computer);
				removeObject(binetIndex, binet, binetByName, computer);
				removeObject(oranjeIndex, oranje, oranjeByName, computer);
				removeObject(joneIndex, jone, joneByName, computer);
			}
			else
				updateObject(roujeIndex, rouje, computer);
			break;
	}

	//Rangement en rezal
	int rezalId = computer->rezalId();
	if(!rezals[rezalId].contains(computer))
	{
		for(int i = 0 ; i < Rzx::RZL_NUMBER ; i++)
			if(i == rezalId)
				insertObject(rezalIndex[i], rezals[i], rezalsByName[i], computer);
			else
				removeObject(rezalIndex[i], rezals[i], rezalsByName[i], computer);
	}
	else
		updateObject(rezalIndex[rezalId], rezals[rezalId], computer);
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
	deleteGroup(everybody, everybodyGroup);
	deleteGroup(favorites, favoriteIndex);
	deleteGroup(ignored, ignoredIndex);
	deleteGroup(neutral, neutralIndex);
	deleteGroup(jone, joneIndex);
	deleteGroup(rouje, roujeIndex);
	deleteGroup(oranje, oranjeIndex);
	deleteGroup(binet, binetIndex);
	for(int i = 0 ; i < Rzx::RZL_NUMBER ; i++)
		deleteGroup(rezals[i], rezalIndex[i]);
}


///Insertion d'un objet dans la liste et le groupe correspondant
void RzxRezalModel::insertObject(const QModelIndex& parent, QList<RzxComputer*>& list, RzxDict<QString, RzxComputer*>& tree, RzxComputer *computer)
{
	int row = list.count();
	beginInsertRows(parent, row, row);
	list << computer;
	tree.insert(computer->name(), &computer);
	endInsertRows();
}

///Suppression d'un objet de la liste et du groupe correspondant
void RzxRezalModel::removeObject(const QModelIndex& parent, QList<RzxComputer*>& list, RzxDict<QString, RzxComputer*>& tree, RzxComputer *computer)
{
	int row = list.indexOf(computer);
	if(row == -1) return;
	beginRemoveRows(parent, row, row);
	list.removeAll(computer);
	tree.remove(computer->name());
	endRemoveRows();
}

///Mise à jours des données concernant un object
void RzxRezalModel::updateObject(const QModelIndex& parent, const QList<RzxComputer*>& list, RzxComputer *computer)
{
	int row = list.indexOf(computer);
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

	QVariant value = data(ref, Qt::UserRole);
#ifndef Q_OS_MAC
	if(!value.canConvert<RzxComputer*>())
		return QModelIndexList() << ref;
#endif

	QModelIndexList indexList;
	RzxComputer *computer = value.value<RzxComputer*>();
	insert(everybody, everybodyGroup);
	insert(favorites, favoriteIndex);
	insert(ignored, ignoredIndex);
	insert(neutral, neutralIndex);
	insert(jone, joneIndex);
	insert(rouje, roujeIndex);
	insert(oranje, oranjeIndex);
	insert(binet, binetIndex);
	for(int i = 0 ; i<Rzx::RZL_NUMBER ; i++)
		insert(rezals[i], rezalIndex[i]);
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

	sortList(everybody, everybodyGroup);
	sortList(favorites, favoriteIndex);
	sortList(ignored, ignoredIndex);
	sortList(neutral, neutralIndex);
	sortList(jone, joneIndex);
	sortList(rouje, roujeIndex);
	sortList(oranje, oranjeIndex);
	sortList(binet, binetIndex);
	for(int i=0 ; i < Rzx::RZL_NUMBER ; i++)
		sortList(rezals[i], rezalIndex[i]);
#undef sortList
}

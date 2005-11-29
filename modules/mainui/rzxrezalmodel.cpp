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
	if(RzxRezalModel::global()->sens != Qt::AscendingOrder)
		qSwap(c1, c2);
	if(!c1) return false;
	if(!c2) return true;
	switch(order)
	{
		case RzxRezalModel::ColNom: return c1->name().toLower() < c2->name().toLower();
		case RzxRezalModel::ColRemarque: return c1->remarque().toLower() < c2->remarque().toLower();
		case RzxRezalModel::ColSamba: return c1->hasSambaServer() && !c2->hasSambaServer();
		case RzxRezalModel::ColFTP: return c1->hasFtpServer() && !c2->hasFtpServer();
		case RzxRezalModel::ColHTTP: return c1->hasHttpServer() && !c2->hasHttpServer();
		case RzxRezalModel::ColNews: return c1->hasNewsServer() && !c2->hasNewsServer();
		case RzxRezalModel::ColOS: return c1->sysEx() < c2->sysEx();
		case RzxRezalModel::ColGateway: return c1->isSameGateway(c2);
		case RzxRezalModel::ColPromo: return c1->promo() < c2->promo();
		case RzxRezalModel::ColRezal: return c1->rezal() < c2->rezal();
		case RzxRezalModel::ColIP: return (qint32)c1->ip() < (qint32)c2->ip();
		case RzxRezalModel::ColClient: return c1->client() < c2->client();
		default: return false;
	}
}

///Construction du Model
RzxRezalModel::RzxRezalModel()
{
	//Base de l'arbre
	insertRows(0, 4);
	everybodyGroup = QAbstractItemModel::createIndex(TREE_BASE_EVERYBODY, 0, (int)TREE_FLAG_BASE);
	favoritesGroup = QAbstractItemModel::createIndex(TREE_BASE_FAVORITE, 0, (int)TREE_FLAG_BASE);
	promoGroup = QAbstractItemModel::createIndex(TREE_BASE_PROMO, 0, (int)TREE_FLAG_BASE);
	rezalGroup = QAbstractItemModel::createIndex(TREE_BASE_REZAL, 0, (int)TREE_FLAG_BASE);

	//Arborescence favoris/ignoré
	favoriteIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_FAVORITE, 0, (int)TREE_FLAG_FAVORITE);
	ignoredIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_IGNORED, 0, (int)TREE_FLAG_FAVORITE);
	neutralIndex = QAbstractItemModel::createIndex(TREE_FAVORITE_NEUTRAL, 0, (int)TREE_FLAG_FAVORITE);

	//Arborescence par promo
	insertRows(0, 4, promoGroup);
	joneIndex = QAbstractItemModel::createIndex(TREE_PROMO_JONE, 0, (int)TREE_FLAG_PROMO);
	roujeIndex = QAbstractItemModel::createIndex(TREE_PROMO_ROUJE, 0, (int)TREE_FLAG_PROMO);
	oranjeIndex = QAbstractItemModel::createIndex(TREE_PROMO_ORANJE, 0, (int)TREE_FLAG_PROMO);
	
	//Initialisation de l'ordre de tri
	order = (NumColonne)RzxMainUIConfig::sortColumn();
	sens = RzxMainUIConfig::sortOrder();

	//Arborescence par rezal
	insertRows(0, RzxConfig::rezalNumber(), rezalGroup);
	rezalIndex = new QPersistentModelIndex[RzxConfig::rezalNumber()];
	rezals = new RzxRezalSearchList[RzxConfig::rezalNumber()];
	rezalsByName = new RzxRezalSearchTree[RzxConfig::rezalNumber()];
	for(uint i = 0 ; i < RzxConfig::rezalNumber() ; i++)
		rezalIndex[i] = QAbstractItemModel::createIndex(i, 0, (int)TREE_FLAG_REZAL);

	object = this;
	QList<RzxComputer*> computers = RzxConnectionLister::global()->computerList();
	foreach(RzxComputer *computer, computers)
		login(computer);

	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(logout(RzxComputer* )), this, SLOT(logout(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(update(RzxComputer* )), this, SLOT(update(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(clear()), this, SLOT(clear()));
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
	//Le parent est invalide, c'est donc la racine...
	//Ses fils sont donc les début de branche :
	//	- Tout le monde... (BASE_EVERYBODY)
	//	- Favoris/Ignored (BASE_FAVORITE)
	//	- Par promo (BASE_PROMO)
	//	- Bâtiments (BASE_REZAL)
	if(!parent.isValid())
		switch(row)
		{
			case TREE_BASE_EVERYBODY: if(!column) return everybodyGroup; break;
			case TREE_BASE_FAVORITE: if(!column) return favoritesGroup; break;
			case TREE_BASE_PROMO: if(!column) return promoGroup; break;
			case TREE_BASE_REZAL: if(!column) return rezalGroup; break;
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
						case TREE_FAVORITE_FAVORITE: if(!column) return favoriteIndex; break;
						case TREE_FAVORITE_IGNORED: if(!column) return ignoredIndex; break;
						case TREE_FAVORITE_NEUTRAL: if(!column) return neutralIndex; break;
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
						case TREE_PROMO_JONE: if(!column) return joneIndex; break;
						case TREE_PROMO_ROUJE: if(!column) return roujeIndex; break;
						case TREE_PROMO_ORANJE: if(!column) return oranjeIndex; break;
					}
					break;

				//Le père est Rezal, donc row est le rezalId
				//- Rezal (FLAG_REZAL | rezalId)
				case TREE_BASE_REZAL:
					if(row < (int)RzxConfig::rezalNumber())
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
	}

	//Cas Particulier : les rezal
	if((index.internalId() & TREE_FLAG_HARDMASK) == TREE_FLAG_REZAL)
	{
		uint rezal = GET_REZAL_FROM_ID(index.internalId());
		if(rezal < RzxConfig::rezalNumber())
			return rezalIndex[rezal];
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
			switch(value)
			{
				case TREE_FAVORITE_FAVORITE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Favorites"));
				case TREE_FAVORITE_IGNORED: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_BAN), tr("Banned"));
				case TREE_FAVORITE_NEUTRAL: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Others..."));
			}
			break;

		case TREE_FLAG_PROMO:
			switch(value)
			{
				case TREE_PROMO_JONE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_JONE), tr("Jones"));
				case TREE_PROMO_ROUJE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_ROUJE), tr("Roujes"));
				case TREE_PROMO_ORANJE: return getMenuItem(role, child, RzxIconCollection::getIcon(Rzx::ICON_ORANJE), tr("Oranjes"));
			}
			break;

		case TREE_FLAG_REZAL:
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
 	tooltip += "<b><i>" + tr("Informations :") + "</b></i><br/>";
	
	if(computer->hasFtpServer() && (tooltipFlags & (int)RzxRezalModel::TipFtp))
		tooltip += "<b>-></b>&nbsp;" + tr("ftp server : ") + tr("<b>on</b>") + "<br/>";
	if(computer->hasHttpServer() && (tooltipFlags & (int)RzxRezalModel::TipHttp))
		tooltip += "<b>-></b>&nbsp;" + tr("web server : ") + tr("<b>on</b>") + "<br/>";
	if(computer->hasNewsServer() && (tooltipFlags & (int)RzxRezalModel::TipNews))
		tooltip += "<b>-></b>&nbsp;" + tr("news server : ") + tr("<b>on</b>") + "<br/>";
	if(computer->hasSambaServer() && (tooltipFlags & (int)RzxRezalModel::TipSamba))
		tooltip += "<b>-></b>&nbsp;" + tr("samba server : ") + tr("<b>on</b>") + "<br/>";
	if(computer->hasPrinter() && (tooltipFlags & (int)RzxRezalModel::TipPrinter))
		tooltip += "<b>-></b>&nbsp;" + tr("printer : ") + tr("<b>yes</b>") + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipOS)
		tooltip += "<b>-></b>&nbsp;os : " + computer->sysExText() + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipClient)
		tooltip += "<b>-></b>&nbsp;" + computer->client() + "<br/>";
	if(tooltipFlags & (int)RzxRezalModel::TipFeatures)
	{
		if(computer->can(Rzx::CAP_ON))
		{
			int nb = 0;
			tooltip += "<b>-></b>&nbsp;" + tr("features : ");
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
		tooltip += "<b>-></b>&nbsp;" + tr("Location : ") + computer->rezalName(false) + "<br/>";
	
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
/*			if(computer->rezal() == Rzx::RZL_BINETS || computer->rezal() == Rzx::RZL_BR)
				insertObject(binetIndex, binet, binetByName, computer);
			else*/
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
	int rezalId = computer->rezal();
	if(rezalId >= 0 && rezalId < (int)RzxConfig::rezalNumber())
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
/*			if(computer->rezal() == Rzx::RZL_BINETS || computer->rezal() == Rzx::RZL_BR)
				removeObject(binetIndex, binet, binetByName, computer);
			else*/
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
	int rezalId = computer->rezal();
	if(rezalId >= 0 && rezalId < (int)RzxConfig::rezalNumber())
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
			if(!oranje.contains(computer))
			{
				removeObject(roujeIndex, rouje, roujeByName, computer);
				insertObject(oranjeIndex, oranje, oranjeByName, computer);
				removeObject(joneIndex, jone, joneByName, computer);
			}
			else
				updateObject(oranjeIndex, oranje, computer);
			break;
		case Rzx::PROMAL_JONE:
			if(!jone.contains(computer))
			{
				removeObject(roujeIndex, rouje, roujeByName, computer);
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
				removeObject(oranjeIndex, oranje, oranjeByName, computer);
				removeObject(joneIndex, jone, joneByName, computer);
			}
			else
				updateObject(roujeIndex, rouje, computer);
			break;
	}

	//Rangement en rezal
	uint rezalId = computer->rezal();
	if(rezalId < RzxConfig::rezalNumber() && !rezals[rezalId].contains(computer))
	{
		for(uint i = 0 ; i < RzxConfig::rezalNumber() ; i++)
			if(i == rezalId)
				insertObject(rezalIndex[i], rezals[i], rezalsByName[i], computer);
			else
				removeObject(rezalIndex[i], rezals[i], rezalsByName[i], computer);
	}
	else if(rezalId < RzxConfig::rezalNumber())
		updateObject(rezalIndex[rezalId], rezals[rezalId], computer);
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
	deleteGroup(everybody, everybodyGroup);
	deleteGroup(favorites, favoriteIndex);
	deleteGroup(ignored, ignoredIndex);
	deleteGroup(neutral, neutralIndex);
	deleteGroup(jone, joneIndex);
	deleteGroup(rouje, roujeIndex);
	deleteGroup(oranje, oranjeIndex);
	for(uint i = 0 ; i < RzxConfig::rezalNumber() ; i++)
		deleteGroup(rezals[i], rezalIndex[i]);
}


///Insertion d'un objet dans la liste et le groupe correspondant
void RzxRezalModel::insertObject(const QModelIndex& parent, RzxRezalSearchList& list, RzxRezalSearchTree& tree, RzxComputer *computer)
{
	list << computer;
	qSort(list.begin(), list.end(), sortComputer);
	const int row = list.indexOf(computer);
	beginInsertRows(parent, row, row);
	tree.insert(computer->name().toLower(), new QPointer<RzxComputer>(computer));
	endInsertRows();
}

///Suppression d'un objet de la liste et du groupe correspondant
void RzxRezalModel::removeObject(const QModelIndex& parent, RzxRezalSearchList& list, RzxRezalSearchTree& tree, RzxComputer *computer)
{
	const int row = list.indexOf(computer);
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
	insert(everybody, everybodyGroup);
	insert(favorites, favoriteIndex);
	insert(ignored, ignoredIndex);
	insert(neutral, neutralIndex);
	insert(jone, joneIndex);
	insert(rouje, roujeIndex);
	insert(oranje, oranjeIndex);
	for(uint i = 0 ; i<RzxConfig::rezalNumber() ; i++)
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
	for(uint i=0 ; i < RzxConfig::rezalNumber() ; i++)
		sortList(rezals[i], rezalIndex[i]);
#undef sortList
}

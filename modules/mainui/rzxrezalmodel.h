/***************************************************************************
                          rzxrezalmodel  -  description
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
#ifndef RZXREZALMODEL_H
#define RZXREZALMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QList>
#include <QIcon>
#include <QString>
#include <QPointer>

#include <RzxGlobal>
#include <RzxSubnet>

#include "rzxmainuiglobal.h"
#include "rzxdict.h"

class RzxComputer;
typedef class RzxDict<QString, QPointer<RzxComputer> > RzxRezalSearchTree;
typedef class QList< QPointer<RzxComputer> > RzxRezalSearchList;
/**
 *@author Florent Bruneau
 */

///Model de base de l'affichage de qRezix
/** Ce modèle utilise les RzxComputer comme source de données. Le stockage est réalisé de manière à pouvoir réaliser des accès
 * très puissants à une catégorie uniquement de machines. Ainsi, on a les types suivants de tri :
 * 	- Par vrac :) en gros on met tout le monde
 * 	- Par affinité : on tri les favoris et les ignorés
 * 	- Par Promo
 * 	- Par état de connexion
 * 	- Par bâtiment (enfin par Rezal)
 * 
 * Le stockage interne des informations aura donc une arborescence de la forme :
 * 	- Tout le monde... (FLAG_BASE | BASE_EVERYBODY)
 * 		- ... (FLAG_EVERYBODY | row)
 * 	- Favoris/Ignored (FLAG_BASE | BASE_FAVORITE)
 * 		- Favoris (FLAG_FAVORITE | FAVORITE_FAVORITE)
 * 			- ... (FLAG_FAVORITE_FAVORITE | row)
 * 		- Ignored (FLAG_FAVORITE | FAVORITE_IGNORED)
 * 			- ... (FLAG_FAVORITE_IGNORED | row)
 * 		- Autres (FLAG_FAVORITE | FAVORITE_NEUTRAL)
 * 			- ... (FLAG_FAVORITE_NEUTRAL | row)
 * 	- Par promo (FLAG_BASE | BASE_PROMO)
 * 		- Jônes (FLAG_PROMO | PROMO_JONE)
 * 			- ... (FLAG_PROMO_JONE | row)
 * 		- Roujes (FLAG_PROMO | PROMO_ROUJE)
 * 			- ... (FLAG_PROMO_ROUJE | row)
 * 		- Oranjes (FLAG_PROMO | PROMO_ORANJE)
 * 			- ... (FLAG_PROMO_ORANJE | row)
 * 		- Binets si oranje en rezalId = binet ou BR (FLAG_PROMO | PROMO_BINET)
 * 			- ... (FLAG_PROMO_BINET | row)
 * 	- Bâtiments (FLAG_BASE | BASE_REZAL)
 * 		- Rezal (FLAG_REZAL | rezalId)
 * 			- ... (FLAG_REZAL | ((rezalId + 1) << 16) | row)
 *
 * Chacune de ces catégories stockent tout le monde 1 fois... donc la charge mémoire du modèle est de l'ordre de 
 * 4*4*nbconnectés = 8ko avec 500 connectés, ce qui est très raisonnable. 
 */
class RZX_MAINUI_EXPORT RzxRezalModel:public QAbstractItemModel
{
	Q_OBJECT
	Q_ENUMS(NumColonne ToolTipFlags)
	Q_FLAGS(ToolTip)
	RZX_GLOBAL(RzxRezalModel)

	///Construction de l'objet
	RzxRezalModel();

	public:
		///Identifiant les colonnes
		enum NumColonne {
			ColNom = 0,
			ColRemarque = 1,
			ColSamba = 2,
			ColFTP = 3,
			ColHTTP = 4,
			ColNews = 5,
			ColPrinter = 6,
			ColOS = 7,
			ColGateway = 8,
			ColPromo = 9,
			ColRezal = 10,
			ColIP = 11,
			ColClient = 12,
			numColonnes = 13
		};

		///Flags des éléments des tooltips
		enum ToolTipFlags
		{
			TipEnable = 1,
			TipFtp = 2,
			TipHttp = 4,
			TipNews = 8,
			TipSamba = 16,
			TipPromo = 32,
			TipOS = 64,
			TipClient = 128,
			TipIP = 256,
			TipResal = 512,
			TipFeatures = 1024,
			TipProperties = 2048,
			TipPrinter = 4092
		};
		Q_DECLARE_FLAGS(ToolTip, ToolTipFlags)

	private:
		///Nom des colonnes
		static const char *colNames[numColonnes];

		//Stockage des objets
		RzxRezalSearchList everybody;
		RzxRezalSearchList favorites;
		RzxRezalSearchList ignored;
		RzxRezalSearchList neutral;
		RzxRezalSearchList jone;
		RzxRezalSearchList rouje;
		RzxRezalSearchList oranje;
		RzxRezalSearchList *rezals;

		RzxRezalSearchTree everybodyByName;
		RzxRezalSearchTree favoritesByName;
		RzxRezalSearchTree ignoredByName;
		RzxRezalSearchTree neutralByName;
		RzxRezalSearchTree joneByName;
		RzxRezalSearchTree roujeByName;
		RzxRezalSearchTree oranjeByName;
		RzxRezalSearchTree *rezalsByName;

		///Ordre utilisé pour trier
		NumColonne order;
		Qt::SortOrder sens;

		friend bool sortComputer(RzxComputer *, RzxComputer *);

	protected:
		///Identifiants des objets de base de l'arborescence
		enum TreeBase {
			TREE_BASE_EVERYBODY = 0,
			TREE_BASE_FAVORITE = 1,
			TREE_BASE_PROMO = 2,
			TREE_BASE_REZAL = 3,
			TREE_BASE_NUMBER = 4
		};

		///Identifiants des fils du groupe Favoris/Ignorés
		enum TreeFavorite {
			TREE_FAVORITE_FAVORITE = 0,
			TREE_FAVORITE_IGNORED = 1,
			TREE_FAVORITE_NEUTRAL = 2,
			TREE_FAVORITE_NUMBER = 3
		};

		///Identifiants des fils du groupe Promo
		enum TreePromo {
			TREE_PROMO_JONE = 0,
			TREE_PROMO_ROUJE = 1,
			TREE_PROMO_ORANJE = 2,
			TREE_PROMO_NUMBER = 3
		};

		///Flags permettant d'identifier les identifiant des objets
		enum TreeFlags {
			TREE_FLAG_BASE 		= 0x00000000, 	/**< Flag permettant d'identifier les objets du groupe base */
			TREE_FLAG_EVERYBODY	= 0x00010000,	/**< Flag permettant d'identifier les objets du groupe Tout le monde */
			TREE_FLAG_FAVORITE 	= 0x00020000, 	/**< Flag permettant d'identifier les objets du groupe Favoris */
			TREE_FLAG_FAVORITE_FAVORITE	= 0x00020001,
			TREE_FLAG_FAVORITE_IGNORED		= 0x00020002,
			TREE_FLAG_FAVORITE_NEUTRAL		= 0x00020003,
			TREE_FLAG_PROMO 		= 0x00030000, 	/**< Flag permettant d'identifier les objets du groupe Promo */
			TREE_FLAG_PROMO_JONE		= 0x00030001,
			TREE_FLAG_PROMO_ROUJE	= 0x00030002,
			TREE_FLAG_PROMO_ORANJE	= 0x00030003,
			TREE_FLAG_PROMO_BINET	= 0x00030004,
			TREE_FLAG_REZAL 		= 0x00040000, 	/**< Flag permettant d'identifier les objets du groupe Rezal */
			TREE_FLAG_HARDMASK	= 0xffff0000,	/**< Masque qui isole la catégorie et la sous-catérogie */
			TREE_FLAG_SOFTMASK	= 0x0000ffff,	/**< Masque qui isole la sous-catégorie de la catégorie */
		};

#define GET_REZAL_FROM_ID(id) ((id & TREE_FLAG_SOFTMASK) - 1)
#define GET_ID_FROM_REZAL(rezal) (int)(TREE_FLAG_REZAL | ((uint)(rezal + 1) & TREE_FLAG_SOFTMASK))

		///Insertion d'un objet dans le groupe et la liste donnée
		void insertObject(const QModelIndex&, RzxRezalSearchList&, RzxRezalSearchTree&, RzxComputer*);
		///Suppression d'un objet d'un groupe et de la liste donnée
		void removeObject(const QModelIndex&, RzxRezalSearchList&, RzxRezalSearchTree&, RzxComputer*);
		///Indication de modification d'un objet
		void updateObject(const QModelIndex&, const RzxRezalSearchList&, RzxComputer*);

		///Construction d'un index associé à l'objet
		QModelIndex createIndex(int, int, int, const RzxRezalSearchList&) const;

		///Extrait le RzxComputer qui correspond
		virtual QVariant getComputer(int, const RzxRezalSearchList&, int, int) const;

		///Extrait l'item d'un menu
		virtual QVariant getMenuItem(int, int, const QIcon&, const QString&, const QString& desc = QString()) const;

	public:
		//Modèles qui servent de repère pour les entrées
		QPersistentModelIndex everybodyGroup; //fils directs
		QPersistentModelIndex favoritesGroup;
		QPersistentModelIndex favoriteIndex, ignoredIndex, neutralIndex;
		QPersistentModelIndex promoGroup;
		QPersistentModelIndex joneIndex, roujeIndex, oranjeIndex;
		QPersistentModelIndex rezalGroup;
		QPersistentModelIndex *rezalIndex;

	public:
		virtual ~RzxRezalModel();

		virtual QModelIndex index(int, int, const QModelIndex& index = QModelIndex()) const;
		virtual QModelIndex index(RzxComputer*, const QModelIndex& index = QModelIndex()) const;
		virtual QModelIndex index(const RzxSubnet&) const;
		virtual QModelIndex parent(const QModelIndex&) const;
		virtual bool isIndex(const QModelIndex&) const;
		virtual int rezal(const QModelIndex&) const;
		
		virtual int rowCount(const QModelIndex&) const;
		virtual int columnCount(const QModelIndex&) const;

		virtual bool hasChildren(const QModelIndex&) const;
		virtual QVariant data(const QModelIndex&, int) const;

		virtual QModelIndexList selected(const QModelIndex&) const;

		virtual QVariant headerData(int, Qt::Orientation, int) const;

		virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

		virtual const RzxRezalSearchTree *childrenByName(const QModelIndex&) const;
		virtual QString columnName(NumColonne);

	public slots:
		virtual void login(RzxComputer *);
		virtual void logout(RzxComputer *);
		virtual void update(RzxComputer *);
		virtual void clear();

	protected:
		virtual QString tooltip(const RzxComputer *) const;
};

///Création d'un index associé à un objet
/** Permet la création d'un QModelIndex associé à un RzxComputer. Ce RzxComputer est indentifé par deux objets :
 * 	- la liste des Computers associés au flag
 * 	- l'index du computer dans la liste
 *
 * On peut donc vérifier que l'objet existe, générer sont index (flag | row) et retourner l'index correspondant.
 */
inline QModelIndex RzxRezalModel::createIndex(int row, int column, int flag, const RzxRezalSearchList& list) const
{
	if(row < 0 || row >= list.count())
		return QModelIndex();
	
	return QAbstractItemModel::createIndex(row, column, flag);
}

#endif

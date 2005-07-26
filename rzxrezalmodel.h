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

#include "rzxdict.h"

class RzxComputer;
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
class RzxRezalModel:public QAbstractItemModel
{
	Q_OBJECT
	Q_ENUMS(NumColonne)

	///Object statique pour une utilisation globale
	static RzxRezalModel *object;

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
			ColOS = 6,
			ColGateway = 7,
			ColPromo = 8,
			ColRezal = 9,
			ColIP = 10,
			ColClient = 11,
			numColonnes = 12
		};

	private:
		///Nom des colonnes
		static const char *colNames[numColonnes];

		//Stockage des objets
		QList<RzxComputer*> everybody;
		QList<RzxComputer*> favorites;
		QList<RzxComputer*> ignored;
		QList<RzxComputer*> neutral;
		QList<RzxComputer*> jone;
		QList<RzxComputer*> rouje;
		QList<RzxComputer*> oranje;
		QList<RzxComputer*> binet;
		QList<RzxComputer*> *rezals;

		RzxDict<QString, RzxComputer*> everybodyByName;
		RzxDict<QString, RzxComputer*> favoritesByName;
		RzxDict<QString, RzxComputer*> ignoredByName;
		RzxDict<QString, RzxComputer*> neutralByName;
		RzxDict<QString, RzxComputer*> joneByName;
		RzxDict<QString, RzxComputer*> roujeByName;
		RzxDict<QString, RzxComputer*> oranjeByName;
		RzxDict<QString, RzxComputer*> binetByName;
		RzxDict<QString, RzxComputer*> *rezalsByName;

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
			TREE_PROMO_BINET = 3,
			TREE_PROMO_NUMBER = 4
		};

		///Flags permettant d'identifier les identifiant des objets
		enum TreeFlags {
			TREE_FLAG_BASE 		= 0x00000000, 	/**< Flag permettant d'identifier les objets du groupe base */
			TREE_FLAG_EVERYBODY	= 0x01000000,	/**< Flag permettant d'identifier les objets du groupe Tout le monde */
			TREE_FLAG_FAVORITE 	= 0x02000000, 	/**< Flag permettant d'identifier les objets du groupe Favoris */
			TREE_FLAG_FAVORITE_FAVORITE	= 0x02010000,
			TREE_FLAG_FAVORITE_IGNORED		= 0x02020000,
			TREE_FLAG_FAVORITE_NEUTRAL		= 0x02030000,
			TREE_FLAG_PROMO 		= 0x03000000, 	/**< Flag permettant d'identifier les objets du groupe Promo */
			TREE_FLAG_PROMO_JONE		= 0x03010000,
			TREE_FLAG_PROMO_ROUJE	= 0x03020000,
			TREE_FLAG_PROMO_ORANJE	= 0x03030000,
			TREE_FLAG_PROMO_BINET	= 0x03040000,
			TREE_FLAG_REZAL 		= 0x04000000, 	/**< Flag permettant d'identifier les objets du groupe Rezal */
			TREE_FLAG_MASK 		= 0xffff0000, 	/**< Masque permettant d'extraire le flag de catégorie */
			TREE_FLAG_HARDMASK	= 0xff000000,	/**< Masque qui isole la catégorie et la sous-catérogie */
			TREE_FLAG_SOFTMASK	= 0x00ff0000,	/**< Masque qui isole la sous-catégorie de la catégorie */
			TREE_FLAG_VALUE		= 0x0000ffff	/**< Masque permettant d'extraire la valeur de l'identifiant*/
		};

#define GET_REZAL_FROM_ID(id) (((id & TREE_FLAG_SOFTMASK) >> 16) - 1)
#define GET_ID_FROM_REZAL(rezal) (((rezal + 1) << 16) | TREE_FLAG_REZAL)

		///Insertion d'un objet dans le groupe et la liste donnée
		void insertObject(const QModelIndex&, QList<RzxComputer*>&, RzxDict<QString, RzxComputer*>&, RzxComputer*);
		///Suppression d'un objet d'un groupe et de la liste donnée
		void removeObject(const QModelIndex&, QList<RzxComputer*>&, RzxDict<QString, RzxComputer*>&, RzxComputer*);
		///Indication de modification d'un objet
		void updateObject(const QModelIndex&, const QList<RzxComputer*>&, RzxComputer*);

		///Construction d'un index associé à l'objet
		QModelIndex createIndex(int, int, int, const QList<RzxComputer*>&) const;

		///Extrait le RzxComputer qui correspond
		virtual QVariant getComputer(int, const QList<RzxComputer*>&, int, int) const;

		///Extrait l'item d'un menu
		virtual QVariant getMenuItem(int, const QIcon&, const QString&, const QString& desc = QString()) const;

	public:
		//Modèles qui servent de repère pour les entrées
		QPersistentModelIndex everybodyGroup; //fils directs
		QPersistentModelIndex favoritesGroup;
		QPersistentModelIndex favoriteIndex, ignoredIndex, neutralIndex;
		QPersistentModelIndex promoGroup;
		QPersistentModelIndex joneIndex, roujeIndex, oranjeIndex, binetIndex;
		QPersistentModelIndex rezalGroup;
		QPersistentModelIndex *rezalIndex;

	public:
		virtual ~RzxRezalModel();

		static RzxRezalModel *global();

		virtual QModelIndex index(int, int, const QModelIndex& index = QModelIndex()) const;
		virtual QModelIndex index(RzxComputer*, const QModelIndex& index = QModelIndex()) const;
		virtual QModelIndex parent(const QModelIndex&) const;
		
		virtual int rowCount(const QModelIndex&) const;
		virtual int columnCount(const QModelIndex&) const;

		virtual bool hasChildren(const QModelIndex&) const;
		virtual QVariant data(const QModelIndex&, int) const;

		virtual QModelIndexList selected(const QModelIndex&) const;

		virtual QVariant headerData(int, Qt::Orientation, int) const;

		virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

		virtual const RzxDict<QString, RzxComputer*> *childrenByName(const QModelIndex&) const;

	public slots:
		virtual void login(RzxComputer *);
		virtual void logout(RzxComputer *);
		virtual void update(RzxComputer *);
		virtual void clear();
};

///Retourne l'objet global
/** L'objet est construit si ceci est nécessaire. */
inline RzxRezalModel *RzxRezalModel::global()
{
	if(!object)
		object = new RzxRezalModel();
	return object;
}

///Création d'un index associé à un objet
/** Permet la création d'un QModelIndex associé à un RzxComputer. Ce RzxComputer est indentifé par deux objets :
 * 	- la liste des Computers associés au flag
 * 	- l'index du computer dans la liste
 *
 * On peut donc vérifier que l'objet existe, générer sont index (flag | row) et retourner l'index correspondant.
 */
inline QModelIndex RzxRezalModel::createIndex(int row, int column, int flag, const QList<RzxComputer*>& list) const
{
	if(row < 0 || row >= list.count())
		return QModelIndex();
	
	return QAbstractItemModel::createIndex(row, column, (int)flag | row);
}

#endif

/***************************************************************************
                          rzxrezalmap  -  description
                               -------------------
    begin                : Sat Sep 11 2005
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
#ifndef RZXREZALMAP_H
#define RZXREZALMAP_H

#include <QAbstractItemView>
#include <QHash>
#include <QPolygon>

#include <RzxHostAddress>
#include <RzxSubnet>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLMAP_BUILTIN
#       define RZX_BUILTIN
#else
#       define RZX_PLUGIN
#endif

#include <RzxRezal>

class QPainter;
class QComboBox;
class QSettings;

/**
 @author Florent Bruneau
 */

///Afficheur des items sous forme de plan
/**
 * Ce Rezal offre l'affichage des items sous d'un plan qui permet de situer les
 * personnes sélectionnées.
 *
 * Il utilise pour ceci un système de cartes qui sont chargées à partir d'un fichier de configuration...
 * Les cartes sont stockées dans un fichier map.ini qui se décompose de la manière suivante :
 * 	- une section [%General] qui contient la liste des cartes disponibles (les cartes sont indexées à 0) :
 * 		- map_number = nombre de cartes disponibles
 * 		- x_map = chemin de l'image de la carte
 * 		- x_map_name = nom interne de la carte
 * 		- x_map_human_name = nom humainement compréhensible de la carte
 * 	- une section [Place_nnnn] par carte, où nnnn est le nom interne de la carte. Cette section est composée d'une série d'entrées
 * qui comportent chacune 2 parties :
 * 		- un nom pppp donnée à chaque polygone
 * 		- une liste de point de la forme XXXxYYY séparés par des espaces comme valeur.
 * 	- une section [Attrib_nnnn] par carte, ou nnnn est le nom interne de la carte. Cette section est composée d'une série d'entrées
 * permettant d'associer des lieux aux adresses réseau :
 * 		- une liste d'ip ou de subnet sous la forme ip,ip,ip ou ip-mask,ip-mask,ip-mask
 * 		- les informations de traitement sous la forme d'un place_pppp qui référencie une entrée de la section précédente, et si
 * la section décrit une liste de subnets, un champs goto_nnnn permet de créer un lien vers une autre carte.
 * 	- une section [Mask_nnnn] par carte, ou nnnn est le nom interne de la carte. Cette section est composée d'une série d'entréees
 * permettant d'associer à chaque lieu une image servant de masque pour repérer ce lieu. Chaque entrée est défini comme suit :
 * 		- le nom du lieu à traiter
 * 		- les informations du masque : la coordonnée du point supérieur droit du masque sur l'image de la carte suivi du chemin relatif
 * du masque par rapport au fichier map.ini. La coordonnée est facultative... si elle est absente, le point est automatique 0x0
 *
 * La section [Mask_nnnn]. En cas d'absence d'entrée pour un lieu dans [Mask], un masque par défaut sera généré par le module à partir
 * des coordonnées du polygone définissant le lieu.
 *
 * Un exemple plus ou moins complet avec 2 cartes :
 * \code
 * [%General]
 * map_number=2
 * 0_map=maps/ma_carte1.png
 * 0_map_human_name=Ma première carte
 * 0_map_name=carte1
 * 1_map=maps/ma_carte2.png
 * 1_map_human_name=Ma deuxième carte
 * 1_map_name_carte2
 *
 *
 * [Place_carte1]
 * Place1 = 0x0 0x10 20x10 20x0
 * Place2 = 42x69 62x69 62x49 42x49
 *
 * [Attrib_carte1]
 * 129.104.201.51=place_Place1
 * 129.104.207.0-24=place_Place2 goto_carte2
 *
 * [Mask_carte1]
 * Place1 = carte1/place1.png
 * Place2 = 40x60 carte1/place2.png
 *
 *
 * [Place_carte2]
 * Place1 = ...
 * Place2 = ...
 * Place3 = ...
 * Place4 = ...
 * Place5 = ...
 *
 * [Attrib_carte2]
 * 129.104.207.153=place_Place1
 * 129.104.207.83=place_Place2
 * 129.104.227.161=place_Place3
 * \endcode
 */
class RzxRezalMap : public QAbstractItemView, public RzxRezal
{
	Q_OBJECT

	struct Map
	{
		QPixmap pixmap;
		QString name;
		QString humanName;
		bool useSubnets;
		QList<RzxSubnet> subnets;
		QHash<RzxHostAddress, QString> polygons;
		QHash<RzxHostAddress, QString> links;
		QHash<QString, QPolygon> places;
		QHash<QString, QString> masks;
		QHash<QString, QPoint> maskPositions;
	};

	enum ScrollHintExt
	{
		EnsureVisible = 0,
		PositionAtTop = 1,
		PositionAtBottom = 2,
		PositionCentered = 3,
	};

	enum SelectionType
	{
		Index = 0,
		Place = 1
	};

	QVector<Map*> mapTable;
	Map *currentMap;
	
	QComboBox *mapChooser;
	QComboBox *placeSearch;
	QString currentPlace;
	bool initialised;

	SelectionType selection;

	private:
		QStringList loadMaps();
		void loadMap(QSettings&, const QDir&, Map*);
		void loadPlaces(QSettings&, Map*);
		void loadMasks(QSettings&, const QDir&, Map*);
	
	public:
		RzxRezalMap(QWidget *parent = 0);
		~RzxRezalMap();

		virtual bool isInitialised() const;

		virtual QAbstractItemView *widget();
		virtual QDockWidget::DockWidgetFeatures features() const;
		virtual Qt::DockWidgetAreas allowedAreas() const;
		virtual Qt::DockWidgetArea area() const;
		virtual bool floating() const;
		virtual void updateLayout();
		
		virtual QModelIndex indexAt(const QPoint&) const;
		virtual QString placeAt(const QPoint&) const;
		virtual void scrollTo(const QModelIndex&, ScrollHint hint = QAbstractItemView::EnsureVisible);
		virtual QRect visualRect(const QModelIndex&) const;
		
	protected:
		virtual int horizontalOffset() const;
		virtual int verticalOffset() const;
		virtual bool isIndexHidden(const QModelIndex&) const;
		virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
		virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
		virtual QRegion visualRegionForSelection(const QItemSelection&) const;

		virtual void resizeEvent(QResizeEvent*);
		virtual void mousePressEvent(QMouseEvent *e);
		virtual void mouseDoubleClickEvent(QMouseEvent *e);
		virtual void paintEvent(QPaintEvent*);
		void drawSelection(QPainter&);
		void drawPlace(const QString&, QPainter&, const QColor&);

		QString place(const QModelIndex&) const;
		QString place(const RzxHostAddress&) const;

		QPolygon polygon(const QModelIndex&) const;
		QPolygon polygon(const RzxHostAddress&) const;
		QPolygon polygon(const QString&) const;
		void scrollTo(const QRect& , ScrollHintExt);

	protected slots:
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		void setMap(int);
		void setMap(const QString&);
		void setPlace(int);
		void checkPlace(const QString &);
};

#endif

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
 * 	- une section [Attrib_nnnn] par carte, où nnnn est le nom interne de la carte. Cette section est composée d'une série d'entrées
 * qui comportent chacune 2 parties :
 * 		- une liste d'ip ou de subnets (ip-masque) séparées par des virgules comme nom de la clé
 * 		- une liste de point de la forme XXXxYYY séparés par des espaces comme valeur. Si l'objet est décrit par des subnets les
 * valeurs peuvent également contenir une entrée goto_nnnn qui renvoie vers une autre carte. La sélection de l'objet renverra vers une
 * carte détaillée de l'objet sélectionné.
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
		QHash<RzxHostAddress, QPolygon> polygons;
		QHash<RzxHostAddress, QString> links;
	};

	QVector<Map*> mapTable;
	Map *currentMap;

	QComboBox *mapChooser;
	bool initialised;

	private:
		QStringList loadMaps();
		void loadMap(QSettings&, Map*);
	
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
		virtual void scrollTo(const QModelIndex&, ScrollHint hint = EnsureVisible);
		virtual QRect visualRect(const QModelIndex&) const;
		
	protected:
		virtual int horizontalOffset() const;
		virtual int verticalOffset() const;
		virtual bool isIndexHidden(const QModelIndex&) const;
		virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
		virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
		virtual QRegion visualRegionForSelection(const QItemSelection&) const;

		virtual void mouseDoubleClickEvent(QMouseEvent *e);
		virtual void paintEvent(QPaintEvent*);
		void drawSelection(QPainter&);

		QPolygon polygon(const QModelIndex&) const;
		QPolygon polygon(const RzxHostAddress&) const;

	protected slots:
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		void setMap(int);
		void setMap(const QString&);
};

#endif

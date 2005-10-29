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

#include "rzxrezal.h"

#include <RzxHostAddress>

class QPainter;
class QComboBox;
class QSettings;

/**
 @author Florent Bruneau
 */

///Afficheur des items sous forme de plan
/**
 * Ce Rezal offre l'affichage des items sous d'un plan qui permet de situer les
 * personnes s�lectionn�es.
 *
 * Il utilise pour ceci un syst�me de cartes qui sont charg�es � partir d'un fichier de configuration...
 * Les cartes sont stock�es dans un fichier map.ini qui se d�compose de la mani�re suivante :
 * 	- une section [%General] qui contient la liste des cartes disponibles (les cartes sont index�es � 0) :
 * 		- map_number = nombre de cartes disponibles
 * 		- x_map = chemin de l'image de la carte
 * 		- x_map_name = nom interne de la carte
 * 		- x_map_human_name = nom humainement compr�hensible de la carte
 * 	- une section [Attrib_nnnn] par carte, o� nnnn est le nom interne de la carte. Cette section est compos�e d'une s�rie d'entr�es
 * qui comportent chacune 2 parties :
 * 		- une liste d'ip s�par�es par des virgules comme nom de la cl�
 * 		- une liste de point de la forme XXXxYYY s�par�s par des espaces comme valeur
 */
class RzxRezalMap:public QAbstractItemView, public RzxRezal
{
	Q_OBJECT

	struct Map
	{
		QPixmap pixmap;
		QString name;
		QString humanName;
		QHash<RzxHostAddress, QPolygon> polygons;
	};

	QVector<Map*> mapTable;
	Map *currentMap;

	QComboBox *mapChooser;

	private:
		QStringList loadMaps();
		void loadMap(QSettings&, Map*);
	
	public:
		RzxRezalMap(QWidget *parent = 0);
		~RzxRezalMap();
	
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

		virtual void paintEvent(QPaintEvent*);
		void drawSelection(QPainter&);

		QPolygon polygon(const QModelIndex&) const;

	protected slots:
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		void setMap(int);
};

#endif

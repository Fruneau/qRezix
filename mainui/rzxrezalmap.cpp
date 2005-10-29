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
#include <QPaintEvent>
#include <QPainter>
#include <QRegExp>
#include <QList>
#include <QComboBox>

#include <QSettings>

#include <RzxConfig>
#include <RzxComputer>

#include "rzxrezalmap.h"

#include "rzxrezalmodel.h"

RZX_REZAL_EXPORT(RzxRezalMap)

///Construction de l'objet
/** Chargement de l'objet... c'est juste la construction des différents items et le chargement
 * du fichier de configuration avec la liste des cartes à charger
 */
RzxRezalMap::RzxRezalMap(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal("Platal 1.7.0-svn", "Show an interactive map of the campus")
{
	beginLoading();
	setType(TYP_DOCKABLE);
	setType(TYP_INDEXED);
	setModel(RzxRezalModel::global());
	currentMap = NULL;

	mapChooser = new QComboBox(this);
	mapChooser->move(3, 3);
	connect(mapChooser, SIGNAL(activated(int)), this, SLOT(setMap(int)));
	mapChooser->addItems(loadMaps());
	
	setMap(0);
	endLoading();
}

///Destruction
RzxRezalMap::~RzxRezalMap()
{
	beginClosing();
	currentMap = NULL;
	qDeleteAll(mapTable);
	mapTable.clear();
	endClosing();
}

///Charge la liste des cartes disponibles
QStringList RzxRezalMap::loadMaps()
{
	//Chargement de la liste des cartes et des paramètres qui vont avec...
	QString file = RzxConfig::findFile("map.ini");
	if(file.isNull())
	{
		qDebug("No map file found...");
		return QStringList();
	}

	QSettings maps(file, QSettings::IniFormat);
	maps.beginGroup("general");
	int mapNb = maps.value("map_number", 1).toInt();
	if(!mapNb)
	{
		qDebug() << "No map found in the file" << file;
		return QStringList();
	}

	//Récupération de la liste des cartes disponibles
	mapTable.resize(mapNb);
	QStringList mapNames;
	for(int i = 0 ; i < mapNb ; i++)
	{
		Map *map = new Map;
		map->name = maps.value(QString("%1_map_name").arg(i)).toString();
		map->humanName = maps.value(QString("%1_map_human_name").arg(i)).toString();
		map->pixmap = QPixmap(maps.value(QString("%1_map").arg(i)).toString());
		mapTable[i] = map;
		mapNames << map->humanName;
	}
	maps.endGroup();

	//Récupération des cartes
	for(int i = 0 ; i < mapNb ; i++)
		loadMap(maps, mapTable[i]);

	return mapNames;
}

///Charge les données de la carte indiquées
/** Les données de la carte sont constituées des associations polygone/ip qui permet
 * de gérer totalement la carte de façon dynamique
 */
void RzxRezalMap::loadMap(QSettings &maps, Map *map)
{
	maps.beginGroup("Attrib_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(QString key, keys)
	{
		QString value = maps.value(key).toString();
		QStringList subs = value.split(" ", QString::SkipEmptyParts);
		QVector<QPoint> points(subs.size());
		int i = 0;
		foreach(QString sub, subs)
		{
			QRegExp point("(\\d+)x(\\d+)");
			if(point.indexIn(sub) != -1)
				points[i] = QPoint(point.cap(1).toInt(), point.cap(2).toInt());
			else
				points[i] = QPoint();
			i++;
		}
	
		QStringList ipStrings = key.split(",");
		foreach(QString ip, ipStrings)
			map->polygons.insert(RzxHostAddress(QHostAddress(ip)), QPolygon(points));
	}
	maps.endGroup();
}

///Change la carte active
void RzxRezalMap::setMap(int map)
{
	if(map < mapTable.size() && map >= 0)
		currentMap = mapTable[map];
	viewport()->update();
}

///Retourne une fenêtre utilisable pour l'affichage.
QAbstractItemView *RzxRezalMap::widget()
{
	return this;
}

///Retourne les caractéristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalMap::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autorisées du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalMap::allowedAreas() const
{
	return Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea;
}

///Retourne la position par défaut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalMap::area() const
{
	return Qt::BottomDockWidgetArea;
}

///Retourne l'état par défaut du rezal
bool RzxRezalMap::floating() const
{
	return false;
}

/** \reimp */
void RzxRezalMap::updateLayout()
{
}

QModelIndex RzxRezalMap::indexAt(const QPoint &point) const
{
	foreach(RzxHostAddress ip, currentMap->polygons.keys())
	{
		if(QRegion(currentMap->polygons[ip]).contains(point))
		{
			RzxComputer *computer = ip.computer();
			if(computer)
				return RzxRezalModel::global()->index(computer, RzxRezalModel::global()->everybodyGroup);

			//On pourrait quitter immédiatement, mais un même lieu peut avoir plusieurs IPs...
			//return QModelIndex();
		}
	}
	return QModelIndex();
}

void RzxRezalMap::scrollTo(const QModelIndex&, ScrollHint)
{
}

QRect RzxRezalMap::visualRect(const QModelIndex& index) const
{
	return polygon(index).boundingRect();
}

int RzxRezalMap::horizontalOffset() const
{
	return 0;
}

int RzxRezalMap::verticalOffset() const
{
	return 0;
}

bool RzxRezalMap::isIndexHidden(const QModelIndex& index) const
{
	return !index.isValid();
}

QModelIndex RzxRezalMap::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
	return QModelIndex();
}

void RzxRezalMap::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
}

QRegion RzxRezalMap::visualRegionForSelection(const QItemSelection& sel) const
{
	QModelIndexList indexes = sel.indexes();
	QRegion region;
	foreach(QModelIndex index, indexes)
		region += QRegion(polygon(index));
	return region;
}

///Retourne le polygone associé à un index
QPolygon RzxRezalMap::polygon(const QModelIndex& index) const
{
	if(!index.isValid() || !currentMap)
		return QPolygon();

	QVariant value = index.model()->data(index, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
		return QPolygon();

	RzxComputer *computer = value.value<RzxComputer*>();
	if(computer)
		return currentMap->polygons[computer->ip()];
	else
		return QPolygon();
}

///Affichage... réalise simplement le dessin
void RzxRezalMap::paintEvent(QPaintEvent*)
{
	if(!currentMap) return;

	QPainter painter(viewport());
	painter.drawPixmap(0,0, currentMap->pixmap);
	drawSelection(painter);
}

///Dessin de la sélection
void RzxRezalMap::drawSelection(QPainter& painter)
{
	painter.drawPolygon(polygon(currentIndex()));
}

///Raffraichi l'affichage lors d'un changement de sélection
void RzxRezalMap::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QAbstractItemView::currentChanged(current, previous);
	viewport()->update();
}

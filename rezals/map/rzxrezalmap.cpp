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
#define RZX_MODULE_NAME "Platal"
#define RZX_MODULE_DESCRIPTION "Show an interactive map of the campus"
#define RZX_MODULE_ICON RzxThemedIcon("rzlmap")

#include <QPaintEvent>
#include <QPainter>
#include <QRegExp>
#include <QList>
#include <QComboBox>
#include <QScrollBar>
#include <QSettings>

#include <RzxApplication>
#include <RzxConfig>
#include <RzxComputer>

#include <RzxRezalModel>
#include <RzxRezalPopup>

#include "rzxrezalmap.h"

RZX_REZAL_EXPORT(RzxRezalMap)

///Construction de l'objet
/** Chargement de l'objet... c'est juste la construction des différents items et le chargement
 * du fichier de configuration avec la liste des cartes à charger
 */
RzxRezalMap::RzxRezalMap(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal(RZX_MODULE_NAME,
		QT_TRANSLATE_NOOP("RzxBaseModule", "Show an interactive map of the campus"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(TYP_ALL);
	setType(TYP_INDEX);
	setIcon(RZX_MODULE_ICON);
	setModel(RzxRezalModel::global());
	currentMap = NULL;

	mapChooser = new QComboBox(this);
	mapChooser->move(3, 3);
	connect(mapChooser, SIGNAL(activated(int)), this, SLOT(setMap(int)));
	placeSearch = new QComboBox(this);
	placeSearch->move(3,22);
	placeSearch->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
	placeSearch->setMinimumContentsLength(10);
	placeSearch->setEditable(true);
	placeSearch->setAutoCompletion(true);
	connect(placeSearch, SIGNAL(activated(int)),this, SLOT(setPlace(int)));
	connect(placeSearch, SIGNAL(editTextChanged(const QString &)),this, SLOT(checkPlace(const QString &)));
	currentPlace = "";

	QStringList mapNames = loadMaps();
	if(mapNames.size())
	{
		mapChooser->addItems(mapNames);
		setMap(0);
		initialised = true;
	}
	else
		initialised = false;
	setCursor(Qt::PointingHandCursor);
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

/** \reimp */
bool RzxRezalMap::isInitialised() const
{
	return initialised;
}

///Charge la liste des cartes disponibles
QStringList RzxRezalMap::loadMaps()
{
	//Chargement de la liste des cartes et des paramètres qui vont avec...
	QString file = RzxConfig::findFile("map.ini");
	QDir dir(file.left(file.length() - 7));
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
		map->pixmap = QPixmap(dir.absoluteFilePath(maps.value(QString("%1_map").arg(i)).toString()));
		mapTable[i] = map;
		mapNames << map->humanName;
	}
	maps.endGroup();

	//Récupération des cartes
	for(int i = 0 ; i < mapNb ; i++)
		loadMap(maps, dir, mapTable[i]);

	return mapNames;
}

///Charge les données de la carte indiquées
/** Les données de la carte sont constituées des associations polygone/ip qui permet
 * de gérer totalement la carte de façon dynamique
 */
void RzxRezalMap::loadMap(QSettings &maps, const QDir& dir, Map *map)
{
	loadPlaces(maps, map);
	loadMasks(maps, dir, map);

	maps.beginGroup("Attrib_" + map->name);
	const QStringList keys = maps.childKeys();
	foreach(QString key, keys)
	{
		QString place;
		QString linkedMap;

		//Analyse des valeurs...
		//Ceci inclus la définition des polygones et des liens
		const QString value = maps.value(key).toString();
		const QStringList subs = value.split(" ", QString::SkipEmptyParts);
		foreach(QString sub, subs)
		{
			static QRegExp linkMask("goto_(.+)");
			static QRegExp placeMask("place_(.+)");
			if(linkMask.indexIn(sub) != -1)
				linkedMap = linkMask.cap(1);
			if(placeMask.indexIn(sub) != -1)
				place = placeMask.cap(1);
		}

		//L'emplacement associé n'est pas défini pour cette carte
		if(place.isEmpty() || !map->places.keys().contains(place)) continue;
	
		//Analyse des adresses
		//Ce qui inclus la gestion des adresses et des sous-réseaux
		QStringList ipStrings = key.split(",");
		map->useSubnets = false;
		foreach(QString ip, ipStrings)
		{
			RzxHostAddress base;
			if(ip.contains("-"))
			{
				ip.replace('-', '/');
				RzxSubnet subnet(ip);
				map->useSubnets = true;
				map->subnets << subnet;
				base = subnet.network();
			}
			else
				base = RzxHostAddress(QHostAddress(ip));
			map->polygons.insert(base, place);
			if(!linkedMap.isEmpty())
				map->links.insert(base, linkedMap);
		}
	}
	maps.endGroup();
}

///Charge les lieux particuliers de la carte indiquées
/** Les données de la carte sont constituées des associations polygone/nom du lieu
 */
void RzxRezalMap::loadPlaces(QSettings &maps, Map *map)
{
	maps.beginGroup("Place_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(QString key, keys)
	{
		//Analyse des valeurs...
		QString value = maps.value(key).toString();
		QStringList subs = value.split(" ", QString::SkipEmptyParts);
		int size = subs.size();
		QVector<QPoint> points(size);
		int i = 0;
		foreach(QString sub, subs)
		{
			static QRegExp point("(\\d+)x(\\d+)");
			if(point.indexIn(sub) != -1)
				points[i] = QPoint(point.cap(1).toInt(), point.cap(2).toInt());
			else
			{
				qDebug() << "* invalid point(" << i << ")" << sub << "in" << map->name + "/" + key;
				points[i] = QPoint();
			}
			i++;
		}
		map->places.insert(key, QPolygon(points));
	}
	maps.endGroup();
}

///Charge les masques associés à la carte donnée
/** Les données doivent être des liens vers des fichiers associés à la coordonnée supérieure gauche
 * à laquelle affiché le masque
 */
void RzxRezalMap::loadMasks(QSettings &maps, const QDir& dir, Map *map)
{
	maps.beginGroup("Mask_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(QString key, keys)
	{
		if(!map->places.keys().contains(key))
			continue;

		//Analyse des valeurs...
		QString value = maps.value(key).toString();
		QStringList subs = value.split(" ", QString::SkipEmptyParts);
		if(subs.size() > 2 || !subs.size())
			continue;

		QPoint p(0,0);
		int i=0;
		if(subs.size() == 2)
		{
			static QRegExp point("(\\d+)x(\\d+)");
			if(point.indexIn(subs[i++]) != -1)
				p = QPoint(point.cap(1).toInt(), point.cap(2).toInt());
		}

		if(dir.exists(subs[i]))
		{
			map->masks.insert(key, dir.absoluteFilePath(subs[i]));
			map->maskPositions.insert(key, p);
		}
	}
	maps.endGroup();
}

///Change la carte active
void RzxRezalMap::setMap(int map)
{
	//Changement de carte active
	if(map < mapTable.size() && map >= 0 && mapTable[map])
		currentMap = mapTable[map];
	else
		return;

	//Met à jour la Combo des lieux importants.
	placeSearch->clear();
	QStringList placeList = currentMap->places.keys();
	qSort(placeList);
	foreach(QString name, placeList)
	{
		if(!currentMap->places[name].isEmpty())
			placeSearch->addItem(name);
	}
	placeSearch->setEditText(tr("Search a place"));
	if (placeSearch->count())
		placeSearch->show();
	else
		placeSearch->hide();

	//Redimensionne les barres de défilement
	horizontalScrollBar()->setRange(0, currentMap->pixmap.width() - viewport()->width());
	horizontalScrollBar()->setPageStep(viewport()->width());
	verticalScrollBar()->setRange(0, currentMap->pixmap.height() - viewport()->height());
	verticalScrollBar()->setPageStep(viewport()->height());

	//Redimensionne la fenêtre et la vérouille
	const QSize pixSize = currentMap->pixmap.size();
	QSize newSize = size();
	if(newSize.width() > pixSize.width())
		newSize.setWidth(pixSize.width());
	if(newSize.height() > pixSize.height())
		newSize.setHeight(pixSize.height());
	resize(newSize);
	setMaximumSize(pixSize);

	//Déplace la vue sur l'index courant
	scrollTo(currentIndex());

	//Met à jour l'affichage
	viewport()->update();
}

///Met à jour la taille de la carte
/** Ajuste les bars de défilement
 */
void RzxRezalMap::resizeEvent(QResizeEvent *e)
{
	QAbstractItemView::resizeEvent(e);
	horizontalScrollBar()->setRange(0, currentMap->pixmap.width() - viewport()->width());
	horizontalScrollBar()->setPageStep(viewport()->width());
	verticalScrollBar()->setRange(0, currentMap->pixmap.height() - viewport()->height());
	verticalScrollBar()->setPageStep(viewport()->height());
	setHorizontalScrollBarPolicy((Qt::ScrollBarPolicy)(size().width() == maximumSize().width() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAlwaysOn));
	setVerticalScrollBarPolicy((Qt::ScrollBarPolicy)(size().height() == maximumSize().height() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAlwaysOn));

	placeSearch->move(width() - placeSearch->width() - 3 - (verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0), 3);
}

///Change la carte active vers la carte indiquée
void RzxRezalMap::setMap(const QString& name)
{
	for(int i = 0 ; i < mapTable.size() ; i++)
	{
		if(mapTable[i]->name == name)
		{
			setMap(i);
			mapChooser->setCurrentIndex(i);
			return;
		}
	}
}

///Déplace la carte vers le lieux demandé
void RzxRezalMap::setPlace(int place)
{
	if(currentMap && place < currentMap->places.size() && place >= 0)
	{
		currentPlace = placeSearch->currentText();
		QRect rect;
		if (currentMap->places.contains(currentPlace))
		{
			selection = Place;
			rect = currentMap->places[currentPlace].boundingRect();
			scrollTo(rect, PositionCentered);
			viewport()->update();
		}
	}
}

///Vérifie que le nom de lieu tapé correspond bien à un existant
void RzxRezalMap::checkPlace(const QString &)
{
	//TODO
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
	viewport()->update();
}

///Recherche l'objet visible au point indiqué
QModelIndex RzxRezalMap::indexAt(const QPoint &point) const
{
	foreach(RzxHostAddress ip, currentMap->polygons.keys())
	{
		if(QRegion(polygon(ip)).contains(point))
		{
			if(!currentMap->useSubnets)
			{
				RzxComputer *computer = ip.computer();
				if(computer)
					return RzxRezalModel::global()->index(computer, RzxRezalModel::global()->everybodyGroup[0]);
			}
			else
			{
				foreach(RzxSubnet subnet, currentMap->subnets)
				{
					if(subnet.contains(ip))
						return RzxRezalModel::global()->index(subnet);
				}
			}
		}
	}
	return QModelIndex();
}

///Recherche le lieu situé au point indiqué
QString RzxRezalMap::placeAt(const QPoint& point) const
{
	if(!currentMap) return QString();

	foreach(QString place, currentMap->places.keys())
	{
		if(QRegion(polygon(place)).contains(point))
			return place;
	}
	return QString();
}

///Déplace la vue pour rendre l'objet visible
void RzxRezalMap::scrollTo(const QModelIndex& index, ScrollHint hint)
{
	QRect rect = visualRect(index);
	rect.translate(horizontalOffset(), verticalOffset());
	switch (hint)
	{
		case QAbstractItemView::EnsureVisible:
			scrollTo(rect, EnsureVisible);
			break;

		case QAbstractItemView::PositionAtTop:
			scrollTo(rect, PositionAtTop);
			break;

		case QAbstractItemView::PositionAtBottom:
			scrollTo(rect, PositionAtBottom);
			break;
	}
}

///Déplace la vue pour rendre le rectangle visible
void RzxRezalMap::scrollTo(const QRect& rect, ScrollHintExt hint)
{
	if(rect.isNull()) return;

	int width = viewport()->width();
	int height = viewport()->height();
	int xOffset = horizontalOffset();
	int yOffset = verticalOffset();

	switch(hint)
	{
		case EnsureVisible:
			if(rect.top() < yOffset)
				yOffset = rect.top();
			else if(rect.bottom() > yOffset + height)
				yOffset = rect.bottom() - height;

			if(rect.left() < xOffset)
				xOffset = rect.left();
			else if(rect.right() > xOffset + width)
				xOffset = rect.right() - width;
			break;

		case PositionAtTop:
			yOffset = rect.top();
			break;

		case PositionAtBottom:
			xOffset = rect.bottom() - width;
			break;

		case PositionCentered:
			xOffset = (rect.left() + rect.right() - width)/2;
			yOffset = (rect.top() + rect.bottom() - height)/2;
			break;
	}
	horizontalScrollBar()->setValue(xOffset);
	verticalScrollBar()->setValue(yOffset);
}

///Retourne le rectangle dans lequel se trouve l'index
QRect RzxRezalMap::visualRect(const QModelIndex& index) const
{
	return polygon(index).boundingRect();
}

///Retourne la position horizontale de la vue
int RzxRezalMap::horizontalOffset() const
{
	return horizontalScrollBar()->value();
}

///Retourne la postion verticale de la vue
int RzxRezalMap::verticalOffset() const
{
	return verticalScrollBar()->value();
}

///Indique si l'index spécifié est caché ou non
bool RzxRezalMap::isIndexHidden(const QModelIndex& index) const
{
	return !index.isValid();
}

///Déplace la sélection avec le clavier
/** Pour l'instant non implémenté... à voir si nécessaire */
QModelIndex RzxRezalMap::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
	return QModelIndex();
}

///Défini la sélection correspondante au rectangle indiqué
void RzxRezalMap::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
}

///Retourne la région correspondante à la liste des objets sélectionnés
/** L'implémentation est faite proprement... mais normalement il n'y a jamais plus d'un
 * objet sélectionné à la fois
 */
QRegion RzxRezalMap::visualRegionForSelection(const QItemSelection& sel) const
{
	QModelIndexList indexes = sel.indexes();
	QRegion region;
	foreach(QModelIndex index, indexes)
		region += QRegion(polygon(index));
	return region;
}

///Retourne le lieu associé à l'index
QString RzxRezalMap::place(const QModelIndex& index) const
{
	if(!index.isValid() || !currentMap)
		return QString();

	if(!currentMap->useSubnets || RzxRezalModel::global()->rezal(index) == -1)
	{
		QVariant value = index.model()->data(index, Qt::UserRole);
		if(!value.canConvert<RzxComputer*>())
			return QString();

		RzxComputer *computer = value.value<RzxComputer*>();
		if(!computer)
			return QString();

		return place(computer->ip());
	}
	else
	{
		const int rezal = RzxRezalModel::global()->rezal(index);
		foreach(RzxHostAddress ip, currentMap->polygons.keys())
		{
			if(RzxConfig::rezal(ip) == rezal)
				return currentMap->polygons[ip];
		}
		return QString();
	}
}

///Retourne le lieu associé à l'ip indiquée
QString RzxRezalMap::place(const RzxHostAddress& ip) const
{
	if(!currentMap)
		return QString();

	if(currentMap->useSubnets)
	{
		foreach(RzxSubnet net, currentMap->subnets)
		{
			if(net.contains(ip))
				return currentMap->polygons[net.network()];
		}
	}

	return currentMap->polygons[ip];
}

///Retourne le polygone associé à un index
/** Le polygon prend en compte la position de l'affichage...
 */
QPolygon RzxRezalMap::polygon(const QModelIndex& index) const
{
	QString plc = place(index);
	if(plc.isNull())
		return QPolygon();
	else
		return polygon(plc);
}

///Retourne le polygone associé à l'adresse
/** Le polygon prend en compte la position de l'affichage...
 */
QPolygon RzxRezalMap::polygon(const RzxHostAddress& ip) const
{
	QString plc = place(ip);
	if(plc.isNull())
		return QPolygon();
	else
		return polygon(plc);
}

///Retourne le polygone associé à l'adresse
/** Le polygon prend en compte la position de l'affichage...
 */
QPolygon RzxRezalMap::polygon(const QString& place) const
{
	if(!currentMap)
		return QPolygon();

	QPolygon poly;
	if (currentMap->places.contains(place))
		poly = currentMap->places[place];
	else
		poly = QPolygon();
	poly.translate(-horizontalOffset(), -verticalOffset());
	return poly;
}


///Affichage... réalise simplement le dessin
void RzxRezalMap::paintEvent(QPaintEvent*)
{
	if(!currentMap) return;

	QPainter painter(viewport());
	painter.drawPixmap(-horizontalOffset(),-verticalOffset(), currentMap->pixmap);
	drawSelection(painter);
}

///Dessin de la sélection
void RzxRezalMap::drawSelection(QPainter& painter)
{
	painter.setPen(Qt::NoPen);

	switch(selection)
	{
		case Index:
			drawPlace(place(currentIndex()), painter, QColor(00, 00, 0xa0, 0xc0));
			break;

		case Place:
			drawPlace(currentPlace, painter, QColor(00, 0xa0, 00, 0xd0)); //TODO voir la couleur
			break;
	}
}

///Dessine le lieu indiqué avec la couleur indiquée en cas d'absence de masque
void RzxRezalMap::drawPlace(const QString& place, QPainter& painter, const QColor& color)
{
	if(place.isEmpty()) return;

	if(currentMap->masks.keys().contains(place))
		painter.drawPixmap(currentMap->maskPositions[place] - QPoint(horizontalOffset(),verticalOffset()),
			 QPixmap(currentMap->masks[place]));
	else
	{
		painter.setBrush(color);
		painter.drawPolygon(polygon(place));
	}
}

///Raffraichi l'affichage lors d'un changement de sélection
void RzxRezalMap::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QAbstractItemView::currentChanged(current, previous);
	if(RzxRezalModel::global()->rezal(current) != -1)
	{
		const QString plc = place(current);
		if(!plc.isNull())
		{
			const int i = placeSearch->findText(plc);
			if(i != -1)
			{
				placeSearch->setCurrentIndex(i);
				setPlace(i);
			}
		}
		return;
	}

	selection = Index;
	scrollTo(current);
	viewport()->update();
}

///On clique sur un objet...
/** Le clic active la sélection des lieux
 */
void RzxRezalMap::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::RightButton)
		new RzxRezalPopup(indexAt(e->pos()), e->globalPos(), this);
	else if(e->button() == Qt::LeftButton)
	{
		initialPoint = e->pos();
		const QModelIndex index = indexAt(e->pos());
		if(!index.isValid())
		{
			const QString plc = placeAt(e->pos());
			if(!plc.isNull())
			{
				const int i = placeSearch->findText(plc);
				if(i != -1)
				{
					placeSearch->setCurrentIndex(i);
					setPlace(i);
				}
			}
		}
		else
		{
			setCurrentIndex(index);
			emit activated(index);
			emit clicked(index);
		}
	}
}

///Lorsque la souris bouge...
void RzxRezalMap::mouseMoveEvent(QMouseEvent *e)
{
	setCursor(Qt::SizeAllCursor);
	horizontalScrollBar()->setValue(horizontalOffset() + initialPoint.x() - e->pos().x());
	verticalScrollBar()->setValue(verticalOffset() + initialPoint.y() - e->pos().y());
	initialPoint = e->pos();
}

///Lorsque la souris est relachée, on remet l'ancien curseur
void RzxRezalMap::mouseReleaseEvent(QMouseEvent *)
{
	setCursor(Qt::PointingHandCursor);
}

///On double clique sur un objet...
/** Le double clic active les liens entre les cartes
 */
void RzxRezalMap::mouseDoubleClickEvent(QMouseEvent *e)
{
	QAbstractItemView::mouseDoubleClickEvent(e);

	foreach(RzxHostAddress host, currentMap->polygons.keys())
	{
		if(QRegion(polygon(host)).contains(e->pos()))
		{
			setMap(currentMap->links[host]);
			return;
		}
	}
}

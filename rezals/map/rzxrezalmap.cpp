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
/** Chargement de l'objet... c'est juste la construction des diff�rents items et le chargement
 * du fichier de configuration avec la liste des cartes � charger
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
	ignoreChange = currentHasChanged = false;

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
	currentPlace = "";
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
	//Chargement de la liste des cartes et des param�tres qui vont avec...
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

	//R�cup�ration de la liste des cartes disponibles
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

	//R�cup�ration des cartes
	for(int i = 0 ; i < mapNb ; i++)
		loadMap(maps, dir, mapTable[i]);

	return mapNames;
}

///Charge les donn�es de la carte indiqu�es
/** Les donn�es de la carte sont constitu�es des associations polygone/ip qui permet
 * de g�rer totalement la carte de fa�on dynamique
 */
void RzxRezalMap::loadMap(QSettings &maps, const QDir& dir, Map *map)
{
	loadPlaces(maps, map);
	loadMasks(maps, dir, map);

	maps.beginGroup("Attrib_" + map->name);
	const QStringList keys = maps.childKeys();
	foreach(const QString &key, keys)
	{
		QString place;
		QString linkedMap;

		//Analyse des valeurs...
		//Ceci inclus la d�finition des polygones et des liens
		const QString value = maps.value(key).toString();
		const QStringList subs = value.split(" ", QString::SkipEmptyParts);
		foreach(const QString &sub, subs)
		{
			if(sub.size() > 5 && sub.startsWith("goto_"))
			{
				linkedMap = sub.mid(5);
			}
			else if (sub.size() > 6 && sub.startsWith("place_"))
			{
				place = sub.mid(6);
			}
		}

		//L'emplacement associ� n'est pas d�fini pour cette carte
		if(place.isEmpty() || !map->places.keys().contains(place)) continue;
	
		//Analyse des adresses
		//Ce qui inclus la gestion des adresses et des sous-r�seaux
		const QStringList ipStrings = key.split(",");
		map->useSubnets = false;
		foreach(const QString &ip, ipStrings)
		{
			RzxHostAddress base;
			if(ip.contains("-"))
			{
				QString ipCopy(ip);
				ipCopy.replace('-', '/');
				RzxSubnet subnet(ipCopy);
				map->useSubnets = true;
				map->subnets << subnet;
				base = subnet.network();
			}
			else
			{
				base = RzxHostAddress(QHostAddress(ip));
				map->noSubnet << base;
			}
			map->polygons.insert(base, place);
			if(!linkedMap.isEmpty())
				map->links.insert(base, linkedMap);
		}
	}
	maps.endGroup();
}

///Charge les lieux particuliers de la carte indiqu�es
/** Les donn�es de la carte sont constitu�es des associations polygone/nom du lieu
 */
void RzxRezalMap::loadPlaces(QSettings &maps, Map *map)
{
	maps.beginGroup("Place_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(const QString &key, keys)
	{
		//Analyse des valeurs...
		const QString value = maps.value(key).toString();
		const QStringList subs = value.split(" ", QString::SkipEmptyParts);
		int size = subs.size();
		QVector<QPoint> points(size);
		int i = 0;
		foreach(const QString &sub, subs)
		{
			int pos = sub.indexOf('x');
			if (pos > 0) {
				bool ok = false;
				int x = sub.left(pos).toInt(&ok);
				if (ok) {
					int y = sub.mid(pos + 1).toInt(&ok);
					if (ok) {
						points[i++] = QPoint(x, y);
						continue;
					}
				}
			}
			qDebug() << "* invalid point(" << i << ")" << sub << "in" << map->name + "/" + key;
			points[i++] = QPoint();
		}
		map->places.insert(key, QPolygon(points));
	}
	maps.endGroup();
}

///Charge les masques associ�s � la carte donn�e
/** Les donn�es doivent �tre des liens vers des fichiers associ�s � la coordonn�e sup�rieure gauche
 * � laquelle affich� le masque
 */
void RzxRezalMap::loadMasks(QSettings &maps, const QDir& dir, Map *map)
{
	maps.beginGroup("Mask_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(const QString &key, keys)
	{
		if(!map->places.keys().contains(key))
			continue;

		//Analyse des valeurs...
		const QString value = maps.value(key).toString();
		const QStringList subs = value.split(" ", QString::SkipEmptyParts);
		if(subs.size() > 2 || !subs.size())
			continue;

		QPoint p(0,0);
		int i=0;
		if(subs.size() == 2)
		{
			const QString& sub = subs[i++];
			int pos = sub.indexOf('x');
			if (pos > 0) {
				bool ok = false;
				int x = sub.left(pos).toInt(&ok);
				if (ok) {
					int y = sub.mid(pos + 1).toInt(&ok);
					if (ok) {
						p = QPoint(x, y);
					}
				}
			}
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
	if(map < mapTable.size() && map >= 0 && mapTable[map] && currentMap != mapTable[map])
	{
		currentMap = mapTable[map];
		if(mapChooser->currentIndex() != map)
			mapChooser->setCurrentIndex(map);
	}
	else
		return;

	//Met � jour la Combo des lieux importants.
	placeSearch->clear();
	QStringList placeList = currentMap->places.keys();
	qSort(placeList);
	foreach(const QString &name, placeList)
	{
		if(!currentMap->places[name].isEmpty())
			placeSearch->addItem(name);
	}
	placeSearch->setEditText(tr("Search a place"));
	if (placeSearch->count())
		placeSearch->show();
	else
		placeSearch->hide();

	//Redimensionne les barres de d�filement
	horizontalScrollBar()->setRange(0, currentMap->pixmap.width() - viewport()->width());
	horizontalScrollBar()->setPageStep(viewport()->width());
	verticalScrollBar()->setRange(0, currentMap->pixmap.height() - viewport()->height());
	verticalScrollBar()->setPageStep(viewport()->height());

	//Redimensionne la fen�tre et la v�rouille
	const QSize pixSize = currentMap->pixmap.size();
	QSize newSize = size();
	if(newSize.width() > pixSize.width())
		newSize.setWidth(pixSize.width());
	if(newSize.height() > pixSize.height())
		newSize.setHeight(pixSize.height());
	viewport()->resize(newSize);
	viewport()->setMaximumSize(pixSize);

	//D�place la vue sur l'index courant
	scrollTo(currentIndex());

	//Met � jour l'affichage
	viewport()->update();
}

///Met � jour la taille de la carte
/** Ajuste les bars de d�filement
 */
void RzxRezalMap::resizeEvent(QResizeEvent *e)
{
	QAbstractItemView::resizeEvent(e);

	if(!currentMap) return;
	const QSize pixsize = currentMap->pixmap.size();
	const QSize viewsize = viewport()->size();

	horizontalScrollBar()->setRange(0, pixsize.width() - viewsize.width());
	horizontalScrollBar()->setPageStep(viewsize.width());
	verticalScrollBar()->setRange(0, pixsize.height() - viewsize.height());
	verticalScrollBar()->setPageStep(viewsize.height());

	int viewportWidth = viewsize.width();
	int viewportHeight = viewsize.height();
	if(verticalScrollBar()->isVisible()) viewportWidth += verticalScrollBar()->width();
	if(horizontalScrollBar()->isVisible()) viewportHeight += horizontalScrollBar()->height();

	setHorizontalScrollBarPolicy((Qt::ScrollBarPolicy)(viewportWidth >= pixsize.width() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAlwaysOn));
	setVerticalScrollBarPolicy((Qt::ScrollBarPolicy)(viewportHeight >= pixsize.height() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAlwaysOn));

	placeSearch->move(viewsize.width() - placeSearch->width() - 3, 3);
}

///Change la carte active vers la carte indiqu�e
void RzxRezalMap::setMap(const QString& name)
{
	setMap(map(name));
}

///D�place la carte vers le lieux demand�
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
			if(!currentHasChanged)
				scrollTo(rect, PositionCentered);
			timer.start(250, this);
			blinkingStep = 0;
			viewport()->update();
		}
	}
}

///Retourne une fen�tre utilisable pour l'affichage.
QAbstractItemView *RzxRezalMap::widget()
{
	return this;
}

///Retourne les caract�ristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalMap::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autoris�es du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalMap::allowedAreas() const
{
	return Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea;
}

///Retourne la position par d�faut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalMap::area() const
{
	return Qt::BottomDockWidgetArea;
}

///Retourne l'�tat par d�faut du rezal
bool RzxRezalMap::floating() const
{
	return false;
}

///Ne retourne rien puisque pas de bouton
QAction* RzxRezalMap::toolButton()
{
	return 0;
}

/** \reimp */
void RzxRezalMap::updateLayout()
{
	viewport()->update();
}

///Recherche l'objet visible au point indiqu�
QModelIndex RzxRezalMap::indexAt(const QPoint &point) const
{
	foreach(const RzxHostAddress &ip, currentMap->polygons.keys())
	{
		if(QRegion(polygon(ip)).contains(point))
		{
			if(currentMap->noSubnet.contains(ip))
			{
				RzxComputer *computer = ip.computer();
				if(computer)
				{
					const bool isMenu = RzxRezalModel::global()->hasChildren(currentIndex());
					const QModelIndex indexWithParent = RzxRezalModel::global()->index(computer, 
								isMenu ? currentIndex():currentIndex().parent());
					if(indexWithParent.isValid())
						return indexWithParent;
					if((isMenu && currentIndex().parent().isValid()) || currentIndex().parent().parent().isValid())
					{
						const QModelIndex parent = isMenu ? currentIndex().parent() : currentIndex().parent().parent();
						for(int i = 0 ; i < RzxRezalModel::global()->rowCount(parent) ; i++)
						{
							const QModelIndex index = RzxRezalModel::global()->index(computer, parent.child(i, 0));
							if(index.isValid())
								return index;
						}
					}

					return RzxRezalModel::global()->index(computer, RzxRezalModel::global()->everybodyGroup[0]);
				}
			}
			else
			{
				foreach(const RzxSubnet &subnet, currentMap->subnets)
				{
					if(subnet.contains(ip))
						return RzxRezalModel::global()->index(subnet);
				}
			}
		}
	}
	return QModelIndex();
}

///Recherche le lieu situ� au point indiqu�
QString RzxRezalMap::placeAt(const QPoint& point) const
{
	if(!currentMap) return QString();

	foreach(const QString &place, currentMap->places.keys())
	{
		if(QRegion(polygon(place)).contains(point))
			return place;
	}
	return QString();
}

///D�place la vue pour rendre l'objet visible
void RzxRezalMap::scrollTo(const QModelIndex& index, ScrollHint hint)
{
	QRect rect = visualRect(index);

	if(rect.isNull() && currentHasChanged)
	{
		if(currentMap == mapTable[0]) return;
		int i = 0;
		if(model()->data(index, Qt::UserRole).canConvert<RzxComputer*>())
			i = map(model()->data(index, Qt::UserRole).value<RzxComputer*>()->ip());
		setMap(i);
		return;
	}

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

                default:
                        break;
	}
}

///D�place la vue pour rendre le rectangle visible
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
	if(!horizontalScrollBar()->isVisible())
		return 0;
	return horizontalScrollBar()->value();
}

///Retourne la postion verticale de la vue
int RzxRezalMap::verticalOffset() const
{
	if(!verticalScrollBar()->isVisible())
		return 0;
	return verticalScrollBar()->value();
}

///Indique si l'index sp�cifi� est cach� ou non
bool RzxRezalMap::isIndexHidden(const QModelIndex& index) const
{
	return !index.isValid();
}

///D�place la s�lection avec le clavier
/** Pour l'instant non impl�ment�... � voir si n�cessaire */
QModelIndex RzxRezalMap::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
	return QModelIndex();
}

///D�fini la s�lection correspondante au rectangle indiqu�
void RzxRezalMap::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
}

///Retourne la r�gion correspondante � la liste des objets s�lectionn�s
/** L'impl�mentation est faite proprement... mais normalement il n'y a jamais plus d'un
 * objet s�lectionn� � la fois
 */
QRegion RzxRezalMap::visualRegionForSelection(const QItemSelection& sel) const
{
	QModelIndexList indexes = sel.indexes();
	QRegion region;
	foreach(const QModelIndex &index, indexes)
		region += QRegion(polygon(index));
	return region;
}

///Recherche une carte qui contient exactement l'adresse donn�e
int RzxRezalMap::map(const RzxHostAddress& ip, bool withGlobal) const
{
	for(int i = withGlobal? 0:1 ; i< mapTable.size() ; i++)
	{
		const QString link = mapTable[i]->links[ip];
		if(mapTable[i]->polygons.keys().contains(ip) && !mapTable[i]->polygons[ip].isNull() && link.isNull())
			return i;
		if(!link.isNull())
		{
			const int m = map(link);
			if(m != -1)
				return m;
		}
	}
	if(withGlobal)
		return 0;
	else
		return map(ip, true);
}

///Recherche la carte dont le nom est sp�cifi�
int RzxRezalMap::map(const QString& name) const
{
	for(int i = 0 ; i< mapTable.size() ; i++)
		if(mapTable[i]->name == name)
			return i;
	return -1;
}

///Parcours les liens en profondeur pour trouver la carte � afficher
/** Retourne le nom de la carte � afficher
 */
QString RzxRezalMap::link(const RzxHostAddress& ip) const
{
	Map *localMap = currentMap;
	QList<Map*> maps;
	maps << localMap;
	while(true)
	{
		QString lnk = localMap->links[ip];
		if(lnk.isNull())
		{
			foreach(const RzxSubnet &net, localMap->subnets)
			{
				if(net.contains(ip))
				{
					lnk = localMap->links[net.network()];
					break;
				}
			}
		}

		if(!lnk.isNull())
		{
			const int mapId = map(lnk);
			if(mapId == -1) return localMap->name;
			localMap = mapTable[mapId];
		}
		else
			return localMap->name;

		if(maps.contains(localMap))
		{
			if(maps.size() > 1)
				return maps[1]->name;
			else
				return maps[0]->name;
		}
		maps << localMap;
	}
}

///Retourne le lieu associ� � l'index
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
		foreach(const RzxHostAddress &ip, currentMap->polygons.keys())
		{
			if(RzxConfig::rezal(ip) == rezal)
				return currentMap->polygons[ip];
		}
		return QString();
	}
}

///Retourne le lieu associ� � l'ip indiqu�e
QString RzxRezalMap::place(const RzxHostAddress& ip) const
{
	if(!currentMap)
		return QString();

	if(!currentMap->noSubnet.contains(ip))
	{
		foreach(const RzxSubnet &net, currentMap->subnets)
		{
			if(net.contains(ip))
				return currentMap->polygons[net.network()];
		}
	}

	return currentMap->polygons[ip];
}

///Retourne le polygone associ� � un index
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

///Retourne le polygone associ� � l'adresse
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

///Retourne le polygone associ� � l'adresse
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

///Affichage... r�alise simplement le dessin
void RzxRezalMap::paintEvent(QPaintEvent*)
{
	if(!currentMap) return;

	QPainter painter(viewport());
	painter.setPen(Qt::NoPen);
	painter.drawPixmap(-horizontalOffset(),-verticalOffset(), currentMap->pixmap);
	const QList<RzxHostAddress> ipList = currentMap->polygons.keys();
	const QList<RzxHostAddress> linkList = currentMap->links.keys();
	const QBrush brush = painter.brush();
	QBrush newBrush;
	foreach(const RzxHostAddress &ip, ipList)
	{
		const RzxComputer *computer = ip.computer();
		if(computer && (!linkList.contains(ip) || currentMap->links[ip].isEmpty()))
		{
			const QPolygon polyg = polygon(ip);
			const QRect rect = polyg.boundingRect();
			int w = rect.width(), h = rect.height();
			int px = w, py = h;
			if(4*w < 3*h) w = (h * 3) / 4;
			if(4*h < 3*w) h = (w * 3) / 4;
			px = (w - px) / 2;
			py = (h - py) / 2;
			const QPixmap icon = computer->icon().scaled(w, h);
			newBrush.setTexture(icon);
			painter.setBrush(newBrush);
			painter.setBrushOrigin(rect.topLeft() - QPoint(px, py));
			painter.drawPolygon(polyg);
		}
	}
	painter.setBrush(brush);
	drawSelection(painter);
}

///Pour le clignotement de la case s�lectionn�e
void RzxRezalMap::timerEvent(QTimerEvent *e)
{
	if(e->timerId() == timer.timerId())
	{
		blinkingStep++;
		if(blinkingStep == 5) timer.stop();
		update();
	}
	else
		QAbstractItemView::timerEvent(e);
}

///Dessin de la s�lection
void RzxRezalMap::drawSelection(QPainter& painter)
{
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

///Dessine le lieu indiqu� avec la couleur indiqu�e en cas d'absence de masque
void RzxRezalMap::drawPlace(const QString& place, QPainter& painter, const QColor& color)
{
	if(place.isEmpty()) return;

	if(currentMap->masks.keys().contains(place))
		painter.drawPixmap(currentMap->maskPositions[place] - QPoint(horizontalOffset(),verticalOffset()),
			 QPixmap(currentMap->masks[place]));
	else
	{
		painter.setBrush(blinkingStep&1? color :  QColor(0xff, 00, 00, 0xd0));
		painter.drawPolygon(polygon(place));
	}
}

///Raffraichi l'affichage lors d'un changement de s�lection
void RzxRezalMap::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if(ignoreChange)
	{
		viewport()->update();
		ignoreChange = false;
		return;
	}

	timer.start(250, this);
	blinkingStep = 0;
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
		else
		{
			if(currentMap != mapTable[0])
			{
				const int rezal = RzxRezalModel::global()->rezal(current);
				if(rezal != -1)
				{
					const int m = map(RzxConfig::rezalBase(rezal));
					setMap(m);
				}
			}
		}
		return;
	}

	selection = Index;
	if(!currentHasChanged) //Si la modification n'a pas �t� faite par l'utilisateur...
	{
		currentHasChanged = true;
		scrollTo(current);
	}
	currentHasChanged = false;
	viewport()->update();
}

///Enregistre la d�connexion de l'�l�ment s�lectionn� pour �viter de switch� de map dans ce cas
void RzxRezalMap::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int)
{
	const QModelIndex index = model()->index(start, 0, parent);
	if(selection == Index && RzxRezalModel::global()->isSameComputer(index, currentIndex()))
	{
		selection = Place;
		ignoreChange = true;
	}
}

///On clique sur un objet...
/** Le clic active la s�lection des lieux
 */
void RzxRezalMap::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::RightButton)
		new RzxRezalPopup(indexAt(e->pos()), e->globalPos(), this);
	else if(e->button() == Qt::LeftButton)
	{
		initialPoint = e->pos();
		const QModelIndex index = indexAt(e->pos());
		const QString plc = placeAt(e->pos());
		int placeId = -1;
		if(!plc.isNull())
			placeId = placeSearch->findText(plc);

		if(placeId != -1)
			placeSearch->setCurrentIndex(placeId);

		if(!index.isValid())
		{
			currentHasChanged = true;
			if(placeId != -1)
				setPlace(placeId);
			currentHasChanged = false;
		}
		else if(place(index) == place(currentIndex())) //m�me ligne ?
		{
			selection = Index;
			viewport()->update();
		}
		else
		{
			currentHasChanged = true;
			setCurrentIndex(index);
			currentHasChanged = false;
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

///Lorsque la souris est relach�e, on remet l'ancien curseur
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
	QModelIndex index = indexAt(e->pos());
	if(place(index) == place(currentIndex()))
		index = currentIndex();

	//Si on peu attribuer une ip � l'index ==> on parcours les liens en profondeur vers cette ip
	QHostAddress ip;
	QVariant data = RzxRezalModel::global()->data(index, Qt::UserRole);
	if(data.canConvert<RzxComputer*>())
		ip = data.value<RzxComputer*>()->ip();

	if(!ip.isNull())
		setMap(map(link(ip)));
	else
	{
		//Sinon on fait au plus facile...
		foreach(const RzxHostAddress &host, currentMap->polygons.keys())
		{
			if(QRegion(polygon(host)).contains(e->pos()))
			{
				setMap(currentMap->links[host]);
				return;
			}
		}
	}
}

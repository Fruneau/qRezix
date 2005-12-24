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

#include "rzxrezalmap.h"

RZX_REZAL_EXPORT(RzxRezalMap)

///Construction de l'objet
/** Chargement de l'objet... c'est juste la construction des diff�rents items et le chargement
 * du fichier de configuration avec la liste des cartes � charger
 */
RzxRezalMap::RzxRezalMap(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal(RZX_MODULE_NAME, QT_TR_NOOP("Show an interactive map of the campus"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(TYP_ALL);
	setType(TYP_INDEXED);
	setIcon(RzxThemedIcon("rzlmap"));
	setModel(RzxRezalModel::global());
	currentMap = NULL;

	mapChooser = new QComboBox(this);
	mapChooser->move(3, 3);
	connect(mapChooser, SIGNAL(activated(int)), this, SLOT(setMap(int)));
	debugText = new QLineEdit(this);	//DEBUG
	debugText->move (150,150);			//DEBUG
	debug =0;		//DEBUG
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
		loadMap(maps, mapTable[i]);

	return mapNames;
}

///Charge les donn�es de la carte indiqu�es
/** Les donn�es de la carte sont constitu�es des associations polygone/ip qui permet
 * de g�rer totalement la carte de fa�on dynamique
 */
void RzxRezalMap::loadMap(QSettings &maps, Map *map)
{
	maps.beginGroup("Attrib_" + map->name);
	QStringList keys = maps.childKeys();
	foreach(QString key, keys)
	{
		//Analyse des valeurs...
		//Ceci inclus la d�finition des polygones et des liens
		QString value = maps.value(key).toString();
		QStringList subs = value.split(" ", QString::SkipEmptyParts);
		int size = subs.size();
		if(value.contains("goto"))
			size--;
		QVector<QPoint> points(size);
		QString linkedMap;
		int i = 0;
		foreach(QString sub, subs)
		{
			static QRegExp point("(\\d+)x(\\d+)");
			static QRegExp link("goto_(.+)");
			if(point.indexIn(sub) != -1)
				points[i] = QPoint(point.cap(1).toInt(), point.cap(2).toInt());
			else if(link.indexIn(sub) != -1)
			{
				i--;
				linkedMap = link.cap(1);
			}
			else
			{
				qDebug() << "* invalid point(" << i << ")" << sub << "in" << map->name + "/" + key;
				points[i] = QPoint();
			}
			i++;
		}
	
		//Analyse des adresses
		//Ce qui inclus la gestion des adresses et des sous-r�seaux
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
			map->polygons.insert(base, QPolygon(points));
			if(!linkedMap.isEmpty())
				map->links.insert(base, linkedMap);
		}
	}
	maps.endGroup();
	loadPlaces(maps, map);
}

///Charge les lieux particuliers de la carte indiqu�es
/** Les donn�es de la carte sont constitu�es des associations polygone/nom du lieu
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

///Change la carte active
void RzxRezalMap::setMap(int map)
{
	//Changement de carte active
	if(map < mapTable.size() && map >= 0 && mapTable[map])
		currentMap = mapTable[map];
	else
		return;

	//Met � jour la Combo des lieux importants.
	placeSearch->clear();
	QString str = QString("%1").arg(currentMap->places.count());	//DEBUG
	debugText->setText(str);		//DEBUG
	foreach(QString name, currentMap->places.keys())
	{
		placeSearch->addItem(name);
	}
	placeSearch->setEditText("Rechercher un lieu");
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
	resize(newSize);
	setMaximumSize(pixSize);

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
	horizontalScrollBar()->setRange(0, currentMap->pixmap.width() - viewport()->width());
	horizontalScrollBar()->setPageStep(viewport()->width());
	verticalScrollBar()->setRange(0, currentMap->pixmap.height() - viewport()->height());
	verticalScrollBar()->setPageStep(viewport()->height());
}

///Change la carte active vers la carte indiqu�e
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

///D�place la carte vers le lieux demand�
void RzxRezalMap::setPlace(int place)
{
	if(currentMap && place < currentMap->places.size() && place >= 0)
	{
		currentPlace = placeSearch->currentText();
		QRect rect;
		if (currentMap->places.contains(currentPlace))
		{
			rect = currentMap->places[currentPlace].boundingRect();
			scrollTo(rect, PositionCentered);
		}
	}
	else
		return;
}

///V�rifie que le nom de lieu tap� correspond bien � un existant
void RzxRezalMap::checkPlace(const QString &)
{
	//TODO
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
	return Qt::TopDockWidgetArea;
}

///Retourne l'�tat par d�faut du rezal
bool RzxRezalMap::floating() const
{
	return false;
}

/** \reimp */
void RzxRezalMap::updateLayout()
{
	viewport()->update();
}

///Recherche l'objet visible au point indiqu�
QModelIndex RzxRezalMap::indexAt(const QPoint &point) const
{
	foreach(RzxHostAddress ip, currentMap->polygons.keys())
	{
		if(QRegion(polygon(ip)).contains(point))
		{
			RzxComputer *computer = ip.computer();
			if(computer)
				return RzxRezalModel::global()->index(computer, RzxRezalModel::global()->everybodyGroup);
		}
	}
	return QModelIndex();
}

///D�place la vue pour rendre l'objet visible
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
	return horizontalScrollBar()->value();
}

///Retourne la postion verticale de la vue
int RzxRezalMap::verticalOffset() const
{
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
	foreach(QModelIndex index, indexes)
		region += QRegion(polygon(index));
	return region;
}

///Retourne le polygone associ� � un index
/** Le polygon prend en compte la position de l'affichage...
 */
QPolygon RzxRezalMap::polygon(const QModelIndex& index) const
{
	if(!index.isValid() || !currentMap)
		return QPolygon();

	QVariant value = index.model()->data(index, Qt::UserRole);
	if(!value.canConvert<RzxComputer*>())
		return QPolygon();

	RzxComputer *computer = value.value<RzxComputer*>();
	if(computer)
		return polygon(computer->ip());
	else
		return QPolygon();
}

///Retourne le polygone associ� � l'adresse
/** Le polygon prend en compte la position de l'affichage...
 */
QPolygon RzxRezalMap::polygon(const RzxHostAddress& ip) const
{
	if(!currentMap)
		return QPolygon();

	if(currentMap->useSubnets)
	{
		foreach(RzxSubnet net, currentMap->subnets)
		{
			if(net.contains(ip))
			{
				QPolygon poly = currentMap->polygons[net.network()];
				poly.translate(-horizontalOffset(), -verticalOffset());
				return poly;
			}
		}
	}

	QPolygon poly = currentMap->polygons[ip];
	poly.translate(-horizontalOffset(), -verticalOffset());
	return poly;
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
	painter.drawPixmap(-horizontalOffset(),-verticalOffset(), currentMap->pixmap);
	drawSelection(painter);
}

///Dessin de la s�lection
void RzxRezalMap::drawSelection(QPainter& painter)
{
	painter.setPen(Qt::NoPen);

	painter.setBrush(QColor(00, 0xa0, 00, 0xd0));
	painter.drawPolygon(polygon(RzxComputer::localhost()->ip()));

	painter.setBrush(QColor(00, 00, 0xa0, 0xc0));
	painter.drawPolygon(polygon(currentIndex()));

	painter.setBrush(QColor(00, 00, 0xa0, 0xc0)); //TODO voir la couleur
	painter.drawPolygon(polygon(currentPlace));
}

///Raffraichi l'affichage lors d'un changement de s�lection
void RzxRezalMap::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	QAbstractItemView::currentChanged(current, previous);
	scrollTo(current);
	viewport()->update();
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

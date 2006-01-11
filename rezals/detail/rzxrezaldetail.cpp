/***************************************************************************
                         rzxrezaldetail.h  -  description
                             -------------------
    begin                : Sat Aug 13 2005
    copyright            : (C) 2005 Florent Bruneau
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
#define RZX_MODULE_NAME "Item details"
#define RZX_MODULE_DESCRIPTION "Detail of an item"
#define RZX_MODULE_ICON RzxThemedIcon("rzldetail")

#include <QMouseEvent>
#include <QHelpEvent>
#include <QEvent>
#include <QToolTip>
#include <QSplitter>
#include <QWidget>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxApplication>

#include "rzxrezaldetail.h"

RZX_REZAL_EXPORT(RzxRezalDetail)

#include <QRezix>
#include <RzxRezalDrag>
#include <RzxRezalModel>
#include <RzxRezalAction>
#include <RzxMainUIConfig>

#include "rzxrezaldetailconfig.h"
#include "ui_rzxitem.h"
#include "ui_rzxprops.h"

RZX_CONFIG_INIT(RzxRezalDetailConfig)

///Le RzxRezalDetail est un ItemView extr�mement simple
/** Il n'affiche en effet qu'un item... le currentIndex.
 */
RzxRezalDetail::RzxRezalDetail(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Detail of an item"), RZX_MODULE_VERSION),
		computer(NULL), waitProp(NULL)
{
	beginLoading();
	setModel(RzxRezalModel::global());

	new RzxRezalDetailConfig(this);

	uiDetails = new Ui::RzxItem();
	QWidget *details = new QWidget;
	uiDetails->setupUi(details);

	uiProps = new Ui::RzxProps();
	QWidget *props = new QWidget;
	uiProps->setupUi(props);
	
	splitter = new QSplitter;
	splitter->addWidget(details);
	splitter->addWidget(props);
	connect(splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(resizeEvent()));
	
	QGridLayout *glayout = new QGridLayout(viewport());
	viewport()->setLayout(glayout);
	glayout->setMargin(0);
	glayout->setSpacing(0);
	glayout->addWidget(splitter);

	splitter->setSizes(QList<int>() << RzxRezalDetailConfig::detailSize() << RzxRezalDetailConfig::propsSize());

	setMinimumSize(viewport()->minimumSize());
	setType(TYP_DOCKABLE);
	setIcon(RZX_MODULE_ICON);
	connect(RzxApplication::instance(), SIGNAL(haveProperties(RzxComputer*, bool*)),
		this, SLOT(propChanged(RzxComputer*,bool*)));
	clear();
	endLoading();
}

///Destruction
RzxRezalDetail::~RzxRezalDetail()
{
	beginClosing();
	const QList<int> sizes = splitter->sizes();
	RzxRezalDetailConfig::setDetailSize(sizes[0]);
	RzxRezalDetailConfig::setPropsSize(sizes[1]);

	delete uiDetails;
	delete uiProps;
	delete RzxRezalDetailConfig::global();
	endClosing();
}

/** \reimp */
QModelIndex RzxRezalDetail::indexAt(const QPoint&) const
{
	return currentIndex();
}

/** \reimp */
void RzxRezalDetail::scrollTo( const QModelIndex&, ScrollHint)
{
}

/** \reimp */
QRect RzxRezalDetail::visualRect( const QModelIndex& index) const
{
	if(index == currentIndex())
		return rect();
	else return QRect();
}

/** \reimp */
int RzxRezalDetail::horizontalOffset() const
{
	return 0;
}

/** \reimp */
bool RzxRezalDetail::isIndexHidden(const QModelIndex & index ) const
{
	return index != currentIndex();
}

/** \reimp */
QModelIndex RzxRezalDetail::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
	return currentIndex();
}

/** \reimp */
void RzxRezalDetail::setSelection(const QRect&, QItemSelectionModel::SelectionFlags)
{
}

/** \reimp */
int RzxRezalDetail::verticalOffset() const
{
	return 0;
}

/** \reimp */
QRegion RzxRezalDetail::visualRegionForSelection(const QItemSelection&) const
{
	return QRegion();
}

///Change l'item courant
/** Le changement de l'item courant de le cadre de ce format correspond au changement
 * complet de l'affichage...
 */
void RzxRezalDetail::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	QAbstractItemView::currentChanged(current, previous);
	clear();
	if(!current.isValid())
	{
		computer = NULL;
		return;
	}
	computer = RzxRezalModel::global()->data(current, Qt::UserRole).value<RzxComputer*>();
	if(!computer) return;
	drawComputer(computer);
	connect(uiProps->btnProperties, SIGNAL(clicked()), this, SLOT(checkProp()));
}

///Mets � jour l'affichage si n�cessaire
void RzxRezalDetail::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
	if(currentIndex().row() >= topLeft.row() && currentIndex().row() <= bottomRight.row())
		drawComputer(computer);
}

///Prend en compte le fait que la ligne actuelle va dispara�tre
/** On efface donc le contenu */
void RzxRezalDetail::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	int row = currentIndex().row();
	if(currentIndex().parent() == parent && row >= start && row <= end)
	{
		computer = NULL;
		clear();
	}
}

///Vide l'affichage
void RzxRezalDetail::clear()
{
	const QList<QLabel*> labels = findChildren<QLabel*>();
	foreach(QLabel *label, labels)
		label->clear();
	uiProps->propsView->setEnabled(false);
	uiProps->propsView->clear();
	uiProps->btnProperties->setEnabled(false);
	QAbstractItemView::disconnect(uiProps->btnProperties, SIGNAL(clicked()), 0, 0);
	drawComputer(NULL);
}

///Affichage l'object d�fini par le RzxComputer
void RzxRezalDetail::drawComputer(RzxComputer *computer)
{
	const bool active = computer;
	if(!computer) computer = RzxComputer::localhost();

	uiDetails->lblIcon->setPixmap(computer->icon());
	uiDetails->lblName->setText("<h3>" + computer->name() + "</h3>");
	uiDetails->lblComment->setText(computer->remarque(true));
	uiDetails->lblFtp->setPixmap(RzxIconCollection::getPixmap(computer->hasFtpServer()?Rzx::ICON_FTP:Rzx::ICON_NOFTP));
	uiDetails->lblHttp->setPixmap(RzxIconCollection::getPixmap(computer->hasHttpServer()?Rzx::ICON_HTTP:Rzx::ICON_NOHTTP));
	uiDetails->lblNews->setPixmap(RzxIconCollection::getPixmap(computer->hasNewsServer()?Rzx::ICON_NEWS:Rzx::ICON_NONEWS));
	uiDetails->lblSamba->setPixmap(RzxIconCollection::getPixmap(computer->hasSambaServer()?Rzx::ICON_SAMBA:Rzx::ICON_NOSAMBA));
	uiDetails->lblPrinter->setPixmap(RzxIconCollection::getPixmap(computer->hasPrinter()?Rzx::ICON_PRINTER:Rzx::ICON_NOPRINTER));
	uiDetails->lblOS->setPixmap(RzxIconCollection::global()->osPixmap(computer->sysEx(), false));
	uiDetails->lblClient->setText(computer->client());
	uiDetails->lblPromo->setPixmap(RzxIconCollection::global()->promoPixmap(computer->promo()));
	uiDetails->lblRezal->setText(computer->rezalName());
	uiDetails->lblIP->setText(computer->ip().toString());
	uiDetails->lblStateIcon->setPixmap(RzxIconCollection::getPixmap(computer->isOnResponder()?Rzx::ICON_AWAY:Rzx::ICON_HERE));
	uiDetails->lblState->setText(QString(computer->isOnResponder()?tr("away"):tr("connected")));
	if(computer->isFavorite())
		uiDetails->lblFavorite->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_FAVORITE));
	else if(computer->isIgnored())
		uiDetails->lblFavorite->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_BAN));
	else
		uiDetails->lblFavorite->clear();

	// Pour que localhost apparaisse en gris�...
	const QList<QLabel*> labels = findChildren<QLabel*>();
	foreach(QLabel *label, labels)
		label->setEnabled(active);
	if(!active) return;

	// Remplissage
	uiProps->propsView->clear();
	uiProps->btnProperties->setEnabled(true);
	QStringList props = computer->properties().split('|');
	if(props.size())
	{
		QTreeWidgetItem *item = NULL;
		uiProps->propsView->setEnabled(true);
		for(int i = 0 ; i < props.size() - 1 ; i+=2)
		{
			item = new QTreeWidgetItem(uiProps->propsView, item);
			item->setText(0, props[i]);
			item->setText(1, props[i+1]);
		}
		uiProps->lblDate->setText(RzxConfig::getCacheDate(computer->ip()));
	}
	else
		uiProps->lblDate->setText(tr("Never checked"));
}

///Demande les propri�t�s de l'object actuel
void RzxRezalDetail::checkProp()
{
	waitProp = computer;
	if(computer)	
		computer->checkProperties();
}

///Notification de la modification des propri�t�s de l'ordinateur
void RzxRezalDetail::propChanged(RzxComputer *m_computer, bool *displayed)
{
	if(m_computer == computer)
	{
		if(displayed && waitProp == m_computer) *displayed = true;
		drawComputer(computer);
		waitProp = NULL;
	}
}

///Application d'un double clic sur un des objets
/** Un double clic sur ftp lancera le ftp, etc...
 * Un double clic sur le nom ou l'ic�ne lance le chat.
 */
void RzxRezalDetail::mouseDoubleClickEvent(QMouseEvent *e)
{
	const QWidget *child = childAt(e->pos());

	if(child == uiDetails->lblFtp)
		computer->ftp();
	else if(child == uiDetails->lblHttp)
		computer->http();
	else if(child == uiDetails->lblSamba)
		computer->samba();
	else if(child == uiDetails->lblNews)
		computer->news();
	else if(child == uiDetails->lblState || child == uiDetails->lblStateIcon)
		computer->chat();
	else if(child == uiDetails->lblName || child == uiDetails->lblIcon)
		RzxRezalAction::run(computer);
	else
		QAbstractItemView::mouseDoubleClickEvent(e);
}

///Choix des �l�ments � afficher en fonction de la taille de la fen�tre
void RzxRezalDetail::resizeEvent(QResizeEvent *)
{
	uiDetails->lblState->setVisible(uiDetails->frmDetails->size().width() > 200);
	uiDetails->lblClient->setVisible(uiDetails->frmDetails->size().width() > 200);
	uiDetails->lblIP->setVisible(uiDetails->frmDetails->size().width() > 200);
	uiDetails->lblRezal->setVisible(uiDetails->frmDetails->size().width() > 210);
}

///Re�ois les informations du QDockWidget auquel la fen�tre est rattach�e
bool RzxRezalDetail::eventFilter(QObject *dock, QEvent *e)
{
	if(dock == dockWidget() && e->type() == QEvent::Move)
	{
		const Qt::DockWidgetArea area = QRezix::global()->dockWidgetArea(dockWidget());
		const Qt::Orientation orientation = splitter->orientation();
		switch(area)
		{
			case Qt::BottomDockWidgetArea: case Qt::TopDockWidgetArea:
				if(orientation == Qt::Vertical)
					splitter->setOrientation(Qt::Horizontal);
				break;
			case Qt::RightDockWidgetArea: case Qt::LeftDockWidgetArea:
				if(orientation == Qt::Horizontal)
					splitter->setOrientation(Qt::Vertical);
				break;
			default:
				break;
		}
	}
	return false;
}

///R�impl�mentation de l'affichage des tooltips...
bool RzxRezalDetail::viewportEvent(QEvent *e)
{
	if(e->type() == QEvent::Show)
		resizeEvent();

	if(e->type() != QEvent::ToolTip)
		return QAbstractItemView::viewportEvent(e);

	if (!isActiveWindow())
		return false;
	QHelpEvent *he = static_cast<QHelpEvent*>(e);

	//Recherche de l'item sur lequel on affiche le ToolTip
	const QWidget *child = childAt(he->pos());
	int column = -1;
	if(child == uiDetails->lblFtp)
		column = RzxRezalModel::ColFTP;
	else if(child == uiDetails->lblHttp)
		column = RzxRezalModel::ColHTTP;
	else if(child == uiDetails->lblSamba)
		column = RzxRezalModel::ColSamba;
	else if(child == uiDetails->lblNews)
		column = RzxRezalModel::ColNews;
	else if(child == uiDetails->lblPrinter)
		column = RzxRezalModel::ColPrinter;
	else if(child == uiDetails->lblPromo)
		column = RzxRezalModel::ColPromo;
	else if(child == uiDetails->lblOS)
		column = RzxRezalModel::ColOS;
	else if(child == uiDetails->lblRezal)
		column = RzxRezalModel::ColRezal;
	else if(child == uiDetails->lblIP)
		column = RzxRezalModel::ColIP;

	//Si �a ne correspond pas � un item � d�crire, on n'affiche rien
	if(column == -1) return true;

	//Sinon on affiche le ToolTip
	const QString tooltip = model()->headerData(column, Qt::Horizontal, Qt::ToolTipRole).toString();
	if(!tooltip.isEmpty())
		QToolTip::showText(he->globalPos(), tooltip, this);
	return true;
}

///Retourne une fen�tre utilisable pour l'affichage.
QAbstractItemView *RzxRezalDetail::widget()
{
	return this;
}

///Retourne les caract�ristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalDetail::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autoris�es du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalDetail::allowedAreas() const
{
	return Qt::AllDockWidgetAreas;
}

///Retourne la position par d�faut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalDetail::area() const
{
	return Qt::BottomDockWidgetArea;
}

///Retourne l'�tat par d�faut du rezal
bool RzxRezalDetail::floating() const
{
	return false;
}

/** \reimp */
void RzxRezalDetail::updateLayout()
{
	drawComputer(computer);
}

///Pour le drag du drag&drop
void RzxRezalDetail::mousePressEvent(QMouseEvent *e)
{
	QModelIndex model = indexAt(e->pos());
	if(!model.isValid()) return;
	RzxComputer *computer = model.model()->data(model, Qt::UserRole).value<RzxComputer*>();
	RzxRezalDrag::mouseEvent(this, e, computer);

	QAbstractItemView::mousePressEvent(e);
}

///Pour le drag du drag&drop
void RzxRezalDetail::mouseMoveEvent(QMouseEvent *e)
{
	RzxRezalDrag::mouseEvent(this, e);
	QAbstractItemView::mouseMoveEvent(e);
}

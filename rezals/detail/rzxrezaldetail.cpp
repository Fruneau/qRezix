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

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxApplication>

#include "rzxrezaldetail.h"

#include <RzxRezalDrag>
#include <RzxRezalModel>
#include <RzxRezalAction>
#include <RzxMainUIConfig>

#include "ui_rzxitem.h"

RZX_REZAL_EXPORT(RzxRezalDetail)

///Le RzxRezalDetail est un ItemView extrêmement simple
/** Il n'affiche en effet qu'un item... le currentIndex.
 */
RzxRezalDetail::RzxRezalDetail(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Detail of an item"), RZX_MODULE_VERSION),
		computer(NULL), waitProp(NULL)
{
	ui = new Ui::RzxItem();
	beginLoading();
	setModel(RzxRezalModel::global());
	ui->setupUi(viewport());
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
	delete ui;
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
	connect(ui->btnProperties, SIGNAL(clicked()), this, SLOT(checkProp()));
}

///Mets à jour l'affichage si nécessaire
void RzxRezalDetail::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
	if(currentIndex().row() >= topLeft.row() && currentIndex().row() <= bottomRight.row())
		drawComputer(computer);
}

///Prend en compte le fait que la ligne actuelle va disparaître
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
	ui->propsView->setEnabled(false);
	ui->propsView->clear();
	ui->btnProperties->setEnabled(false);
	QAbstractItemView::disconnect(ui->btnProperties, SIGNAL(clicked()), 0, 0);
	drawComputer(NULL);
}

///Affichage l'object défini par le RzxComputer
void RzxRezalDetail::drawComputer(RzxComputer *computer)
{
	const bool active = computer;
	if(!computer) computer = RzxComputer::localhost();

	ui->lblIcon->setPixmap(computer->icon());
	ui->lblName->setText("<h3>" + computer->name() + "</h3>");
	ui->lblComment->setText(computer->remarque(true));
	ui->lblFtp->setPixmap(RzxIconCollection::getPixmap(computer->hasFtpServer()?Rzx::ICON_FTP:Rzx::ICON_NOFTP));
	ui->lblHttp->setPixmap(RzxIconCollection::getPixmap(computer->hasHttpServer()?Rzx::ICON_HTTP:Rzx::ICON_NOHTTP));
	ui->lblNews->setPixmap(RzxIconCollection::getPixmap(computer->hasNewsServer()?Rzx::ICON_NEWS:Rzx::ICON_NONEWS));
	ui->lblSamba->setPixmap(RzxIconCollection::getPixmap(computer->hasSambaServer()?Rzx::ICON_SAMBA:Rzx::ICON_NOSAMBA));
	ui->lblPrinter->setPixmap(RzxIconCollection::getPixmap(computer->hasPrinter()?Rzx::ICON_PRINTER:Rzx::ICON_NOPRINTER));
	ui->lblOS->setPixmap(RzxIconCollection::global()->osPixmap(computer->sysEx(), false));
	ui->lblClient->setText(computer->client());
	ui->lblPromo->setPixmap(RzxIconCollection::global()->promoPixmap(computer->promo()));
	ui->lblRezal->setText(computer->rezalName());
	ui->lblIP->setText(computer->ip().toString());
	ui->lblStateIcon->setPixmap(RzxIconCollection::getPixmap(computer->isOnResponder()?Rzx::ICON_AWAY:Rzx::ICON_HERE));
	ui->lblState->setText(QString(computer->isOnResponder()?tr("away"):tr("connected")));
	if(computer->isFavorite())
		ui->lblFavorite->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_FAVORITE));
	else if(computer->isIgnored())
		ui->lblFavorite->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_BAN));
	else
		ui->lblFavorite->clear();

	// Pour que localhost apparaisse en grisé...
	const QList<QLabel*> labels = findChildren<QLabel*>();
	foreach(QLabel *label, labels)
		label->setEnabled(active);
	if(!active) return;

	// Remplissage
	ui->propsView->clear();
	ui->btnProperties->setEnabled(true);
	QStringList props = computer->properties().split('|');
	if(props.size())
	{
		QTreeWidgetItem *item = NULL;
		ui->propsView->setEnabled(true);
		for(int i = 0 ; i < props.size() - 1 ; i+=2)
		{
			item = new QTreeWidgetItem(ui->propsView, item);
			item->setText(0, props[i]);
			item->setText(1, props[i+1]);
		}
		ui->lblDate->setText(RzxConfig::getCacheDate(computer->ip()));
	}
	else
		ui->lblDate->setText(tr("Never checked"));
}

///Demande les propriétés de l'object actuel
void RzxRezalDetail::checkProp()
{
	waitProp = computer;
	if(computer)	
		computer->checkProperties();
}

///Notification de la modification des propriétés de l'ordinateur
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
 * Un double clic sur le nom ou l'icône lance le chat.
 */
void RzxRezalDetail::mouseDoubleClickEvent(QMouseEvent *e)
{
	const QWidget *child = childAt(e->pos());

	if(child == ui->lblFtp)
		computer->ftp();
	else if(child == ui->lblHttp)
		computer->http();
	else if(child == ui->lblSamba)
		computer->samba();
	else if(child == ui->lblNews)
		computer->news();
	else if(child == ui->lblState || child == ui->lblStateIcon)
		computer->chat();
	else if(child == ui->lblName || child == ui->lblIcon)
		RzxRezalAction::run(computer);
	else
		QAbstractItemView::mouseDoubleClickEvent(e);
}

///Choix des éléments à afficher en fonction de la taille de la fenêtre
void RzxRezalDetail::resizeEvent(QResizeEvent *)
{
	if(size().width() < 300)
		ui->frmProp->setVisible(false);
	else
	{
		ui->frmProp->setVisible(true);
		ui->lblDate->setVisible(ui->frmProp->size().width() > 200);
	}

	ui->lblState->setVisible(ui->frmDetails->size().width() > 210 ||  ui->frmProp->size().width() > 200);
	ui->lblClient->setVisible(ui->frmDetails->size().width() > 210 ||  ui->frmProp->size().width() > 200);
	ui->lblIP->setVisible(ui->frmDetails->size().width() > 210 ||  ui->frmProp->size().width() > 200);
	ui->lblRezal->setVisible(ui->frmDetails->size().width() > 210 ||  ui->frmProp->size().width() > 200);
}

///Réimplémentation de l'affichage des tooltips...
bool RzxRezalDetail::viewportEvent(QEvent *e)
{
	if(e->type() != QEvent::ToolTip)
		return QAbstractItemView::viewportEvent(e);

	if (!isActiveWindow())
		return false;
	QHelpEvent *he = static_cast<QHelpEvent*>(e);

	//Recherche de l'item sur lequel on affiche le ToolTip
	const QWidget *child = childAt(he->pos());
	int column = -1;
	if(child == ui->lblFtp)
		column = RzxRezalModel::ColFTP;
	else if(child == ui->lblHttp)
		column = RzxRezalModel::ColHTTP;
	else if(child == ui->lblSamba)
		column = RzxRezalModel::ColSamba;
	else if(child == ui->lblNews)
		column = RzxRezalModel::ColNews;
	else if(child == ui->lblPrinter)
		column = RzxRezalModel::ColPrinter;
	else if(child == ui->lblPromo)
		column = RzxRezalModel::ColPromo;
	else if(child == ui->lblOS)
		column = RzxRezalModel::ColOS;
	else if(child == ui->lblRezal)
		column = RzxRezalModel::ColRezal;
	else if(child == ui->lblIP)
		column = RzxRezalModel::ColIP;

	//Si ça ne correspond pas à un item à décrire, on n'affiche rien
	if(column == -1) return true;

	//Sinon on affiche le ToolTip
	const QString tooltip = model()->headerData(column, Qt::Horizontal, Qt::ToolTipRole).toString();
	if(!tooltip.isEmpty())
		QToolTip::showText(he->globalPos(), tooltip, this);
	return true;
}

///Retourne une fenêtre utilisable pour l'affichage.
QAbstractItemView *RzxRezalDetail::widget()
{
	return this;
}

///Retourne les caractéristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalDetail::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autorisées du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalDetail::allowedAreas() const
{
	return Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea;
}

///Retourne la position par défaut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalDetail::area() const
{
	return Qt::BottomDockWidgetArea;
}

///Retourne l'état par défaut du rezal
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

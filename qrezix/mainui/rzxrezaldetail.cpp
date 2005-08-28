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
#include <QMouseEvent>

#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxApplication>

#include "rzxrezaldetail.h"

#include "rzxrezalmodel.h"
#include "rzxmainuiconfig.h"

RZX_REZAL_EXPORT(RzxRezalDetail)

///Le RzxRezalDetail est un ItemView extrêmement simple
/** Il n'affiche en effet qu'un item... le currentIndex.
 */
RzxRezalDetail::RzxRezalDetail(QWidget *widget)
	:QAbstractItemView(widget), RzxRezal("Item details 1.7.0-svn", "Detail of an item"), computer(NULL), waitProp(NULL)
{
	beginLoading();
	setModel(RzxRezalModel::global());
	setupUi(this);
	setType(TYP_DOCKABLE);
	connect(RzxApplication::instance(), SIGNAL(haveProperties(RzxComputer*, bool*)),
		this, SLOT(propChanged(RzxComputer*,bool*)));

	// Création des en-têtes de colonnes
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, tr("Property"));
	item->setText(1, tr("Value"));
	propsView->setHeaderItem(item);

	endLoading();
}

///Destruction
RzxRezalDetail::~RzxRezalDetail()
{
	beginClosing();
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
	connect(btnProperties, SIGNAL(clicked()), this, SLOT(checkProp()));
}

///Mets à jour l'affichage si nécessaire
void RzxRezalDetail::dataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
	if(currentIndex().row() >= topLeft.row() && currentIndex().row() <= bottomRight.row())
		drawComputer(computer);
}

///Vide l'affichage
void RzxRezalDetail::clear()
{
	lblFtp->clear();
	lblHttp->clear();
	lblNews->clear();
	lblSamba->clear();
	lblIcon->clear();
	lblName->clear();
	lblComment->clear();
	lblOS->clear();
	lblClient->clear();
	lblPromo->clear();
	lblRezal->clear();
	lblIP->clear();
	lblDate->clear();
	lblStateIcon->clear();
	lblState->clear();
	propsView->clear();
	propsView->setEnabled(false);
	btnProperties->setEnabled(false);
	QAbstractItemView::disconnect(btnProperties, SIGNAL(clicked()), 0, 0);
}

///Affichage l'object défini par le RzxComputer
void RzxRezalDetail::drawComputer(RzxComputer *computer)
{
	if(!computer) return;

	lblIcon->setPixmap(computer->icon());
	lblName->setText("<h3>" + computer->name() + "</h3>");
	lblComment->setText(computer->remarque());
	if(computer->hasFtpServer()) lblFtp->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_FTP));
	if(computer->hasHttpServer()) lblHttp->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_HTTP));
	if(computer->hasNewsServer()) lblNews->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_NEWS));
	if(computer->hasSambaServer()) lblSamba->setPixmap(RzxIconCollection::getPixmap(Rzx::ICON_SAMBA));
	lblOS->setPixmap(RzxIconCollection::global()->osPixmap(computer->sysEx(), false));
	lblClient->setText(computer->client());
	lblPromo->setPixmap(RzxIconCollection::global()->promoPixmap(computer->promo()));
	lblRezal->setText(computer->rezalName());
	lblIP->setText(computer->ip().toString());
	lblStateIcon->setPixmap(RzxIconCollection::getPixmap(computer->isOnResponder()?Rzx::ICON_AWAY:Rzx::ICON_HERE));
	lblState->setText(QString(computer->isOnResponder()?tr("away"):tr("connected")));

	// Création des en-têtes de colonnes
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, tr("Property"));
	item->setText(1, tr("Value"));
	propsView->setHeaderItem(item);

	// Remplissage
	propsView->clear();
	btnProperties->setEnabled(true);
	QStringList props = RzxConfig::cache(computer->ip()).split('|');
	if(props.size())
	{
		item = NULL;
		propsView->setEnabled(true);
		for(int i = 0 ; i < props.size() - 1 ; i+=2)
		{
			item = new QTreeWidgetItem(propsView, item);
			item->setText(0, props[i]);
			item->setText(1, props[i+1]);
		}
		lblDate->setText(RzxConfig::getCacheDate(computer->ip()));
	}
	else
		lblDate->setText(tr("Never checked"));
}

///Demande les propriétés de l'object actuel
void RzxRezalDetail::checkProp()
{
	waitProp = computer;
	if(computer)	
		computer->proprietes();
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
	QWidget *child = childAt(e->pos());

	if(child == lblFtp)
		computer->ftp();
	else if(child == lblHttp)
		computer->http();
	else if(child == lblSamba)
		computer->samba();
	else if(child == lblNews)
		computer->news();
	else if(child == lblState || child == lblStateIcon)
		computer->chat();
	else if(child == lblName || child == lblIcon || child == lblIcon)
	{
		if(RzxMainUIConfig::doubleClicRole() && computer->hasFtpServer())
			computer->ftp();
		else if(!computer->isOnResponder() && computer->can(Rzx::CAP_CHAT) && RzxComputer::localhost()->can(Rzx::CAP_CHAT))
			computer->chat();
	}
	else
		QAbstractItemView::mouseDoubleClickEvent(e);
	return;
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

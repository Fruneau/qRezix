/***************************************************************************
                         rzxrezalview  -  description
                            -------------------
   begin                : Mon Jul 18 2005
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
#define RZX_MODULE_NAME "Item view"

#include <QKeyEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QRect>
#include <QPen>
#include <QPainter>

#include <RzxComputer>
#include <RzxConfig>
#include <RzxApplication>

#include "rzxrezalview.h"

#include <RzxRezalModel>
#include <RzxRezalPopup>
#include <RzxRezalDrag>
#include <RzxMainUIConfig>

RZX_REZAL_EXPORT(RzxRezalView)

///Construction du RezalView
RzxRezalView::RzxRezalView( QWidget *parent )
	: QTreeView( parent ), RzxRezal(RZX_MODULE_NAME, QT_TR_NOOP("Historical way to display computers"), RZX_MODULE_VERSION),
		search(this)
{
	beginLoading();
	setType(TYP_ALL);
	setType(TYP_INDEXED);
	setModel(RzxRezalModel::global());
	setIconSize(QSize(32,32));
	header()->setStretchLastSection(false);
	header()->setSortIndicatorShown(true);
	header()->setClickable(true);
	header()->setHighlightSections(false);
	header()->setSortIndicator(RzxMainUIConfig::sortColumn(), RzxMainUIConfig::sortOrder());
	setRootIndex( RzxRezalModel::global()->everybodyGroup );

	setUniformRowHeights(false);
	setAlternatingRowColors(true);

	//Columns stocke les colonnes avec columns[i] == logical index placé en visual index i
	QList<int> columns = RzxMainUIConfig::columnPositions();
	for(int i = 0 ; i < columns.count() && i < RzxRezalModel::numColonnes ; i++)
		header()->moveSection(header()->visualIndex(columns[i]), i);
	
	afficheColonnes();

	connect(&search, SIGNAL(findItem(const QModelIndex& )), this, SLOT(setCurrentIndex(const QModelIndex&)));
	connect(&search, SIGNAL(searchPatternChanged(const QString& )), this, SIGNAL(searchPatternChanged(const QString& )));
	endLoading();
}

///Destruction du RezalView
RzxRezalView::~RzxRezalView()
{
	beginClosing();
	QList<int> columns;
	for(int i = 0 ; i < RzxRezalModel::numColonnes ; i++)
	{
		int log = header()->logicalIndex(i);
		if(log != -1)
			columns << log;
		else
			break;
	}
	RzxMainUIConfig::setColumnPositions(columns);
		
	endClosing();
}

///Gestion des touches du clavier...
/** Le clavier est utilisé dans 2 buts :
 * 	- la recherche par nom
 * 	- affichage du menu contextuel
 */
void RzxRezalView::keyPressEvent( QKeyEvent *e )
{
	QString s = e->text();

	//Menu contextuel
	if(e->key() == Qt::Key_Right)
	{
		QRect r(visualRect(currentIndex()));
		QPoint qp = r.center();
		qp.setY( qp.y() + r.height() );
		new RzxRezalPopup(currentIndex(), mapToGlobal(qp), this);
		return ;
	}

	//Cas normal... rien action par défaut
	if(e->text().isEmpty() && e->key() != Qt::Key_Backspace)
	{
		search.resetPattern();
		QTreeView::keyPressEvent(e);
		return;
	}

	//Recherche
	QString text = e->text();
	//On continue à augmenter le pattern
	if(e->key() == Qt::Key_Backspace)
		search.reducePattern();
	else
		search.addToPattern(text.toLower());
}

///Pour affichier le menu contextuel
/** Ce menu fournit les actions de bases accessibles depuis l'item */
void RzxRezalView::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::RightButton)
		new RzxRezalPopup(indexAt(e->pos()), e->globalPos(), this);
	else
	{
		QModelIndex model = indexAt(e->pos());
		if(!model.isValid()) return;
		RzxComputer *computer = model.model()->data(model, Qt::UserRole).value<RzxComputer*>();
		RzxRezalDrag::mouseEvent(this, e, computer);

		QTreeView::mousePressEvent(e);
	}
}

///Pour lancer le client qui va bien en fonction de la position du clic
void RzxRezalView::mouseDoubleClickEvent(QMouseEvent *e)
{
	QModelIndex model = indexAt(e->pos());
	if(!model.isValid()) return;
	RzxComputer *computer = model.model()->data(model, Qt::UserRole).value<RzxComputer*>();
	if(!computer) return;

	switch(model.column())
	{
		case RzxRezalModel::ColFTP: computer->ftp(); break;
		case RzxRezalModel::ColHTTP: computer->http(); break;
		case RzxRezalModel::ColSamba: computer->samba(); break;
		case RzxRezalModel::ColNews: computer->news(); break;
		default:
			if(RzxMainUIConfig::doubleClicRole() && computer->hasFtpServer())
				computer->ftp();
			else if(!computer->isOnResponder() && computer->can(Rzx::CAP_CHAT) && RzxComputer::localhost()->can(Rzx::CAP_CHAT))
				computer->chat();
	}
	return;
}

///Pour le drag&drop
void RzxRezalView::mouseMoveEvent(QMouseEvent *e)
{
	RzxRezalDrag::mouseEvent(this, e);
	QTreeView::mouseMoveEvent(e);
}

///Pour le drop du drag&drop
void RzxRezalView::dragEnterEvent(QDragEnterEvent *e)
{
	QTreeView::dragEnterEvent(e);
	if(RzxRezalDrag::dragEvent(e))
		e->accept();
}

///Pour le drop du drag&drop
void RzxRezalView::dragMoveEvent(QDragMoveEvent *e)
{
	QTreeView::dragMoveEvent(e);
	const QModelIndex model = indexAt(e->pos());
	const QRect rect = visualRect(model);
	if(RzxRezalDrag::dragEvent(e, model))
		e->accept(rect);
	else
		e->ignore(rect);
}

///Pour le drop :)
void RzxRezalView::dropEvent(QDropEvent *e)
{
	RzxRezalDrag::dropEvent(e, indexAt(e->pos()));
}

///Surcharge le redimensionnement
/** Uniquement dans le but de bien réadapter les colonnes */
void RzxRezalView::resizeEvent(QResizeEvent * e) {
	QTreeView::resizeEvent(e);
	adapteColonnes();
}

///Mise à jour de l'affichage
/** \reimp */
void RzxRezalView::updateLayout()
{
	afficheColonnes();
}

///Affiche les colonnes qui correspondent
void RzxRezalView::afficheColonnes()
{
	int colonnesAffichees = RzxMainUIConfig::colonnes();

	for(int i = 0; i < RzxRezalModel::numColonnes ; i++)
	{
		if((colonnesAffichees>>i) & 1)
		{
			header()->setSectionHidden(i, false);
			switch(i)
			{
				case RzxRezalModel::ColNom:
					header()->resizeSection(i, header()->sectionSizeHint(i));
					break;

				case RzxRezalModel::ColRezal:
					header()->resizeSection(i, 50);
					break;

				case RzxRezalModel::ColClient:
					header()->resizeSection(i, 85);
					break;

				case RzxRezalModel::ColIP:
					header()->resizeSection(i, 120);
					break;

				case RzxRezalModel::ColRemarque: break;
					header()->setResizeMode(i, QHeaderView::Stretch);
					break;

				default:
					header()->resizeSection(i,40);
			}
		}
		else
			header()->setSectionHidden(i, true);
	}
	adapteColonnes();
}

///Adapte la taille de la colonne de remarque
/** La taille est adaptée juste pour que les items prennent la largeur de l'écran... et n'empiètent pas sur
 * la barre de défilement verticale...
 */
void RzxRezalView::adapteColonnes()
{
	int colonnesAffichees = RzxMainUIConfig::colonnes();
	int somme=0;

	for(int i=0 ; i < RzxRezalModel::numColonnes ; i++)
		if(i != RzxRezalModel::ColRemarque && !header()->isSectionHidden(i))
			somme+=header()->sectionSize(i);

	if(((colonnesAffichees >> RzxRezalModel::ColRemarque) & 1)) {
		if(width()>(somme+110))
			header()->resizeSection(RzxRezalModel::ColRemarque, width()-somme-20);
		else
			header()->resizeSection(RzxRezalModel::ColRemarque, 100);
	}
}

///Réimplémente le changement de root
/** Dans le but de ne pas mettre un mettre un objet
 * comme racine de l'affichage
 */
void RzxRezalView::setRootIndex(const QModelIndex& index)
{
	if(model()->data(index, Qt::UserRole).canConvert<RzxComputer*>())
		QTreeView::setRootIndex(index.parent());
	else
		QTreeView::setRootIndex(index);
	setRootIsDecorated(((RzxRezalModel*)model())->isIndex(rootIndex()));
	setAcceptDrops(index == RzxRezalModel::global()->favoriteIndex ||
		index == RzxRezalModel::global()->ignoredIndex ||
		index == RzxRezalModel::global()->neutralIndex ||
		index == RzxRezalModel::global()->favoritesGroup);

}

///Dessine la ligne
/** Réimplémente la fonction dans le simple but de régler les problèmes de couleur de fond... */
void RzxRezalView::drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem newoption = option;
	QVariant variant = model()->data(index, Qt::BackgroundColorRole);

	if(variant.canConvert<QColor>())
	{
#define setColor(org, dest) \
	newoption.palette.setBrush(dest, QColor(variant.value<QColor>().rgba() & option.palette.org().color().rgba()))
		setColor(highlight, QPalette::Highlight);
		setColor(base, QPalette::Base);
		setColor(alternateBase, QPalette::AlternateBase);
#undef setColor
	}

	QTreeView::drawRow(painter, newoption, index);
}

///Retourne une fenêtre utilisable pour l'affichage.
QAbstractItemView *RzxRezalView::widget()
{
	return this;
}

///Retourne les caractéristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalView::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autorisées du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalView::allowedAreas() const
{
	return Qt::AllDockWidgetAreas;
}

///Retourne la position par défaut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalView::area() const
{
	return Qt::TopDockWidgetArea;
}

///Retourne l'état par défaut du rezal
bool RzxRezalView::floating() const
{
	return false;
}

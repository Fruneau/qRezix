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
#include <QKeyEvent>
#include <QHeaderView>
#include <QMouseEvent>

#include "../core/rzxcomputer.h"
#include "../core/rzxconfig.h"

#include "rzxrezalview.h"

#include "rzxrezalmodel.h"
#include "rzxrezalpopup.h"


///Construction du RezalView
RzxRezalView::RzxRezalView( QWidget *parent )
	: QTreeView( parent ), search(this)
{
	setModel(RzxRezalModel::global());
	setIconSize(QSize(32,32));
	header()->setStretchLastSection(false);
	header()->setSortIndicatorShown(true);
	header()->setClickable(true);
	header()->setHighlightSections(false);
	setRootIndex( RzxRezalModel::global()->everybodyGroup );

	setRootIsDecorated(false);
	setUniformRowHeights(false);
	setAlternatingRowColors(true);

	afficheColonnes();

	connect(&search, SIGNAL(findItem(const QModelIndex& )), this, SLOT(setCurrentIndex(const QModelIndex&)));
	connect(&search, SIGNAL(searchPatternChanged(const QString& )), this, SIGNAL(searchPatternChanged(const QString& )));
}

///Destruction du RezalView
RzxRezalView::~RzxRezalView()
{}

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
	if(e->text().isNull() && e->key() != Qt::Key_Backspace)
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
		QTreeView::mousePressEvent(e);
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
			if(RzxConfig::global()->doubleClicRole() && computer->hasFtpServer())
				computer->ftp();
			else if(!computer->isOnResponder() && computer->can(Rzx::CAP_CHAT) && RzxComputer::localhost()->can(Rzx::CAP_CHAT))
				computer->chat();
	}
	return;
}

///Surcharge le redimensionnement
/** Uniquement dans le but de bien réadapter les colonnes */
void RzxRezalView::resizeEvent(QResizeEvent * e) {
	QTreeView::resizeEvent(e);
	adapteColonnes();
}

///Affiche les colonnes qui correspondent
void RzxRezalView::afficheColonnes()
{
	int colonnesAffichees = RzxConfig::colonnes();

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
	int colonnesAffichees = RzxConfig::colonnes();
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

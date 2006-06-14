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
#define RZX_MODULE_NAME "List"
#define RZX_MODULE_DESCRIPTION "Historical way to display computers"
#define RZX_MODULE_ICON RzxThemedIcon("rzlitem")

#include <QKeyEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QRect>
#include <QPen>
#include <QPainter>
#include <QBitmap>
#include <QScrollBar>

#include <RzxComputer>
#include <RzxConfig>
#include <RzxApplication>
#include <RzxConnectionLister>
#include <RzxTranslator>

#include "rzxrezalview.h"
#include "rzxrezalviewconfig.h"

#include <RzxRezalModel>
#include <RzxRezalPopup>
#include <RzxRezalDrag>
#include <RzxRezalAction>
#include <RzxMainUIConfig>

#include "ui_rzxrezalviewprop.h"

RZX_REZAL_EXPORT(RzxRezalView)
RZX_CONFIG_INIT(RzxRezalViewConfig)

///Construction du RezalView
RzxRezalView::RzxRezalView( QWidget *parent )
	: QTreeView( parent ), RzxRezal(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Historical way to display computers"), RZX_MODULE_VERSION),
		search(this), ui(NULL), propWidget(NULL)
{
	beginLoading();
	RzxRezalViewConfig::global();
	setType(TYP_ALL);
	setType(TYP_INDEXED);
	setIcon(RZX_MODULE_ICON);
	setModel(RzxRezalModel::global());
	setIconSize(QSize(32,32));
	header()->setStretchLastSection(false);
	header()->setSortIndicatorShown(true);
	header()->setClickable(true);
	header()->setHighlightSections(false);
	header()->setSortIndicator(RzxMainUIConfig::sortColumn(), RzxMainUIConfig::sortOrder());
	connect(header(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(columnOrderChanged()));
	setAcceptDrops(true);

	setUniformRowHeights(true);
	setAlternatingRowColors(true);

	//Columns stocke les colonnes avec columns[i] == logical index placé en visual index i
	QList<int> columns = RzxRezalViewConfig::columnPositions();
	for(int i = 0 ; i < columns.count() && i < RzxRezalModel::nbColonnes() ; i++)
		header()->moveSection(header()->visualIndex(columns[i]), i);
	
	afficheColonnes();

	connect(&search, SIGNAL(findItem(const QModelIndex& )), this, SLOT(findIndex(const QModelIndex&)));
	connect(&search, SIGNAL(searchPatternChanged(const QString& )), this, SIGNAL(searchPatternChanged(const QString& )));
	RzxIconCollection::connect(this, SLOT(themeChanged()));
	RzxTranslator::connect(this, SLOT(translate()));

	force = true;
	connect(RzxConnectionLister::global(), SIGNAL(initialLoging(bool)), this, SLOT(setDelayRefresh(bool)));

	delayRedraw.start();
	endLoading();
}

///Destruction du RezalView
RzxRezalView::~RzxRezalView()
{
	beginClosing();
	delete RzxRezalViewConfig::global();
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
		if(!RzxRezalModel::global()->isComputer(currentIndex()))
		{
			QTreeView::keyPressEvent(e);
			return;
		}
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
	if(!computer)
	{
		setRootIndex(model);
		return;
	}

	switch(model.column())
	{
		case RzxRezalModel::ColFTP: computer->ftp(); break;
		case RzxRezalModel::ColHTTP: computer->http(); break;
		case RzxRezalModel::ColSamba: computer->samba(); break;
		case RzxRezalModel::ColNews: computer->news(); break;
		default: RzxRezalAction::run(computer);
	}
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
	if(!RzxRezalDrag::dragEvent(e).isEmpty())
		e->accept();
}

///Pour le drop du drag&drop
void RzxRezalView::dragMoveEvent(QDragMoveEvent *e)
{
	QTreeView::dragMoveEvent(e);
	const QModelIndex model = indexAt(e->pos());
	const QRect rect = visualRect(model);
	if(!RzxRezalDrag::dragEvent(e, model).isEmpty())
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
void RzxRezalView::resizeEvent(QResizeEvent * e)
{
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
	int colonnesAffichees = RzxRezalViewConfig::colonnes();

	for(int i = 0; i < RzxRezalModel::nbColonnes() ; i++)
	{
		if((colonnesAffichees>>i) & 1)
		{
			header()->setSectionHidden(i, false);
			switch(i)
			{
				case RzxRezalModel::ColNom:
				case RzxRezalModel::ColName:
				case RzxRezalModel::ColFirstName:
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
					header()->resizeSection(i, 40);
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
	const int limit = viewport()->width();
	int colonnesAffichees = RzxRezalViewConfig::colonnes();
	int somme = 0;

	for(int i=0 ; i < RzxRezalModel::nbColonnes() ; i++)
		if(i != RzxRezalModel::ColRemarque && !header()->isSectionHidden(i))
			somme += header()->sectionSize(i);

	if(((colonnesAffichees >> RzxRezalModel::ColRemarque) & 1)) {
		if(limit > somme + 110)
			header()->resizeSection(RzxRezalModel::ColRemarque, limit - somme);
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
}

///Change l'index courant
/** Mets en particulier à jour le root index si le changement a modifié
 */
void RzxRezalView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	QTreeView::currentChanged(current, previous);
	
	if(!current.isValid() || rootIndex() == current.parent() || 
		(current.parent() == rootIndex().parent() && current.row() == rootIndex().row()))
	{
		if(force)
			forceRefresh();
		return;
	}
	if(current.parent() == rootIndex().parent() || (!rootIndex().isValid() && current.parent().isValid()))
		setRootIndex(current.parent());
}

///Indique si l'index donné est visible
bool RzxRezalView::isVisible(const QModelIndex& index) const
{
	const QRect rect = visualRect(index);
	return (rect.bottom() >= 0 || rect.top() < viewport()->height()) && rect.top() < rect.bottom();
}

///Sélection l'index trouvé par la recherche
void RzxRezalView::findIndex(const QModelIndex& index)
{
	setCurrentIndex(index);
	scrollTo(index, PositionAtTop);
}

///Mets à jour l'affichage lorsque des objets sont insérés
void RzxRezalView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTreeView::rowsInserted(parent, start, end);

	//Pour éviter de boulétiser le proc, on ne met à jour qu'une fois toutes les secondes maximum
	if(force || delayRedraw.elapsed() > 1000)
		forceRefresh();
}

///Indique si on doit afficher les nouvelles connexion immédiatement ou au contraire ralentir le rafraichissement
void RzxRezalView::setDelayRefresh(bool delay)
{
	force = !delay;
	if(force)
		forceRefresh();
}

///Force le rafraichissement de la fenêtre
void RzxRezalView::forceRefresh()
{
	const bool viewCurrent = isVisible(currentIndex());
	QTreeView::setRootIndex(rootIndex());
	if(viewCurrent)
		scrollTo(currentIndex());
	delayRedraw.restart();
}

///Dessine la ligne
/** Réimplémente la fonction dans le simple but de régler les problèmes de couleur de fond... */
void RzxRezalView::drawRow(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem newoption = option;
	const QVariant variant = model()->data(index, Qt::BackgroundColorRole);

	if(variant.canConvert<QColor>())
	{
		const qint32 rgba = variant.value<QColor>().rgba();
#define setColor(org, dest) \
	newoption.palette.setBrush(dest, QColor(rgba & option.palette.org().color().rgba()))
		setColor(highlight, QPalette::Highlight);
		setColor(base, QPalette::Base);
		setColor(alternateBase, QPalette::AlternateBase);
#undef setColor
	}

	if(isVisible(index))
		QTreeView::drawRow(painter, newoption, index);
}

///Pour éviter que les branches soient ouvertes lors de la suppression d'une ligne...
void RzxRezalView::rowsRemoved(const QModelIndex& parent, int, int)
{
	if(isExpanded(parent))
	{
		setExpanded(parent, false);
		setExpanded(parent, true);
	}
}

///Pour éviter que les branches soient ouvertes lors de la suppression d'une ligne...
void RzxRezalView::rowsAboutToBeRemoved(const QModelIndex&, int, int)
{
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

/** \reimp */
QList<QWidget*> RzxRezalView::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxRezalViewProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect(ui->btnMoveUp, SIGNAL(clicked()), this, SLOT(moveUp()));
		connect(ui->btnMoveDown, SIGNAL(clicked()), this, SLOT(moveDown()));
		connect(ui->btnInit, SIGNAL(clicked()), this, SLOT(reinitialisedOrder()));
		ui->btnInit->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		themeChanged();
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxRezalView::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxRezalView::propInit(bool def)
{
	RzxRezalModel::Colonnes colonnes = (RzxRezalModel::ColonneFlags)RzxRezalViewConfig::colonnes(def);
	ui->cbcNom ->setChecked( colonnes & RzxRezalModel::ColNomFlag );
	ui->cbcRemarque->setChecked( colonnes & RzxRezalModel::ColRemarqueFlag );
	ui->cbcSamba ->setChecked( colonnes & RzxRezalModel::ColSambaFlag );
	ui->cbcFTP ->setChecked( colonnes & RzxRezalModel::ColFTPFlag );
	ui->cbcHTTP ->setChecked( colonnes & RzxRezalModel::ColHTTPFlag );
	ui->cbcNews ->setChecked( colonnes & RzxRezalModel::ColNewsFlag );
	ui->cbcPrinter ->setChecked( colonnes & RzxRezalModel::ColPrinterFlag );
	ui->cbcOS ->setChecked( colonnes & RzxRezalModel::ColOSFlag );
	ui->cbcGateway ->setChecked( colonnes & RzxRezalModel::ColGatewayFlag );
	ui->cbcPromo ->setChecked( colonnes & RzxRezalModel::ColPromoFlag );
	ui->cbcResal ->setChecked( colonnes & RzxRezalModel::ColRezalFlag );
	ui->cbcIP ->setChecked( colonnes & RzxRezalModel::ColIPFlag );
	ui->cbcClient ->setChecked( colonnes & RzxRezalModel::ColClientFlag );
	ui->cbcClient ->setChecked( colonnes & RzxRezalModel::ColNameFlag );
	ui->cbcClient ->setChecked( colonnes & RzxRezalModel::ColFirstNameFlag );

	if(!saveColumns.isEmpty())
		for(int i = 0 ; i < saveColumns.count() && i < RzxRezalModel::nbColonnes() ; i++)
			header()->moveSection(header()->visualIndex(saveColumns[i]), i);
	else
		saveColumns = columnOrder();
	dispColumns(saveColumns);
}

/** \reimp */
void RzxRezalView::propUpdate()
{
	if(!ui) return;

	RzxRezalModel::Colonnes colonnesAffichees;
	if ( ui->cbcNom ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColNomFlag;
	if ( ui->cbcRemarque->isChecked() ) colonnesAffichees |= RzxRezalModel::ColRemarqueFlag;
	if ( ui->cbcSamba ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColSambaFlag;
	if ( ui->cbcFTP ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColFTPFlag;
	if ( ui->cbcHTTP ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColHTTPFlag;
	if ( ui->cbcNews ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColNewsFlag;
	if ( ui->cbcPrinter ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColPrinterFlag;
	if ( ui->cbcOS ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColOSFlag;
	if ( ui->cbcGateway ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColGatewayFlag;
	if ( ui->cbcPromo ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColPromoFlag;
	if ( ui->cbcResal ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColRezalFlag;
	if ( ui->cbcClient ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColClientFlag;
	if ( ui->cbcIP ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColIPFlag;
	if ( ui->cbcName ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColNameFlag;
	if ( ui->cbcFirstName ->isChecked() ) colonnesAffichees |= RzxRezalModel::ColFirstNameFlag;
	RzxRezalViewConfig::setColonnes(colonnesAffichees);
	saveColumns = QList<int>();
	updateLayout();
}

/** \reimp */
void RzxRezalView::propClose()
{
	if(propWidget)
	{
		delete propWidget;
		propWidget = NULL;
	}
	if(ui)
	{
		delete ui;
		ui = NULL;
	}
}

///Retourne l'ordre des colonnes
QList<int> RzxRezalView::columnOrder() const
{
	QList<int> columns;
	for(int i = 0 ; i < RzxRezalModel::nbColonnes() ; i++)
	{
		int log = header()->logicalIndex(i);
		if(log != -1)
			columns << log;
		else
			break;
	}
	return columns;
}

///En cas de déplacement de colonnes
void RzxRezalView::columnOrderChanged()
{
	RzxRezalViewConfig::setColumnPositions(columnOrder());
	if(ui && propWidget)
		dispColumns(columnOrder());
}

///Affichage des colonnes dans la listView qvb
void RzxRezalView::dispColumns(const QList<int>& columns)
{
	if(!ui || !propWidget) return;

	ui->lstColumns->clear();
	QPixmap emptyPix(16, 16);
	emptyPix.fill(Qt::white);
	emptyPix.setMask(emptyPix.createMaskFromColor(Qt::white));
	for(int i = 0 ; i < columns.size() ; i++)
	{
		QListWidgetItem *item = new QListWidgetItem(ui->lstColumns);
		item->setText(RzxRezalModel::global()->columnName((RzxRezalModel::NumColonne)columns[i]));
		const QIcon icon = RzxRezalModel::global()->headerData(columns[i], Qt::Horizontal, Qt::DecorationRole).value<QIcon>();
		if(!icon.isNull())
			item->setIcon(icon);
		else
			item->setIcon(emptyPix);
	}
}

///Déplace l'item sélectionné vers le bas
void RzxRezalView::moveDown()
{
	const int org = ui->lstColumns->currentRow();
	if(org < 0) return;

	int dst = org + 1;
	if(dst >= RzxRezalModel::nbColonnes()) dst = 0;
	header()->moveSection(org, dst);
	ui->lstColumns->setCurrentRow(dst);
}

///Déplace l'item sélectionné vers le haut
void RzxRezalView::moveUp()
{
	const int org = ui->lstColumns->currentRow();
	if(org < 0) return;

	int dst = org - 1;
	if(dst < 0) dst = RzxRezalModel::nbColonnes() - 1;
	header()->moveSection(org, dst);
	ui->lstColumns->setCurrentRow(dst);
}

///Remet les colonnes dans l'ordre d'origine
void RzxRezalView::reinitialisedOrder()
{
	for(int i = 0 ; i < RzxRezalModel::nbColonnes() ; i++)
		header()->moveSection(header()->visualIndex(i), i);
}

///Change les icônes
void RzxRezalView::themeChanged()
{
	if(!ui || !propWidget) return;

	dispColumns(columnOrder());
	ui->btnInit->setIcon(RzxIconCollection::getIcon(Rzx::ICON_RELOAD));
}

///Mise à jour de la traduction
void RzxRezalView::translate()
{
	if(!ui) return;

	ui->retranslateUi(propWidget);
	dispColumns(columnOrder());
}

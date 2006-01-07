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
	setIcon(RzxThemedIcon("rzlitem"));
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

	//Columns stocke les colonnes avec columns[i] == logical index plac� en visual index i
	QList<int> columns = RzxRezalViewConfig::columnPositions();
	for(int i = 0 ; i < columns.count() && i < RzxRezalModel::numColonnes ; i++)
		header()->moveSection(header()->visualIndex(columns[i]), i);
	
	afficheColonnes();

	connect(&search, SIGNAL(findItem(const QModelIndex& )), this, SLOT(setCurrentIndex(const QModelIndex&)));
	connect(&search, SIGNAL(searchPatternChanged(const QString& )), this, SIGNAL(searchPatternChanged(const QString& )));
	RzxIconCollection::connect(this, SLOT(themeChanged()));
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
/** Le clavier est utilis� dans 2 buts :
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

	//Cas normal... rien action par d�faut
	if(e->text().isEmpty() && e->key() != Qt::Key_Backspace)
	{
		search.resetPattern();
		QTreeView::keyPressEvent(e);
		return;
	}

	//Recherche
	QString text = e->text();
	//On continue � augmenter le pattern
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
/** Uniquement dans le but de bien r�adapter les colonnes */
void RzxRezalView::resizeEvent(QResizeEvent * e)
{
	QTreeView::resizeEvent(e);
	adapteColonnes();
}

///Mise � jour de l'affichage
/** \reimp */
void RzxRezalView::updateLayout()
{
	afficheColonnes();
}

///Affiche les colonnes qui correspondent
void RzxRezalView::afficheColonnes()
{
	int colonnesAffichees = RzxRezalViewConfig::colonnes();

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
/** La taille est adapt�e juste pour que les items prennent la largeur de l'�cran... et n'empi�tent pas sur
 * la barre de d�filement verticale...
 */
void RzxRezalView::adapteColonnes()
{
	int colonnesAffichees = RzxRezalViewConfig::colonnes();
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

///R�impl�mente le changement de root
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

///Mets � jour l'affichage lorsque des objets sont ins�r�s
void RzxRezalView::rowsInserted(const QModelIndex& parent, int start, int end)
{
	QTreeView::rowsInserted(parent, start, end);

	//Pour �viter de boul�tiser le proc, on ne met � jour qu'une fois toutes les secondes maximum
	if(delayRedraw.elapsed() > 1000)
	{
		QTreeView::setRootIndex(rootIndex());
		scrollTo(currentIndex());
		delayRedraw.restart();
	}
}

///Dessine la ligne
/** R�impl�mente la fonction dans le simple but de r�gler les probl�mes de couleur de fond... */
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

///Retourne une fen�tre utilisable pour l'affichage.
QAbstractItemView *RzxRezalView::widget()
{
	return this;
}

///Retourne les caract�ristiques du rezal en tant que dock
QDockWidget::DockWidgetFeatures RzxRezalView::features() const
{
	return QDockWidget::AllDockWidgetFeatures;
}

///Retourne les positions autoris�es du rezal en tant que dock
Qt::DockWidgetAreas RzxRezalView::allowedAreas() const
{
	return Qt::AllDockWidgetAreas;
}

///Retourne la position par d�faut du rezal en tant que dock
Qt::DockWidgetArea RzxRezalView::area() const
{
	return Qt::TopDockWidgetArea;
}

///Retourne l'�tat par d�faut du rezal
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
	uint colonnes = RzxRezalViewConfig::colonnes(def);
	ui->cbcNom ->setChecked( colonnes & (1<<RzxRezalModel::ColNom) );
	ui->cbcRemarque->setChecked( colonnes & (1<<RzxRezalModel::ColRemarque) );
	ui->cbcSamba ->setChecked( colonnes & (1<<RzxRezalModel::ColSamba) );
	ui->cbcFTP ->setChecked( colonnes & (1<<RzxRezalModel::ColFTP) );
	ui->cbcHTTP ->setChecked( colonnes & (1<<RzxRezalModel::ColHTTP) );
	ui->cbcNews ->setChecked( colonnes & (1<<RzxRezalModel::ColNews) );
	ui->cbcPrinter ->setChecked( colonnes & (1<<RzxRezalModel::ColPrinter) );
	ui->cbcOS ->setChecked( colonnes & (1<<RzxRezalModel::ColOS) );
	ui->cbcGateway ->setChecked( colonnes & (1<<RzxRezalModel::ColGateway) );
	ui->cbcPromo ->setChecked( colonnes & (1<<RzxRezalModel::ColPromo) );
	ui->cbcResal ->setChecked( colonnes & (1<<RzxRezalModel::ColRezal) );
	ui->cbcIP ->setChecked( colonnes & (1<<RzxRezalModel::ColIP) );
	ui->cbcClient ->setChecked( colonnes & (1<<RzxRezalModel::ColClient) );

	if(!saveColumns.isEmpty())
		for(int i = 0 ; i < saveColumns.count() && i < RzxRezalModel::numColonnes ; i++)
			header()->moveSection(header()->visualIndex(saveColumns[i]), i);
	else
		saveColumns = columnOrder();
	dispColumns(saveColumns);
}

/** \reimp */
void RzxRezalView::propUpdate()
{
	if(!ui) return;

	int colonnesAffichees = 0;
	if ( ui->cbcNom ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNom;
	if ( ui->cbcRemarque->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRemarque;
	if ( ui->cbcSamba ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColSamba;
	if ( ui->cbcFTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColFTP;
	if ( ui->cbcHTTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColHTTP;
	if ( ui->cbcNews ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNews;
	if ( ui->cbcPrinter ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColPrinter;
	if ( ui->cbcOS ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColOS;
	if ( ui->cbcGateway ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColGateway;
	if ( ui->cbcPromo ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColPromo;
	if ( ui->cbcResal ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRezal;
	if ( ui->cbcClient ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColClient;
	if ( ui->cbcIP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColIP;
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
	for(int i = 0 ; i < RzxRezalModel::numColonnes ; i++)
	{
		int log = header()->logicalIndex(i);
		if(log != -1)
			columns << log;
		else
			break;
	}
	return columns;
}

///En cas de d�placement de colonnes
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
	for(int i = 0 ; i < columns.size() ; i++)
		ui->lstColumns->insertItem(i, RzxRezalModel::global()->columnName((RzxRezalModel::NumColonne)columns[i]));
}

///D�place l'item s�lectionn� vers le bas
void RzxRezalView::moveDown()
{
	const int org = ui->lstColumns->currentRow();
	if(org < 0) return;

	int dst = org + 1;
	if(dst >= RzxRezalModel::numColonnes) dst = 0;
	header()->moveSection(org, dst);
	ui->lstColumns->setCurrentRow(dst);
}

///D�place l'item s�lectionn� vers le haut
void RzxRezalView::moveUp()
{
	const int org = ui->lstColumns->currentRow();
	if(org < 0) return;

	int dst = org - 1;
	if(dst < 0) dst = RzxRezalModel::numColonnes - 1;
	header()->moveSection(org, dst);
	ui->lstColumns->setCurrentRow(dst);
}

///Remet les colonnes dans l'ordre d'origine
void RzxRezalView::reinitialisedOrder()
{
	for(int i = 0 ; i < RzxRezalModel::numColonnes ; i++)
		header()->moveSection(header()->visualIndex(i), i);
}

///Change les ic�nes
void RzxRezalView::themeChanged()
{
	if(!ui || !propWidget) return;

	ui->btnInit->setIcon(RzxIconCollection::getIcon(Rzx::ICON_RELOAD));
}

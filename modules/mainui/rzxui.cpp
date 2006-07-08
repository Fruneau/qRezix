/***************************************************************************
                                 rzxui.cpp
          Interface du module pour l'linterface principale de qRezix
                             -------------------
    begin                : Sat Aug 6 2005
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define RZX_MODULE_NAME "Interface"
#define RZX_MODULE_DESCRIPTION "Main UI for qRezix"
#define RZX_MODULE_ICON RzxThemedIcon("mainui")

#include <QWidget>
#include <QPainter>
#include <QBrush>
#include <QPen>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxIconCollection>
#include <RzxTranslator>
#include <RzxApplication>
#include <RzxComputer>

#include "rzxui.h"

///Exporte le module
RZX_MODULE_EXPORT(RzxUi)

#include "qrezix.h"
#include "rzxusergroup.h"
#include "rzxrezalmodel.h"
#include "rzxmainuiconfig.h"
#include "ui_rzxmainuiprop.h"
#include "ui_rzxgroupsprop.h"

///Initialise la configuration
RZX_CONFIG_INIT(RzxMainUIConfig)

/** \reimp */
RzxUi::RzxUi()
	:RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Main UI for qRezix"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(MOD_MAINGUI);
	ui = NULL;
	groupsUi = NULL;
	groupsWidget = propWidget = NULL;
	RzxUserGroup::loadGroups(new RzxMainUIConfig(this));
	qrezix = QRezix::global();
	connect(qrezix, SIGNAL(wantQuit()), this, SIGNAL(wantQuit()));
	connect(qrezix, SIGNAL(wantPreferences()), this, SIGNAL(wantPreferences()));
	connect(qrezix, SIGNAL(wantToggleResponder()), this, SIGNAL(wantToggleResponder()));
	connect(qrezix, SIGNAL(wantReload()), this, SLOT(reload()), Qt::QueuedConnection);
	RzxIconCollection::connect(this, SLOT(fillComboBoxes()));
	RzxTranslator::connect(this, SLOT(fillComboBoxes()));
	setIcon(RZX_MODULE_ICON);
	endLoading();
}

/** \reimp */
RzxUi::~RzxUi()
{
	beginClosing();
	delete qrezix;
	delete RzxRezalModel::global();
	RzxUserGroup::clearGroups();
	delete RzxMainUIConfig::global();
	endClosing();
}

/** \reimp */
bool RzxUi::isInitialised() const
{
	return qrezix->isInitialised();
}

/** \reimp */
void RzxUi::show()
{
	qrezix->show();
	if(qrezix->isMinimized())
		qrezix->showNormal();
}

/** \reimp */
void RzxUi::hide()
{
	qrezix->hide();
}

/** \reimp */
void RzxUi::toggleVisible()
{
	qrezix->toggleVisible();
}

/** \reimp */
QList<RzxBaseModule*> RzxUi::childModules() const
{
	QList<RzxRezal*> rezals = qrezix->moduleList();
	QList<RzxBaseModule*> modules;
	foreach(RzxRezal *rezal, rezals)
		modules << rezal;
	return modules;
}

/** \reimp */
QWidget *RzxUi::mainWindow() const
{
	return qrezix;
}

/** \reimp */
QList<QWidget*> RzxUi::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxMainuiProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		ui->rezalLoader->setLoader(qrezix);
		connect(ui->cbTopLeft, SIGNAL(activated(int)), this, SLOT(redrawCorners()));
		connect(ui->cbBottomLeft, SIGNAL(activated(int)), this, SLOT(redrawCorners()));
		connect(ui->cbTopRight, SIGNAL(activated(int)), this, SLOT(redrawCorners()));
		connect(ui->cbBottomRight, SIGNAL(activated(int)), this, SLOT(redrawCorners()));
	}
	if(!groupsUi)
		groupsUi = new Ui::RzxGroupsProp;
	if(!groupsWidget)
	{
		groupsWidget = new QWidget;
		groupsUi->setupUi(groupsWidget);
		connect(groupsUi->btnAdd, SIGNAL(clicked()), this, SLOT(addGroup()));
		connect(groupsUi->btnDelete, SIGNAL(clicked()), this, SLOT(deleteGroup()));
		connect(groupsUi->lstGroups, SIGNAL(itemSelectionChanged()), this, SLOT(fillMemberBox()));
		connect(groupsUi->lstGroups, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(fillMemberBox()));
		connect(groupsUi->newGroup, SIGNAL(textChanged(const QString&)), this, SLOT(newGroupEdited()));
	}
	fillComboBoxes();
	return QList<QWidget*>() << propWidget << groupsWidget;
}

/** \reimp */
QStringList RzxUi::propWidgetsName()
{
	return QStringList() << name() << tr("User groups");
}

/** \reimp */
void RzxUi::propInit(bool def)
{
	if(!ui) return;

	disconnect(this, SLOT(enterPressed(bool&)));
	RzxProperty *prop = qobject_cast<RzxProperty*>(groupsWidget->window());
	if(prop)
		connect(prop, SIGNAL(enterPressed(bool&)), this, SLOT(enterPressed(bool&)));

	QList<RzxRezal*> rezals = qrezix->moduleList();
	foreach(RzxRezal *rezal, rezals)
		rezal->propInit(def);

	ui->cmdDoubleClic->setCurrentIndex( RzxMainUIConfig::doubleClicRole(def) );
	ui->cmbDefaultTab->setCurrentIndex(RzxMainUIConfig::defaultTab(def));
	if(RzxMainUIConfig::defaultTab(def) >= RzxMainUIConfig::FirstGroup)
	{
		int i = ui->cmbDefaultTab->findText(RzxMainUIConfig::defaultTabGroup(def));
		if(i == -1)
			i = 0;
		ui->cmbDefaultTab->setCurrentIndex(i);
	}

	uint tooltip = RzxMainUIConfig::tooltip(def);
	ui->cbTooltips->setChecked(tooltip & RzxRezalModel::TipEnable);
	ui->cbTooltipFtp->setChecked(tooltip & RzxRezalModel::TipFtp);
	ui->cbTooltipHttp->setChecked(tooltip & RzxRezalModel::TipHttp);
	ui->cbTooltipNews->setChecked(tooltip & RzxRezalModel::TipNews);
	ui->cbTooltipSamba->setChecked(tooltip & RzxRezalModel::TipSamba);
	ui->cbTooltipPrinter->setChecked(tooltip & RzxRezalModel::TipPrinter);
	ui->cbTooltipPromo->setChecked(tooltip & RzxRezalModel::TipPromo);
	ui->cbTooltipOS->setChecked(tooltip & RzxRezalModel::TipOS);
	ui->cbTooltipVersion->setChecked(tooltip & RzxRezalModel::TipClient);
	ui->cbTooltipIP->setChecked(tooltip & RzxRezalModel::TipIP);
	ui->cbTooltipResal->setChecked(tooltip & RzxRezalModel::TipResal);
	ui->cbTooltipFeatures->setChecked(tooltip & RzxRezalModel::TipFeatures);
	ui->cbTooltipProperties->setChecked(tooltip & RzxRezalModel::TipProperties);

	ui->cbSearch->setChecked( RzxMainUIConfig::useSearch(def) );
	ui->cbQuit->setChecked(RzxMainUIConfig::showQuit());

	ui->cbTopLeft->setCurrentIndex(RzxMainUIConfig::topLeftCorner() == Qt::LeftDockWidgetArea ? 1 : 0);
	ui->cbBottomLeft->setCurrentIndex(RzxMainUIConfig::bottomLeftCorner() == Qt::LeftDockWidgetArea ? 1 : 0);
	ui->cbTopRight->setCurrentIndex(RzxMainUIConfig::topRightCorner() == Qt::RightDockWidgetArea ? 1 : 0);
	ui->cbBottomRight->setCurrentIndex(RzxMainUIConfig::bottomRightCorner() == Qt::RightDockWidgetArea ? 1 : 0);
	redrawCorners();
	fillGroupsBox();
}

///Construit les combos box
void RzxUi::fillComboBoxes()
{
	if(groupsUi && groupsWidget)
	{
		groupsUi->btnDelete->setIcon(RzxIconCollection::getIcon(Rzx::ICON_UNLOAD));
		groupsUi->btnAdd->setIcon(RzxIconCollection::getIcon(Rzx::ICON_LOAD));
		fillGroupsBox();

		groupsUi->cbIcon->clear();
		for(int i = 0 ; i < Rzx::ICON_NUMBER ; i++)
		{
			const QIcon icon = RzxIconCollection::getIcon((Rzx::Icon)i);
			if(!icon.isNull())
				groupsUi->cbIcon->addItem(icon, " ", RzxIconCollection::getIconName((Rzx::Icon)i));
		}
		groupsUi->cbIcon->setCurrentIndex(groupsUi->cbIcon->findData("favorite"));
	}
	if(!ui || !propWidget) return;

	int i;

	// Choix de l'affichage
	i = ui->cmbDefaultTab->currentIndex();
	ui->cmbDefaultTab->clear();
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Favorites"));
	ui->cmbDefaultTab->addItem(RzxThemedIcon(RzxComputer::localhost()->promo()), tr("My Promo"));
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), tr("My Subnet"));
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Everybody"));
	const int groupNumber = RzxUserGroup::groupNumber();
	for(int i = 0 ; i < groupNumber ; i++)
	{
		const RzxUserGroup *group = RzxUserGroup::group(i);
		ui->cmbDefaultTab->addItem(group->icon(), group->name());
	}
	if(i >= ui->cmbDefaultTab->count())
		i = RzxMainUIConfig::defaultTab(true);
	ui->cmbDefaultTab->setCurrentIndex(i);

	// Choix de l'action
	i = ui->cmdDoubleClic->currentIndex();
	ui->cmdDoubleClic->clear();
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_CHAT), tr("Chat"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_FTP), tr("Open FTP"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_HTTP), tr("Open Web page"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_NEWS), tr("Read News"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_SAMBA), tr("Open Samba"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_MAIL), tr("Send a Mail"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_PROPRIETES), tr("Check Properties"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_HISTORIQUE), tr("View History"));
	ui->cmdDoubleClic->setCurrentIndex(i);

	int tl, tr, bl, br;
	tl = ui->cbTopLeft->currentIndex();
	ui->cbTopLeft->clear();
	tr = ui->cbTopRight->currentIndex();
	ui->cbTopRight->clear();
	bl = ui->cbBottomLeft->currentIndex();
	ui->cbBottomLeft->clear();
	br = ui->cbBottomRight->currentIndex();
	ui->cbBottomRight->clear();
	ui->retranslateUi(propWidget);
	ui->cbTopLeft->setCurrentIndex(tl);
	ui->cbTopRight->setCurrentIndex(tr);
	ui->cbBottomLeft->setCurrentIndex(bl);
	ui->cbBottomRight->setCurrentIndex(br);
}

///Construit l'image des coins
/** Pour donner un aperçu de l'utilisation des coins de l'ui par les 
 * rezals qui y sont
 */
void RzxUi::redrawCorners()
{
	corners = QPixmap(80, 60);
	QPainter painter(&corners);
	const QBrush brush = QBrush(Qt::darkGray);
	const QPen pen = QPen(Qt::black);

	painter.setBrush(brush);
	painter.setPen(pen);
	painter.drawRect(0, 0, 79, 59);

	drawRect(painter, Qt::blue, 20, 1, 39, 18);
	drawRect(painter, Qt::blue, 20, 40, 39, 18);
	drawRect(painter, Qt::red, 1, 20, 18, 19);
	drawRect(painter, Qt::red, 60, 20, 18, 19);

	drawCorner(painter, ui->cbTopLeft, 1, 1);
	drawCorner(painter, ui->cbBottomLeft, 1, 40);
	drawCorner(painter, ui->cbTopRight, 60, 1);
	drawCorner(painter, ui->cbBottomRight, 60, 40);

	ui->lblCorners->setPixmap(corners);
}

///Dessine un coin en fonction des données de la combobox
void RzxUi::drawCorner(QPainter &painter, QComboBox *cb, int x, int y)
{
	const QColor color = cb->currentIndex() ? Qt::red : Qt::blue;
	drawRect(painter, color, x, y, 18, 18);
}

///Dessine un rectange de couleur donnée
void RzxUi::drawRect(QPainter &painter, const QColor& color, int x, int y, int w, int h)
{
	const QBrush brush = QBrush(color);
	const QPen pen = QPen(color);
	painter.setBrush(brush);
	painter.setPen(pen);
	painter.drawRect(x, y, w, h);
}

///Rempli la liste des groupes existants
void RzxUi::fillGroupsBox()
{
	groupsUi->lstGroups->clear();
	const int groupNumber = RzxUserGroup::groupNumber();
	for(int i = 0 ; i < groupNumber ; i++)
	{
		QListWidgetItem *item = new QListWidgetItem(groupsUi->lstGroups);
		const RzxUserGroup *group = RzxUserGroup::group(i);
		item->setText(group->name());
		item->setIcon(group->icon());
		item->setData(Qt::UserRole, i);
	}
	groupsUi->lstGroups->sortItems();
	groupsUi->lstMembers->setList(NULL);
}

///Lorsque la touche Enter est appuyée...
void RzxUi::enterPressed(bool& used)
{
	if(used) return;

	if(groupsUi->btnAdd->isEnabled() && (groupsUi->newGroup->hasFocus() || groupsUi->btnAdd->hasFocus()))
	{
		used = true;
		addGroup();
	}
}

///On a appuyé sur add ==> on ajoute un groupe
void RzxUi::addGroup()
{
	const QString icon = groupsUi->cbIcon->itemData(groupsUi->cbIcon->currentIndex()).toString();
	const QString name = groupsUi->newGroup->text().simplified();
	const QList<QListWidgetItem*> list = groupsUi->lstGroups->findItems(name, Qt::MatchExactly);
	if(list.isEmpty())
	{
		new RzxUserGroup(icon, groupsUi->newGroup->text());
		RzxRezalModel::global()->newUserGroup();
		groupsUi->newGroup->clear();
	}
	else
	{
		const int groupId = list[0]->data(Qt::UserRole).toInt();
		RzxUserGroup::group(groupId)->setIcon(icon);
	}
	fillComboBoxes();
}

///On a décidé de supprimer un groupe :(
void RzxUi::deleteGroup()
{
	if (!groupsUi->lstGroups->currentItem())
		return;

	const int groupId = groupsUi->lstGroups->currentItem()->data(Qt::UserRole).toInt();
	if(groupsUi->lstGroups->currentItem()->text() == groupsUi->newGroup->text())
		groupsUi->newGroup->clear();
	RzxRezalModel::global()->deleteUserGroup(groupId);
	RzxUserGroup::deleteGroup(groupId);
	fillComboBoxes();
	groupsUi->btnDelete->setEnabled(false);
}

///Le texte de la zone d'édition a changé...
/** On active le bouton 'add' a condition que le texte soit non vide,
 * commence par un caractère raisonnable, et n'existe pas encore
 * dans la liste des groups
 */
void RzxUi::newGroupEdited()
{
	const QString text = groupsUi->newGroup->text().simplified();
	groupsUi->btnAdd->setDisabled(text.isEmpty());
}

///Mets à jour la liste des membres
void RzxUi::fillMemberBox()
{
	if(!groupsUi->lstGroups->currentItem())
	{
		groupsUi->btnDelete->setEnabled(false);
		groupsUi->lstMembers->setList(NULL);
		groupsUi->lstMembers->setEnabled(false);
		return;
	}

	const int groupId = groupsUi->lstGroups->currentItem()->data(Qt::UserRole).toInt();
	groupsUi->btnDelete->setEnabled(true);
	groupsUi->lstMembers->setEnabled(true);
	groupsUi->lstMembers->setList(RzxUserGroup::group(groupId));
	if(groupsUi->newGroup->text().isEmpty())
		groupsUi->newGroup->setText(groupsUi->lstGroups->currentItem()->text());
}

/** \reimp */
void RzxUi::propUpdate()
{
	if(!ui) return;

	RzxMainUIConfig::setTopLeftCorner(ui->cbTopLeft->currentIndex()?Qt::LeftDockWidgetArea :Qt::TopDockWidgetArea);
	RzxMainUIConfig::setBottomLeftCorner(ui->cbBottomLeft->currentIndex()?Qt::LeftDockWidgetArea :Qt::BottomDockWidgetArea);
	RzxMainUIConfig::setTopRightCorner(ui->cbTopRight->currentIndex()?Qt::RightDockWidgetArea :Qt::TopDockWidgetArea);
	RzxMainUIConfig::setBottomRightCorner(ui->cbBottomRight->currentIndex()?Qt::RightDockWidgetArea :Qt::BottomDockWidgetArea);
	RzxMainUIConfig::setDoubleClicRole(ui->cmdDoubleClic->currentIndex());
	RzxMainUIConfig::setDefaultTab(ui->cmbDefaultTab ->currentIndex());
	RzxMainUIConfig::setDefaultTabGroup(ui->cmbDefaultTab ->currentText());
	RzxMainUIConfig::setUseSearch(ui->cbSearch->isChecked());
	RzxMainUIConfig::setShowQuit(ui->cbQuit->isChecked());
	qrezix->buildToolbar(ui->cbSearch->isChecked());

	RzxRezalModel::ToolTip tooltip = 0;
	if(ui->cbTooltips->isChecked()) tooltip |= RzxRezalModel::TipEnable;
	if(ui->cbTooltipFtp->isChecked()) tooltip |= RzxRezalModel::TipFtp;
	if(ui->cbTooltipHttp->isChecked()) tooltip |= RzxRezalModel::TipHttp;
	if(ui->cbTooltipNews->isChecked()) tooltip |= RzxRezalModel::TipNews;
	if(ui->cbTooltipPrinter->isChecked()) tooltip |= RzxRezalModel::TipPrinter;
	if(ui->cbTooltipSamba->isChecked()) tooltip |= RzxRezalModel::TipSamba;
	if(ui->cbTooltipPromo->isChecked()) tooltip |= RzxRezalModel::TipPromo;
	if(ui->cbTooltipOS->isChecked()) tooltip |= RzxRezalModel::TipOS;
	if(ui->cbTooltipVersion->isChecked()) tooltip |= RzxRezalModel::TipClient;
	if(ui->cbTooltipIP->isChecked()) tooltip |= RzxRezalModel::TipIP;
	if(ui->cbTooltipResal->isChecked()) tooltip |= RzxRezalModel::TipResal;
	if(ui->cbTooltipFeatures->isChecked()) tooltip |= RzxRezalModel::TipFeatures;
	if(ui->cbTooltipProperties->isChecked()) tooltip |= RzxRezalModel::TipProperties;
	RzxMainUIConfig::setTooltip(tooltip);

	/* Mise à jour de l'affichage des Rezal */
	qrezix->updateLayout();
}

/** \reimp */
void RzxUi::propClose()
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

/** \reimp */
void RzxUi::setTreeItem(QTreeWidgetItem *item)
{
	ui->rezalLoader->setPropertyParent(item);
}

///Lance un rechargement léger du module
/** Ce rechargement ne fait que détruire le QRezix et le reconstruire
 */
void RzxUi::reload()
{
	if(qrezix)
	{
		beginClosing();
		if(ui)
		{
			const QList<RzxRezal*> rezals = qrezix->moduleList();
			foreach(RzxRezal *rezal, rezals)
				ui->rezalLoader->emitNotifyUnload(rezal->name());
		}
		delete qrezix;
		endClosing();
	}
	beginLoading();
	qrezix = QRezix::global();
	connect(qrezix, SIGNAL(wantQuit()), this, SIGNAL(wantQuit()));
	connect(qrezix, SIGNAL(wantPreferences()), this, SIGNAL(wantPreferences()));
	connect(qrezix, SIGNAL(wantToggleResponder()), this, SIGNAL(wantToggleResponder()));
	connect(qrezix, SIGNAL(wantReload()), this, SLOT(reload()), Qt::QueuedConnection);
	if(ui)
	{
		ui->rezalLoader->setLoader(qrezix);
		const QList<RzxRezal*> rezals = qrezix->moduleList();
		foreach(RzxRezal *rezal, rezals)
			ui->rezalLoader->emitNotifyLoad(rezal->name());
	}
	endLoading();
}

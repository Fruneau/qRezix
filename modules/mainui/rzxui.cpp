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
#define RZX_MODULE_NAME "Main UI"
#define RZX_MODULE_DESCRIPTION "Main UI for qRezix"
#define RZX_MODULE_ICON Rzx::ICON_SYSTRAYHERE

#include <QWidget>

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
#include "rzxrezalmodel.h"
#include "rzxmainuiconfig.h"
#include "ui_rzxmainuiprop.h"

///Initialise la configuration
RZX_CONFIG_INIT(RzxMainUIConfig)

/** \reimp */
RzxUi::RzxUi()
	:RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Main UI for qRezix"), RZX_MODULE_VERSION)
{
	beginLoading();
	setType(MOD_MAINGUI);
	ui = NULL;
	propWidget = NULL;
	new RzxMainUIConfig(this);
	qrezix = QRezix::global();
	connect(qrezix, SIGNAL(wantQuit()), this, SIGNAL(wantQuit()));
	connect(qrezix, SIGNAL(wantPreferences()), this, SIGNAL(wantPreferences()));
	connect(qrezix, SIGNAL(wantToggleResponder()), this, SIGNAL(wantToggleResponder()));
	connect(qrezix, SIGNAL(wantReload()), this, SLOT(reload()));
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
		fillComboBoxes();
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxUi::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxUi::propInit(bool def)
{
	ui->cmdDoubleClic->setCurrentIndex( RzxMainUIConfig::doubleClicRole(def) );
	ui->cmbDefaultTab->setCurrentIndex(RzxMainUIConfig::defaultTab(def));

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
}

///Construit les combos box
void RzxUi::fillComboBoxes()
{
	if(!ui || !propWidget) return;

	int i;

	// Choix de l'affichage
	i = ui->cmbDefaultTab->currentIndex();
	ui->cmbDefaultTab->clear();
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE), tr("Favorites"));
	ui->cmbDefaultTab->addItem(RzxThemedIcon(RzxComputer::localhost()->promo()), tr("My Promo"));
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_SAMEGATEWAY), tr("My Subnet"));
	ui->cmbDefaultTab->addItem(RzxIconCollection::getIcon(Rzx::ICON_NOTFAVORITE), tr("Everybody"));
	ui->cmbDefaultTab->setCurrentIndex(i);

	// Choix de l'action
	i = ui->cmdDoubleClic->currentIndex();
	ui->cmdDoubleClic->clear();
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_CHAT), tr("Chat"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_FTP), tr("Open FTP"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_HTTP), tr("Open Web page"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_NEWS), tr("Read News"));
	ui->cmdDoubleClic->addItem(RzxIconCollection::getIcon(Rzx::ICON_SAMBA), tr("Open Samba"));
	ui->cmdDoubleClic->setCurrentIndex(i);
}

/** \reimp */
void RzxUi::propUpdate()
{
	if(!ui) return;

	RzxMainUIConfig::setDoubleClicRole(ui->cmdDoubleClic->currentIndex());
	RzxMainUIConfig::setDefaultTab(ui->cmbDefaultTab ->currentIndex());
	RzxMainUIConfig::setUseSearch(ui->cbSearch->isChecked());
	RzxMainUIConfig::setShowQuit(ui->cbQuit->isChecked());
	qrezix->showSearch(ui->cbSearch->isChecked());

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
		delete qrezix;
		endClosing();
	}
	beginLoading();
	qrezix = QRezix::global();
	connect(qrezix, SIGNAL(wantQuit()), this, SIGNAL(wantQuit()));
	connect(qrezix, SIGNAL(wantPreferences()), this, SIGNAL(wantPreferences()));
	connect(qrezix, SIGNAL(wantToggleResponder()), this, SIGNAL(wantToggleResponder()));
	connect(qrezix, SIGNAL(wantReload()), this, SLOT(reload()));
	endLoading();
}

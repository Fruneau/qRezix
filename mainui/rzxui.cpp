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
#include <QWidget>

#include <RzxGlobal>
#include <RzxConfig>
#include <RzxIconCollection>

#include "rzxui.h"

#include "qrezix.h"
#include "rzxrezalmodel.h"
#include "ui_rzxmainuipropui.h"

#ifdef RZX_MAINUI_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

///Exporte le module
RZX_MODULE_EXPORT(RzxUi)

/** \reimp */
RzxUi::RzxUi()
	:RzxModule("Main UI 1.7.0-svn", QT_TR_NOOP("Main UI for qRezix"))
{
	beginLoading();
	setType(MOD_MAINUI);
	ui = NULL;
	propWidget = NULL;
	qrezix = QRezix::global();
	connect(qrezix, SIGNAL(wantQuit()), this, SIGNAL(wantQuit()));
	connect(qrezix, SIGNAL(wantPreferences()), this, SIGNAL(wantPreferences()));
	connect(qrezix, SIGNAL(wantToggleResponder()), this, SIGNAL(wantToggleResponder()));
	endLoading();
}

/** \reimp */
RzxUi::~RzxUi()
{
	beginClosing();
	delete qrezix;
	endClosing();
}

/** \reimp */
bool RzxUi::isInitialised() const
{
	return qrezix->isInitialised();
}

/** \reimp */
QIcon RzxUi::icon() const
{
	return RzxIconCollection::getIcon(Rzx::ICON_SYSTRAYHERE);
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
QWidget *RzxUi::mainWindow() const
{
	return qrezix;
}

/** \reimp */
QList<QWidget*> RzxUi::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxMainuiPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxUi::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxUi::propInit()
{
	ui->cmbIconSize->setCurrentIndex( RzxConfig::computerIconSize() );
	ui->cmdDoubleClic->setCurrentIndex( RzxConfig::doubleClicRole() );
	ui->cmbDefaultTab -> setCurrentIndex(RzxConfig::defaultTab());

	int colonnes = RzxConfig::colonnes();
	ui->cbcNom ->setChecked( colonnes & (1<<RzxRezalModel::ColNom) );
	ui->cbcRemarque->setChecked( colonnes & (1<<RzxRezalModel::ColRemarque) );
	ui->cbcSamba ->setChecked( colonnes & (1<<RzxRezalModel::ColSamba) );
	ui->cbcFTP ->setChecked( colonnes & (1<<RzxRezalModel::ColFTP) );
	ui->cbcHTTP ->setChecked( colonnes & (1<<RzxRezalModel::ColHTTP) );
	ui->cbcNews ->setChecked( colonnes & (1<<RzxRezalModel::ColNews) );
	ui->cbcOS ->setChecked( colonnes & (1<<RzxRezalModel::ColOS) );
	ui->cbcGateway ->setChecked( colonnes & (1<<RzxRezalModel::ColGateway) );
	ui->cbcPromo ->setChecked( colonnes & (1<<RzxRezalModel::ColPromo) );
	ui->cbcResal ->setChecked( colonnes & (1<<RzxRezalModel::ColRezal) );
	ui->cbcIP ->setChecked( colonnes & (1<<RzxRezalModel::ColIP) );
	ui->cbcClient ->setChecked( colonnes & (1<<RzxRezalModel::ColClient) );
	ui->cbQuit->setChecked(RzxConfig::showQuit());
	ui->cbHighlight->setChecked(RzxConfig::computerIconHighlight());

	ui->cbSearch->setChecked( RzxConfig::global() ->useSearch() );
}

/** \reimp */
void RzxUi::propUpdate()
{
	if(!ui) return;

	RzxConfig *cfgObject = RzxConfig::global();
	cfgObject->writeEntry( "doubleClic", ui->cmdDoubleClic->currentIndex() );
	cfgObject->writeEntry( "iconsize", ui->cmbIconSize -> currentIndex() );
	cfgObject->writeEntry( "iconhighlight", ui->cbHighlight->isChecked());
	cfgObject->writeEntry( "defaultTab", ui->cmbDefaultTab ->currentIndex() );
	cfgObject->writeEntry( "useSearch", ui->cbSearch->isChecked() ? 1 : 0 );
	cfgObject->writeShowQuit(ui->cbQuit->isChecked());
	qrezix->showSearch(ui->cbSearch->isChecked());

	int colonnesAffichees = 0;
	if ( ui->cbcNom ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNom;
	if ( ui->cbcRemarque->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRemarque;
	if ( ui->cbcSamba ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColSamba;
	if ( ui->cbcFTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColFTP;
	if ( ui->cbcHTTP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColHTTP;
	if ( ui->cbcNews ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColNews;
	if ( ui->cbcOS ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColOS;
	if ( ui->cbcGateway ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColGateway;
	if ( ui->cbcPromo ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColPromo;
	if ( ui->cbcResal ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColRezal;
	if ( ui->cbcClient ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColClient;
	if ( ui->cbcIP ->isChecked() ) colonnesAffichees |= 1<<RzxRezalModel::ColIP;
	RzxConfig::global() ->writeEntry( "colonnes", colonnesAffichees );

	/* Mise à jour de l'affichage des Rezal */
	qrezix -> rezal -> afficheColonnes();
	qrezix -> rezalFavorites -> afficheColonnes();
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

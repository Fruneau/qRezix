/***************************************************************************
                          rzxnotifier  -  description
                             -------------------
    begin                : Sun Jul 31 2005
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
#define RZX_MODULE_NAME "Favorite Notifier"
#define RZX_MODULE_DESCRIPTION "Notify that a favorite state has changed"
#define RZX_MODULE_ICON Rzx::ICON_FAVORITE

#include <RzxGlobal>
#include <RzxComputer>
#include <RzxConnectionLister>
#include <RzxIconCollection>
#include <RzxProperty>
#include <RzxSound>
#include <RzxApplication>
#include <RzxTranslator>

#include "rzxnotifier.h"

#include "rzxtraywindow.h"
#include "ui_rzxnotifierprop.h"
#include "rzxnotifierconfig.h"

///Exporte le module
RZX_MODULE_EXPORT(RzxNotifier)
RZX_CONFIG_INIT(RzxNotifierConfig)

///Contruction du notifier
/** La construction consiste juste à mettre en place les connexions qvb
 */
RzxNotifier::RzxNotifier()
	:RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Notify that a favorite state has changed"), RZX_MODULE_VERSION)
{
	beginLoading();
	favoriteWarn = false;
	ui = NULL;
	propWidget = NULL;
	setType(MOD_GUI);
	setIcon(RZX_MODULE_ICON);
	new RzxNotifierConfig(this);
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(initialLoging(bool)), this, SLOT(ignoreLoging(bool)));
	RzxTranslator::connect(this, SLOT(translate()));
	if(RzxConnectionLister::global()->computerNumber())
		ignoreLoging(false);
	endLoading();
}

///Destruction vide...
RzxNotifier::~RzxNotifier()
{
	beginClosing();
	delete RzxNotifierConfig::global();
	endClosing();
}

/** \reimp */
bool RzxNotifier::isInitialised() const
{
	return true;
}

///Prend en note le fait que les connexions initiales sont terminées
void RzxNotifier::ignoreLoging(bool ignore)
{
	favoriteWarn = (!ignore);
}

///Connecte le nouvel ordinateur
void RzxNotifier::login(RzxComputer *computer)
{
	connect(computer, SIGNAL(favoriteStateChanged(RzxComputer*)), this, SLOT(favoriteUpdated(RzxComputer* )));
	if(computer && computer->isFavorite() && !computer->isLocalhost() && RzxNotifierConfig::notifyHere())
		favoriteUpdated(computer);
}

/// Pour l'affichage d'une notification lors de la connexion d'un favoris
/** Permet d'alerter lors de l'arrivée d'un favoris, que ce soit par l'émission d'un son, ou par l'affichage l'affichage d'un message indiquant la présence de ce favoris */
void RzxNotifier::favoriteUpdated(RzxComputer *computer)
{
	//ne garde que les favoris avec en plus comme condition que ce ne soit pas les gens présents à la connexion
	//evite de notifier la présence de favoris si en fait c'est nous qui arrivons.
	if(!favoriteWarn || (RzxComputer::localhost()->isOnResponder() && RzxNotifierConfig::notNotifyWhenILeave()))
		return;
	
	//Bah, beep à la connexion
	if(RzxNotifierConfig::beepConnection() && computer->state() == Rzx::STATE_HERE)
		RzxSound::play(RzxNotifierConfig::beepSound());
	
	//Affichage de la fenêtre de notification de connexion
	if(RzxNotifierConfig::showConnection())
	{
		switch(computer->state())
		{
			case Rzx::STATE_DISCONNECTED:
				if(!RzxNotifierConfig::notifyDisconnection()) return;
				break;
			case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
				if(!RzxNotifierConfig::notifyAway()) return;
				break;
			 case Rzx::STATE_HERE:
				if(!RzxNotifierConfig::notifyHere()) return;
				break;
		}
		new RzxTrayWindow((RzxTrayWindow::Theme)RzxNotifierConfig::windowStyle(), computer);
	}
}

/** \reimp */
QList<QWidget*> RzxNotifier::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxNotifierProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect( ui->btnBeepBrowse, SIGNAL( clicked()), this, SLOT(chooseBeepConnection())) ;
		connect( ui->btnTestStyle, SIGNAL(clicked()), this, SLOT(showTestWindow()));
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxNotifier::propWidgetsName()
{
	return QStringList() << name();
}

/** \reimp */
void RzxNotifier::propInit(bool def)
{
	ui->cbWarnFavorite->setChecked(RzxNotifierConfig::showConnection(def));
	ui->chkBeepFavorites->setChecked(RzxNotifierConfig::beepConnection(def));
	ui->txtBeepFavorites->setText(RzxNotifierConfig::beepSound(def));
	ui->cbHere->setChecked(RzxNotifierConfig::notifyHere(def));
	ui->cbDisconnect->setChecked(RzxNotifierConfig::notifyDisconnection(def));
	ui->cbAway->setChecked(RzxNotifierConfig::notifyAway(def));
	ui->cbILeave->setChecked(RzxNotifierConfig::notNotifyWhenILeave(def));

	ui->cbStyle->clear();
	ui->cbStyle->addItem(tr("Nice style - transparent window"));
	ui->cbStyle->addItem(tr("Old style - like qRezix 1.6"));
	ui->cbStyle->setCurrentIndex(RzxNotifierConfig::windowStyle()-1);
}

/** \reimp */
void RzxNotifier::propUpdate()
{
	if(!ui) return;

	RzxNotifierConfig::setBeepConnection(ui->chkBeepFavorites->isChecked());
	RzxNotifierConfig::setShowConnection(ui->cbWarnFavorite->isChecked());
	RzxNotifierConfig::setBeepSound(ui->txtBeepFavorites->text());
	RzxNotifierConfig::setNotifyHere(ui->cbHere->isChecked());
	RzxNotifierConfig::setNotifyDisconnection(ui->cbDisconnect->isChecked());
	RzxNotifierConfig::setNotifyAway(ui->cbAway->isChecked());
	RzxNotifierConfig::setNotNotifyWhenILeave(ui->cbILeave->isChecked());
	RzxNotifierConfig::setWindowStyle(ui->cbStyle->currentIndex()+1);
}

/** \reimp */
void RzxNotifier::propClose()
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

///Affiche une fenêtre de test du style courant
void RzxNotifier::showTestWindow() const
{
	if(!ui) return;

	new RzxTrayWindow((RzxTrayWindow::Theme)(ui->cbStyle->currentIndex() +1), RzxComputer::localhost());
}

///Choix du son à jouer à la connexion d'un favoris
void RzxNotifier::chooseBeepConnection()
{
	QString file = RzxProperty::browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	ui->txtBeepFavorites -> setText(file);
}

///Mise à jour de la traduction
void RzxNotifier::translate()
{
	if(ui)
		ui->retranslateUi(propWidget);

	int id = ui->cbStyle->currentIndex();
	ui->cbStyle->clear();
	ui->cbStyle->addItem(tr("Nice style - transparent window"));
	ui->cbStyle->addItem(tr("Old style - like qRezix 1.6"));
	ui->cbStyle->setCurrentIndex(id);
}

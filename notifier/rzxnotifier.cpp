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
#include <QProcess>
#include <QSound>

#include <RzxGlobal>
#include <RzxComputer>
#include <RzxConnectionLister>
#include <RzxIconCollection>
#include <RzxProperty>

#include "rzxnotifier.h"

#include "rzxtraywindow.h"
#include "ui_rzxnotifierpropui.h"
#include "rzxnotifierconfig.h"

///Exporte le module
RZX_MODULE_EXPORT(RzxNotifier)
RZX_CONFIG_INIT(RzxNotifierConfig)

///Contruction du notifier
/** La construction consiste juste à mettre en place les connexions qvb
 */
RzxNotifier::RzxNotifier()
	:RzxModule("Favorite notifier 1.7.0-svn", QT_TR_NOOP("Notify that a favorite state has changed"))
{
	beginLoading();
	favoriteWarn = false;
	ui = NULL;
	propWidget = NULL;
	setType(MOD_GUI);
	setIcon(Rzx::ICON_FAVORITE);
	new RzxNotifierConfig(this);
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(loginEnd()), this, SLOT(loginEnd()));
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
void RzxNotifier::loginEnd()
{
	favoriteWarn = true;
}

///Connecte le nouvel ordinateur
void RzxNotifier::login(RzxComputer *computer)
{
	connect(computer, SIGNAL(favoriteStateChanged(RzxComputer*)), this, SLOT(favoriteUpdated(RzxComputer* )));
	if(computer && computer->isFavorite())
		favoriteUpdated(computer);
}

/// Pour l'affichage d'une notification lors de la connexion d'un favoris
/** Permet d'alerter lors de l'arrivée d'un favoris, que ce soit par l'émission d'un son, ou par l'affichage l'affichage d'un message indiquant la présence de ce favoris */
void RzxNotifier::favoriteUpdated(RzxComputer *computer)
{
	//ne garde que les favoris avec en plus comme condition que ce ne soit pas les gens présents à la connexion
	//evite de notifier la présence de favoris si en fait c'est nous qui arrivons.
	if(!favoriteWarn)
		return;
	
	//Bah, beep à la connexion
	if(RzxNotifierConfig::beepConnection() && computer->state() == Rzx::STATE_HERE)
	{
#if defined (WIN32) || defined (Q_OS_MAC)
		QString file = RzxNotifierConfig::beepSound();
		if( !file.isEmpty() && QFile(file).exists() )
			QSound::play( file );
		else
			QApplication::beep();
#else
		QString cmd = RzxConfig::beepCmd(), file = RzxNotifierConfig::beepSound();
		if (!cmd.isEmpty() && !file.isEmpty()) {
			QProcess process;
			process.start(cmd, QStringList(file));
		}
#endif
	}
	
	//Affichage de la fenêtre de notification de connexion
	if(RzxNotifierConfig::showConnection())
		new RzxTrayWindow(computer);
}

/** \reimp */
QList<QWidget*> RzxNotifier::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxNotifierPropUI;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
		connect( ui->btnBeepBrowse, SIGNAL( clicked()), this, SLOT(chooseBeepConnection())) ;
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
}

/** \reimp */
void RzxNotifier::propUpdate()
{
	if(!ui) return;

	RzxNotifierConfig::setBeepConnection(ui->chkBeepFavorites->isChecked());
	RzxNotifierConfig::setShowConnection(ui->cbWarnFavorite->isChecked());
	RzxNotifierConfig::setBeepSound(ui->txtBeepFavorites->text());
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

///Choix du son à jouer à la connexion d'un favoris
void RzxNotifier::chooseBeepConnection()
{
	QString file = RzxProperty::browse(tr("All files"), tr("Sound file selection"), "*");
	if (file.isEmpty()) return;

	ui->txtBeepFavorites -> setText(file);
}

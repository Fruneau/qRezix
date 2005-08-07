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

#ifdef RZX_NOTIFIER_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

///Exporte le module
RZX_MODULE_EXPORT(RzxNotifier)

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
	connect(RzxConnectionLister::global(), SIGNAL(login(RzxComputer* )), this, SLOT(login(RzxComputer* )));
	connect(RzxConnectionLister::global(), SIGNAL(loginEnd()), this, SLOT(loginEnd()));
	endLoading();
}

///Destruction vide...
RzxNotifier::~RzxNotifier()
{
	beginClosing();
	endClosing();
}

///Indique si l'objet est bien initialisé
bool RzxNotifier::isInitialised() const
{
	return true;
}

/** \reimp */
QIcon RzxNotifier::icon() const
{
	return RzxIconCollection::getIcon(Rzx::ICON_FAVORITE);
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
	if(RzxConfig::beepConnection() && computer->state() == Rzx::STATE_HERE)
	{
#if defined (WIN32) || defined (Q_OS_MAC)
		QString file = RzxConfig::connectionSound();
		if( !file.isEmpty() && QFile(file).exists() )
			QSound::play( file );
		else
			QApplication::beep();
#else
		QString cmd = RzxConfig::beepCmd(), file = RzxConfig::connectionSound();
		if (!cmd.isEmpty() && !file.isEmpty()) {
			QProcess process;
			process.start(cmd, QStringList(file));
		}
#endif
	}
	
	//Affichage de la fenêtre de notification de connexion
	if(RzxConfig::showConnection())
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
void RzxNotifier::propInit()
{
	ui->cbWarnFavorite->setChecked( RzxConfig::showConnection());
	ui->chkBeepFavorites->setChecked( RzxConfig::beepConnection());
}

/** \reimp */
void RzxNotifier::propUpdate()
{
	if(!ui) return;

	RzxConfig *cfgObject = RzxConfig::global();
	cfgObject -> writeEntry( "beepConnection", ui->chkBeepFavorites->isChecked());
	cfgObject -> writeEntry( "showConnection", ui->cbWarnFavorite->isChecked());
	cfgObject -> writeEntry( "txtBeepConnection", ui->txtBeepFavorites->text());
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

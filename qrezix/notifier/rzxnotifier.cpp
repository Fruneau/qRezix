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

#include "../core/rzxglobal.h"
#include "../core/rzxcomputer.h"
#include "../core/rzxconnectionlister.h"

#include "rzxnotifier.h"

#include "rzxtraywindow.h"

///Contruction du notifier
/** La construction consiste juste à mettre en place les connexions qvb
 */
RzxNotifier::RzxNotifier()
	:RzxModule("Favorite notifier 1.7.0-svn", QT_TR_NOOP("Notify that a favorite state has changed"))
{
	beginLoading();
	favoriteWarn = false;
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

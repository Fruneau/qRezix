/***************************************************************************
                               rzxtraywindow.cpp
         Gestion des fenêtres popups de notification de la trayicon
                             -------------------
    begin                : Tue Nov 16 2004
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

#include <qlabel.h>
#include <qpixmap.h>
#include <qlayout.h>

#include "rzxtraywindow.h"

#include "rzxcomputer.h"
#include "trayicon.h"
#include "qrezix.h"

///Construction de la fenêtre de notification d'état de connexion de computer
/** La fenêtre est construite pour disparaître automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(RzxComputer* computer, unsigned int time)
	:QFrame(NULL, "TrayWindow", WStyle_Customize | WStyle_StaysOnTop | WDestructiveClose)
{
	setMinimumWidth(150);
	setMinimumHeight(70);

	//insertion de l'icône
	// Layout, pour le resize libre
	QHBoxLayout *layout = new QHBoxLayout(this, 3, 2, "HTrayWindowLayout");
	QVBoxLayout *textLayout = new QVBoxLayout(this, 0, 0, "VTrayWindowLayout");

	QLabel *icone = new QLabel(this, "TrayWindowIcon");
	QLabel *name = new QLabel(this, "TrayWindowName");
	QLabel *description = new QLabel(this, "TrayWindowDesc");

	layout->addWidget(icone, 0, AlignVCenter | AlignLeft);
	textLayout->addWidget(name, 0, AlignTop | AlignLeft);
	textLayout->addWidget(description, 0, AlignVCenter | AlignLeft);
	layout->addLayout(textLayout, 1);
	
	icone->setPixmap(computer->getIcon());
	name->setTextFormat(RichText);
	name->setText("<h2>" + computer->getName() + "</h2>");
	description->setText(computer->getRepondeur() ? tr("is connected but away") : tr("is connected and here"));

	QPoint point(0,0);
	move(point);
	
	raise();
	show();
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(close()));
	timer.start(time * 1000);
}

///Destruction de la fenêtre
/** Rien à faire ici */
RzxTrayWindow::~RzxTrayWindow()
{
}
/***************************************************************************
                               rzxtraywindow.cpp
         Gestion des fen�tres popups de notification de la trayicon
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
#include <qcolor.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3Frame>
#include <QHBoxLayout>

#include "rzxtraywindow.h"

#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "trayicon.h"
#include "qrezix.h"

///Construction de la fen�tre de notification d'�tat de connexion de computer
/** La fen�tre est construite pour dispara�tre automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(RzxComputer* computer, bool connected, unsigned int time)
	:Q3Frame(NULL, "TrayWindow", Qt::WStyle_Customize | Qt::WStyle_StaysOnTop | Qt::WDestructiveClose)
{
	setMinimumWidth(150);
	setMinimumHeight(70);

	setFrameStyle(Panel | Plain);
	if(!connected)
		setPaletteBackgroundColor(QColor(0xff, 0x20, 0x20));
	else if(computer->getRepondeur())
		setPaletteBackgroundColor(RzxConfig::repondeurBase());
	else
		setPaletteBackgroundColor(0xffffff);
	
	
	//insertion de l'ic�ne
	// Layout, pour le resize libre
	QHBoxLayout *layout = new QHBoxLayout(this, 3, 2, "HTrayWindowLayout");
	QVBoxLayout *textLayout = new QVBoxLayout(this, 0, 0, "VTrayWindowLayout");

	QLabel *icone = new QLabel(this, "TrayWindowIcon");
	QLabel *name = new QLabel(this, "TrayWindowName");
	QLabel *description = new QLabel(this, "TrayWindowDesc");

	layout->addWidget(icone, 0, Qt::AlignVCenter | Qt::AlignLeft);
	textLayout->addWidget(name, 0, Qt::AlignTop | Qt::AlignLeft);
	textLayout->addWidget(description, 0, Qt::AlignVCenter | Qt::AlignLeft);
	layout->addLayout(textLayout, 1);
	
	icone->setPixmap(computer->getIcon());
	name->setTextFormat(Qt::RichText);
	name->setText("<h2>" + computer->getName() + "</h2>");
	if(connected)
		description->setText(computer->getRepondeur() ? tr("is now away") : tr("is now here"));
	else
		description->setText(tr("is now disconnected"));

	QPoint point(0,0);
	move(point);
	
	show();
	
	connect(&timer, SIGNAL(timeout()), this, SLOT(close()));
	timer.start(time * 1000);
}

///Destruction de la fen�tre
/** Rien � faire ici */
RzxTrayWindow::~RzxTrayWindow()
{
}

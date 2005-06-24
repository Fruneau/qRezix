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

#include <QLabel>
#include <QPixmap>
#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "rzxtraywindow.h"

#include "rzxcomputer.h"
#include "rzxconfig.h"
#include "trayicon.h"
#include "qrezix.h"

///Construction de la fen�tre de notification d'�tat de connexion de computer
/** La fen�tre est construite pour dispara�tre automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(RzxComputer* computer, bool connected, unsigned int time)
	:QFrame(NULL, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
	setMinimumWidth(150);
	setMinimumHeight(70);

	setFrameStyle(Panel | Plain);
	
	if(!connected)
		setPaletteBackgroundColor(QColor(0xff, 0x20, 0x20));
	else if(computer->getRepondeur())
		setPaletteBackgroundColor(RzxConfig::repondeurBase());
	else
		setPaletteBackgroundColor(0xffffff);
	
    //Construction des items � ins�rer dans la fen�tre :
    // - l'ic�ne	
	QLabel *icone = new QLabel();
	icone->setPixmap(computer->getIcon());

    // - le pseudo
	QLabel *name = new QLabel();
	name->setTextFormat(Qt::RichText);
	name->setText("<h2>" + computer->getName() + "</h2>");

    // - la description de l'�tat de connexion
	QLabel *description = new QLabel();
	if(connected)
		description->setText(computer->getRepondeur() ? tr("is now away") : tr("is now here"));
	else
		description->setText(tr("is now disconnected"));

	//Insertion des �l�ments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget(name, 0, Qt::AlignTop | Qt::AlignLeft);
	textLayout->addWidget(description, 0, Qt::AlignVCenter | Qt::AlignLeft);

    // - insertion de l'ic�ne � c�t� du texte
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(icone, 0, Qt::AlignVCenter | Qt::AlignLeft);
	layout->addLayout(textLayout, 1);
	setLayout(layout);

    //Affichage de la fen�tre
	QPoint point(0,0);
	move(point);
	setFocusPolicy(Qt::NoFocus);
	show();
	
	//Timer pour d�clencher la destruction de le fen�tre
	connect(&timer, SIGNAL(timeout()), this, SLOT(close()));
	timer.start(time * 1000);
}

///Destruction de la fen�tre
/** Rien � faire ici */
RzxTrayWindow::~RzxTrayWindow()
{
}

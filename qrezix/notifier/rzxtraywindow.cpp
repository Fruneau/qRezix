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
#include <QLabel>
#include <QPixmap>
#include <QBitmap>
#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

#include <RzxComputer>
#include <RzxConfig>

#include "rzxtraywindow.h"


///Construction de la fenêtre de notification d'état de connexion de computer
/** La fenêtre est construite pour disparaître automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow( RzxComputer* computer, unsigned int time )
		: QFrame( NULL, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
{
	setAttribute( Qt::WA_DeleteOnClose );
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );
	setWindowOpacity( 0.70 );
	setFrameStyle( NoFrame );
	QPixmap fond(":/notifier_fond.png");
	setMask(fond.createHeuristicMask());
	resize(fond.size());

	//Construction des items à insérer dans la fenêtre :
	// - l'icône
	QLabel *icone = new QLabel();
	icone->setPixmap( computer->icon() );

	// - le pseudo
	QLabel *name = new QLabel();
	name->setTextFormat( Qt::RichText );
	name->setText( "<h2><font color=\"white\">" + computer->name() + "</font></h2>" );

	// - la description de l'état de connexion
	QLabel *description = new QLabel();
	QPalette palette ;
	switch(computer->state())
	{
		case Rzx::STATE_DISCONNECTED:
			description->setText( tr( "is now disconnected" ) );
			palette.setColor( foregroundRole(), QColor( 0xff, 0x20, 0x20 ) );
			break;
		case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
			description->setText(tr( "is now away" ));
			palette.setColor( foregroundRole(), RzxConfig::repondeurBase() );
			break;
		case Rzx::STATE_HERE:
			description->setText(tr( "is now here" ));
			palette.setColor( foregroundRole(), QColor( 0xff, 0xff, 0xff ) );
			break;
	}
	palette.setColor( backgroundRole(), QColor( 0xc0, 0xc0, 0xc0 ) );
	setPalette( palette );
	name->setPalette(palette);
	description->setPalette(palette);

	//Insertion des éléments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget( name, 0, Qt::AlignTop | Qt::AlignLeft );
	textLayout->addWidget( description, 0, Qt::AlignVCenter | Qt::AlignLeft );

	// - insertion de l'icône à côté du texte
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget( icone, 0, Qt::AlignVCenter | Qt::AlignLeft );
	layout->addLayout( textLayout, 1 );
	setLayout( layout );

	//Affichage de la fenêtre
	int x = QApplication::desktop()->width();
	int y = QApplication::desktop()->height();
	x -= width();
	x >>= 1;
	y -= height();
	y <<= 1;
	y /= 3;
	QPoint point( x, y );

	move( point );
	setFocusPolicy( Qt::NoFocus );
	show();

	//Timer pour déclencher la destruction de le fenêtre
	connect( &timer, SIGNAL( timeout() ), this, SLOT( close() ) );
	timer.start( time * 1000 );
}

///Destruction de la fenêtre
/** Rien à faire ici */
RzxTrayWindow::~RzxTrayWindow()
{}

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

///Construction de la fen�tre de notification d'�tat de connexion de computer
/** La fen�tre est construite pour dispara�tre automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow( RzxComputer* computer, unsigned int time )
		: QFrame( NULL, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
{
	setAttribute( Qt::WA_DeleteOnClose );
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );
	setWindowOpacity( 0.70 );
#ifndef Q_OS_MAC

	setFrameStyle( Panel | Plain );
#else

	setFrameStyle( MenuBarPanel | Plain );
#endif

	QPalette palette;
	switch(computer->state())
	{
		case Rzx::STATE_DISCONNECTED:
			palette.setColor( backgroundRole(), QColor( 0xff, 0x20, 0x20 ) );
			break;
		case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
			palette.setColor( backgroundRole(), RzxConfig::repondeurBase() );
			break;
		case Rzx::STATE_HERE:
			palette.setColor( backgroundRole(), QColor( 0xff, 0xff, 0xff ) );
			break;
	}
	setPalette( palette );

	//Construction des items � ins�rer dans la fen�tre :
	// - l'ic�ne
	QLabel *icone = new QLabel();
	icone->setPixmap( computer->icon() );

	// - le pseudo
	QLabel *name = new QLabel();
	name->setTextFormat( Qt::RichText );
	name->setText( "<h2>" + computer->name() + "</h2>" );

	// - la description de l'�tat de connexion
	QLabel *description = new QLabel();
	switch(computer->state())
	{
		case Rzx::STATE_HERE:
			description->setText(tr( "is now here" ));
			break;
		case Rzx::STATE_REFUSE: case Rzx::STATE_AWAY:
			description->setText(tr( "is now away" ));
			break;
		case Rzx::STATE_DISCONNECTED:
			description->setText( tr( "is now disconnected" ) );
			break;
	}

	//Insertion des �l�ments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget( name, 0, Qt::AlignTop | Qt::AlignLeft );
	textLayout->addWidget( description, 0, Qt::AlignVCenter | Qt::AlignLeft );

	// - insertion de l'ic�ne � c�t� du texte
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget( icone, 0, Qt::AlignVCenter | Qt::AlignLeft );
	layout->addLayout( textLayout, 1 );
	setLayout( layout );

	//Affichage de la fen�tre
#ifdef Q_OS_MAC
	QPoint point( 0, 21 );
#else
	QPoint point( 0, 0 );
#endif

	move( point );
	setFocusPolicy( Qt::NoFocus );
	show();

	//Timer pour d�clencher la destruction de le fen�tre
	connect( &timer, SIGNAL( timeout() ), this, SLOT( close() ) );
	timer.start( time * 1000 );
}

///Destruction de la fen�tre
/** Rien � faire ici */
RzxTrayWindow::~RzxTrayWindow()
{}

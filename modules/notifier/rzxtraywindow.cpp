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
#include <QBitmap>
#include <QPainter>
#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include <RzxComputer>
#include <RzxConfig>

#include "rzxtraywindow.h"


///Construction de la fen�tre de notification d'�tat de connexion de computer
/** La fen�tre est construite pour dispara�tre automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow( RzxComputer* c, unsigned int time )
		: QFrame( NULL, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ), computer(c)
{
	setAttribute( Qt::WA_DeleteOnClose );
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );
	setWindowOpacity( 0.70 );
	setFrameStyle( NoFrame );
	QPixmap fond(":/notifier_fond.png");
	setMask(fond.createHeuristicMask());
	resize(fond.size());

	//Construction des items � ins�rer dans la fen�tre :
	// - l'ic�ne
	QLabel *icone = new QLabel();

	// - le pseudo
	QLabel *name = new QLabel();
	name->setTextFormat( Qt::RichText );
	name->setText( "<h2><font color=\"white\">" + computer->name() + "</font></h2>" );

	// - la description de l'�tat de connexion
	QPixmap symbolPixmap;
	int px, py;
	px = py = 0; // pour faire taire un warning @lc...
	switch(computer->state())
	{
		case Rzx::STATE_DISCONNECTED:
			symbolPixmap = QPixmap(":/notifier_quit.png");
			px = 29; py = 27;
			break;
		case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
			symbolPixmap = QPixmap(":/notifier_away.png");
			px = 19; py = 27;
			break;
		case Rzx::STATE_HERE:
			symbolPixmap = QPixmap(":/notifier_here.png");
			px = 43; py = 10;
			break;
	}

	QPixmap pixmap(symbolPixmap.width(), symbolPixmap.height());
	pixmap.fill(QColor(0,0,0,0));
	QPainter painter(&pixmap);
	painter.drawPixmap(px, py, computer->icon());
	painter.drawPixmap(0, 0, symbolPixmap);
	icone->setPixmap(pixmap);

	QPalette palette ;
	palette.setColor( backgroundRole(), QColor( 0xc0, 0xc0, 0xc0 ) );
	setPalette( palette );

	//Insertion des �l�ments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget( name, 0, Qt::AlignTop | Qt::AlignHCenter );
	textLayout->addWidget( icone, 0, Qt::AlignVCenter | Qt::AlignHCenter );
	setLayout(textLayout);
	
	//Affichage de la fen�tre
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

	//Timer pour d�clencher la destruction de le fen�tre
	connect( &timer, SIGNAL( timeout() ), this, SLOT( close() ) );
	timer.setSingleShot(true);
	timer.start( time * 1000 );
}

///Destruction de la fen�tre
/** Rien � faire ici */
RzxTrayWindow::~RzxTrayWindow()
{}

///Capture de l'action de la souris
/** On applique quelques r�gles simples :
 * - clic droit ==> chat
 * - clic gauche ==> fermeture de la fen�tre
 */
void RzxTrayWindow::mousePressEvent(QMouseEvent *e)
{
	switch(e->button())
	{
		case Qt::RightButton:
			if(computer && computer->state() == Rzx::STATE_HERE)
				computer->chat();

		case Qt::LeftButton:
			timer.stop();
			close();
			break;

		default:
			QWidget::mousePressEvent(e);
	}
}

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


///Construction de la fenêtre de notification d'état de connexion de computer
/** La fenêtre est construite pour disparaître automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(Theme theme, RzxComputer* c, unsigned int time )
		: QFrame( NULL, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ), computer(c)
{
	setAttribute( Qt::WA_DeleteOnClose );
	setAttribute( Qt::WA_QuitOnClose,false);

	//Composition de la fenêtre dans le thème choisi
	switch(theme)
	{
		case Nice: niceTheme(); break;
		case Old: oldTheme(); break;
		default: close(); return;
	}

	//Affichage
	setFocusPolicy( Qt::NoFocus );
	show();

	//Timer pour déclencher la destruction de le fenêtre
	connect( &timer, SIGNAL( timeout() ), this, SLOT( close() ) );
	timer.setSingleShot(true);
	timer.start( time * 1000 );
}

///Destruction de la fenêtre
/** Rien à faire ici */
RzxTrayWindow::~RzxTrayWindow()
{}


///Compose une fenêtre de notification dans le style 'nice'
void RzxTrayWindow::niceTheme()
{
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

	// - le pseudo
	QLabel *name = new QLabel();
	name->setTextFormat( Qt::RichText );
	name->setText( "<h2><font color=\"white\">" + computer->name() + "</font></h2>" );

	// - la description de l'état de connexion
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

	//Insertion des éléments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget( name, 0, Qt::AlignTop | Qt::AlignHCenter );
	textLayout->addWidget( icone, 0, Qt::AlignVCenter | Qt::AlignHCenter );
	setLayout(textLayout);
	
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
}

///Compose une fenêtre de notification de style 'old'
/** Correspond au thème des fenêtre de notification de qRezix > 1.6.1
 */
void RzxTrayWindow::oldTheme()
{
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );

	bool connected = true;;
	QPalette palette ;
	switch(computer->state())
	{
		case Rzx::STATE_DISCONNECTED:
			palette.setColor( backgroundRole(), QColor(0xff, 0x20, 0x20));
			connected = false;
			break;
		case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
			palette.setColor( backgroundRole(), 0xFFEE7C);
			break;
		case Rzx::STATE_HERE:
			palette.setColor( backgroundRole(), 0xffffff);
			break;
	}
	setPalette( palette );

	//insertion de l'icône
	// Layout, pour le resize libre
	QHBoxLayout *layout = new QHBoxLayout();
	QVBoxLayout *textLayout = new QVBoxLayout();

	QLabel *icone = new QLabel();
	QLabel *name = new QLabel();
	QLabel *description = new QLabel();

	layout->addWidget(icone, 0, Qt::AlignVCenter | Qt::AlignLeft);
	textLayout->addWidget(name, 0, Qt::AlignTop | Qt::AlignLeft);
	textLayout->addWidget(description, 0, Qt::AlignVCenter | Qt::AlignLeft);
	layout->addLayout(textLayout, 1);

	icone->setPixmap(computer->icon());
	name->setTextFormat(Qt::RichText);
	name->setText("<h2>" + computer->name() + "</h2>");
	if(connected)
		description->setText(computer->isOnResponder() ? tr("is now away") : tr("is now here"));
	else
		description->setText(tr("is now disconnected"));
	setLayout(layout);

	//Pour avoir un cadre plus soft...
	adjustSize();
	QPixmap fond(":/notifier_fond.png");
	setMask(fond.scaled(size()).createHeuristicMask());

	//Dans le coin en haut à gauche
	QPoint point(0,0);
	move(point);
}

///Capture de l'action de la souris
/** On applique quelques règles simples :
 * - clic droit ==> chat
 * - clic gauche ==> fermeture de la fenêtre
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

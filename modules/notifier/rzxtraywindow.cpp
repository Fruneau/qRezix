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

#ifdef Q_OS_MAC
#include <Growl/GrowlApplicationBridge-Carbon.h>
#include <Growl/GrowlDefines.h>
#endif

#include <RzxComputer>
#include <RzxConfig>

#include "rzxtraywindow.h"


///Construction de la fen�tre de notification d'�tat de connexion de computer
/** La fen�tre est construite pour dispara�tre automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(Theme theme, RzxComputer* c, unsigned int m_time )
		: QFrame( NULL, Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ), time(m_time), computer(c)
{
	setAttribute( Qt::WA_DeleteOnClose );
	setAttribute( Qt::WA_QuitOnClose,false);

	//Composition de la fen�tre dans le th�me choisi
	switch(theme)
	{
		case Nice: niceTheme(); break;
		case Old: oldTheme(); break;
#ifdef Q_OS_MAC
		case Growl: growlNotif(); close(); return;
#endif
		default: close(); return;
	}

	//Affichage
	tryToShow();
}

///Destruction de la fen�tre
/** Rien � faire ici */
RzxTrayWindow::~RzxTrayWindow()
{}


///Tente d'afficher la fen�tre de notification si elle ne risque pas de g�ner
void RzxTrayWindow::tryToShow()
{
	if(QApplication::activePopupWidget())
	{
		QTimer::singleShot(500, this, SLOT(tryToShow()));
		return;
	}
	active = QApplication::activeWindow();
	show();

	//Timer pour d�clencher la destruction de le fen�tre
	QTimer::singleShot(time * 1000, this, SLOT(close()));

	//Timer pour rendre le focus � la fen�tre qui l'avait avant
	QTimer::singleShot(10, this, SLOT(giveFocus()));
}

///Rend le focus � le fen�tre qui l'avait auparavant
void RzxTrayWindow::giveFocus()
{
	if(active)
		active->activateWindow();
}

///Compose une fen�tre de notification dans le style 'nice'
void RzxTrayWindow::niceTheme()
{
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
	int px, py, scale = 64;
	px = py = 0; // pour faire taire un warning @lc...
	switch(computer->state())
	{
		case Rzx::STATE_DISCONNECTED:
			symbolPixmap = QPixmap(":/notifier_quit.png");
			px = 28; py = 46;
			scale = 48;
			break;
		case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
			symbolPixmap = QPixmap(":/notifier_away.png");
			px = 41; py = 27;
			scale = 46;
			break;
		case Rzx::STATE_HERE:
			symbolPixmap = QPixmap(":/notifier_here.png");
			px = 46; py = 20;
			scale = 56;
			break;
	}

	QPixmap pixmap(symbolPixmap.width(), symbolPixmap.height());
	pixmap.fill(QColor(0,0,0,0));
	QPainter painter(&pixmap);
	painter.drawPixmap(px, py, computer->icon().scaled(scale, scale));
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
}

///Compose une fen�tre de notification de style 'old'
/** Correspond au th�me des fen�tre de notification de qRezix > 1.6.1
 */
void RzxTrayWindow::oldTheme()
{
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );

	bool connected = true;
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

	//insertion de l'ic�ne
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

	//Dans le coin en haut � gauche
	QPoint point(0,0);
	move(point);
}

#ifdef Q_OS_MAC

#define CFStringFromQt(qstr) \
	CFStringCreateWithCString(kCFAllocatorDefault, qstr.toLatin1().constData(), kCFStringEncodingISOLatin1)

///Affiche une notification avec Growl
void RzxTrayWindow::growlNotif()
{
	QString title = "qRezix...";
	QString text = computer->name() + " ";

    if(computer->state() != Rzx::STATE_DISCONNECTED)
		text += computer->isOnResponder() ? tr("is now away") : tr("is now here");
	else
		text += tr("is now disconnected");
		
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(
			CFStringFromQt(title),
			CFStringFromQt(text),
			CFSTR("Connection State Change"),
			NULL,       
			0,
			0,
			NULL);
}
#endif

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
			close();
			break;

		default:
			QFrame::mousePressEvent(e);
	}
}

///Pour que la fen�tre de notification ne garde jamais le focus
bool RzxTrayWindow::event(QEvent *e)
{
	if(e->type() == QEvent::ActivationChange && isActiveWindow())
	{
		QTimer::singleShot(10, this, SLOT(giveFocus()));
		return true;
	}
	else
		return QFrame::event(e);
}

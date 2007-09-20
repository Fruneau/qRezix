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
#include <QByteArray>
#include <QBuffer>

#ifdef Q_OS_MAC
#include <Growl-WithInstaller/GrowlApplicationBridge-Carbon.h>
#include <Growl-WithInstaller/GrowlDefines.h>
#endif

#include <RzxComputer>
#include <RzxConfig>

#include "rzxtraywindow.h"

int	NbTrayWindow = 0;

///Construction de la fenêtre de notification d'état de connexion de computer
/** La fenêtre est construite pour disparaître automatiquement au bout de time secondes */
RzxTrayWindow::RzxTrayWindow(QObject *parent, Theme theme, RzxComputer* c, unsigned int m_time )
	: QFrame( NULL, Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ),
	time(m_time), computer(c), type(ConnectionState)
{
        connect(this, SIGNAL(wantTrayNotification(const QString&, const QString&)),
                parent, SIGNAL(wantTrayNotification(const QString&, const QString&)));
	init(theme);
}

///Construction d'une fenêtre de notification de chat
RzxTrayWindow::RzxTrayWindow(QObject *parent, Theme theme, RzxComputer *c, const QString& text, unsigned int m_time)
	: QFrame( NULL, Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ),
	time(m_time), computer(c), type(Chat), chatText(text)
{
        connect(this, SIGNAL(wantTrayNotification(const QString&, const QString&)), 
                parent, SIGNAL(wantTrayNotification(const QString&, const QString&))); 
	init(theme);
}

///Initialise l'objet et en lance l'affichage si nécessaire
/** Configure l'objet pour afficher la fenêtre de notification avec le thème demandé
 */
void RzxTrayWindow::init(Theme theme)
{
	setAttribute( Qt::WA_DeleteOnClose );
	setAttribute( Qt::WA_QuitOnClose,false);

	//Composition de la fenêtre dans le thème choisi
	switch(theme)
	{
		case Nice: niceTheme(); break;
		case Old: oldTheme(); break;
#ifdef Q_OS_MAC
		case Growl: growlNotif(); close(); return;
#endif
                case SysTray: trayNotif(); close(); return;
		default: close(); return;
	}

	//Affichage
	tryToShow();
}

///Destruction de la fenêtre
/** Rien à faire ici */
RzxTrayWindow::~RzxTrayWindow()
{ NbTrayWindow--; }


///Tente d'afficher la fenêtre de notification si elle ne risque pas de gêner
void RzxTrayWindow::tryToShow()
{
	if(QApplication::activePopupWidget())
	{
		QTimer::singleShot(500, this, SLOT(tryToShow()));
		return;
	}
	active = QApplication::activeWindow();
	show();

	NbTrayWindow++;
	if( NbTrayWindow > 5 )					// Pour éviter que trop de fenêtres ne soient affichées, ce qui
		QTimer::singleShot( 0, this, SLOT(close()));	// risquerait de planter l'interface graphique
	else
		//Timer pour déclencher la destruction de le fenêtre
		QTimer::singleShot(time * 1000, this, SLOT(close()));

	//Timer pour rendre le focus à la fenêtre qui l'avait avant
	QTimer::singleShot(10, this, SLOT(giveFocus()));
}

///Rend le focus à le fenêtre qui l'avait auparavant
void RzxTrayWindow::giveFocus()
{
	if(active)
		active->activateWindow();
}

///Compose une fenêtre de notification dans le style 'nice'
void RzxTrayWindow::niceTheme()
{
	if(type == ConnectionState)
	{
		// - la description de l'état de connexion
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
		painter.drawPixmap(px, py, computer->icon(true).scaled(scale, scale));
		painter.drawPixmap(0, 0, symbolPixmap);
		
		niceThemeFactory(computer->name(), QString(), pixmap);
	}
	else if(type == Chat)
		niceThemeFactory(computer->name() + " " + tr("says:"), chatText, computer->icon(true));		
}

///Crée la fenêtre de notification en elle-même pour le thème 'nice'
void RzxTrayWindow::niceThemeFactory(const QString& title, const QString &text, const QPixmap& icon)
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
	icone->setPixmap(icon);
	
	// - le titre
	QLabel *name = new QLabel();
	name->setTextFormat( Qt::RichText );
	name->setText( "<h2><font color=\"white\">" + title + "</font></h2>" );
	
	// - le texte... si n'est pas vide
	QLabel *info = NULL;
	if(!text.isEmpty())
	{
		info = new QLabel();
		info->setTextFormat(Qt::RichText);
		info->setText( "<font color=\"white\">" + text.left(100) + "</font>" );
	}
	
	//Insertion des éléments dans les layout qui corresponend
	// - disposition du texte verticalement
	QVBoxLayout *textLayout = new QVBoxLayout();
	textLayout->addWidget( icone, 0, Qt::AlignTop | Qt::AlignHCenter );
	textLayout->addWidget( name, 0, Qt::AlignHCenter );
	if(info)
		textLayout->addWidget( info, 0, Qt::AlignTop | Qt::AlignHCenter );
	setLayout(textLayout);
	
	QPalette palette ;
	palette.setColor( backgroundRole(), QColor( 0xc0, 0xc0, 0xc0 ) );
	setPalette( palette );
	
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
	QString title = computer->name();
	QString text;
	QColor bg = 0xffffff;

	if(type == ConnectionState)
	{
		bool connected = true;
		switch(computer->state())
		{
			case Rzx::STATE_DISCONNECTED:
				bg = QColor(0xff, 0x20, 0x20);
				connected = false;
				break;
			case Rzx::STATE_AWAY: case Rzx::STATE_REFUSE:
				bg =  0xFFEE7C;
				break;
			default:
				break;
		}

		if(connected)
			text = computer->isOnResponder() ? tr("is now away") : tr("is now here");
		else
			text = tr("is now disconnected");
	}
	else if (type == Chat)
	{
		text = chatText;
		title += " " + tr("says:");
	}
		
	oldThemeFactory(title, text, computer->icon(true), bg);
}

void RzxTrayWindow::oldThemeFactory(const QString& title, const QString& text, const QPixmap& icon, const QColor& bg)
{
	setMinimumWidth( 150 );
	setMinimumHeight( 70 );
	
	QPalette palette ;	
	palette.setColor(backgroundRole(), bg);
	setPalette(palette);

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
	
	icone->setPixmap(icon);
	name->setTextFormat(Qt::RichText);
	name->setText("<h2>" + title + "</h2>");
	description->setText(text);
	setLayout(layout);
	
	//Pour avoir un cadre plus soft...
	adjustSize();
	QPixmap fond(":/notifier_fond.png");
	setMask(fond.scaled(size()).createHeuristicMask());
	
	//Dans le coin en haut à gauche
	QPoint point(0,0);
	move(point);
}

#ifdef Q_OS_MAC

#define CFStringFromQt(qstr) \
	CFStringCreateWithCString(kCFAllocatorDefault, qstr.toLatin1().constData(), kCFStringEncodingISOLatin1)

///Affiche une notification avec Growl
void RzxTrayWindow::growlNotif()
{
	QString title;
	QString text;
	QString notifType;

	if(type == ConnectionState)
	{
		title = tr("qRezix favorite change...");
		text = computer->name() + " ";
		if(computer->state() != Rzx::STATE_DISCONNECTED)
			text += computer->isOnResponder() ? tr("is now away") : tr("is now here");
		else
			text += tr("is now disconnected");
		notifType = "Connection State Change";
	}
	else if (type == Chat)
	{
		title = computer->name() + " " + tr("says on qRezix:");
		text = chatText;
		notifType = "Chat";
	}

	growlNotifFactory(title, text, computer->icon(true), notifType);
}

void RzxTrayWindow::growlNotifFactory(const QString& title, const QString& text, const QPixmap& icon, const QString &type)
{
	QByteArray bytes;
	QBuffer buffer(&bytes);
	buffer.open(QIODevice::WriteOnly);
	icon.save(&buffer, "PNG");
	CFDataRef imageData = CFDataCreate(kCFAllocatorDefault,
									   (const UInt8*)bytes.constData(), 
									   bytes.size());
	
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(
			CFStringFromQt(title),
			CFStringFromQt(text),
			CFStringFromQt(type),
			imageData,       
			0,
			0,
			NULL);
}	
#endif

void RzxTrayWindow::trayNotif()
{
        QString title; 
        QString text; 
 
        if(type == ConnectionState) 
        { 
                title = tr("qRezix favorite change..."); 
                text = computer->name() + " "; 
                if(computer->state() != Rzx::STATE_DISCONNECTED) 
                        text += computer->isOnResponder() ? tr("is now away") : tr("is now here"); 
                else 
                        text += tr("is now disconnected"); 
        } 
        else if (type == Chat) 
        { 
                title = computer->name() + " " + tr("says on qRezix:"); 
                text = chatText; 
        } 
 
        emit wantTrayNotification(title, text);
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
			close();
			break;

		default:
			QFrame::mousePressEvent(e);
	}
}

///Pour que la fenêtre de notification ne garde jamais le focus
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

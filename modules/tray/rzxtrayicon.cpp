/*
* trayicon.cpp - system-independent trayicon class (adapted from Qt example)
* Copyright (C) 2003  Justin Karneges
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Adaptation to qRezix by Florent Bruneau, Copyright (C) 2004-2005 Binet R�seau
*/
#define RZX_MODULE_NAME "Tray icon"
#define RZX_MODULE_DESCRIPTION "Systray and Dock integration"
#define RZX_MODULE_ICON Rzx::ICON_SYSTRAYAWAY

#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QToolTip>
#include <QPainter>
#include <QPaintDevice>
#include <QMenu>
#include <QWidget>
#include <QCursor>
#include <QLibrary>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QEvent>
#include <QMouseEvent>

#include <RzxConfig>
#include <RzxComputer>
#include <RzxIconCollection>
#include <RzxConnectionLister>
#include <RzxApplication>
#include <RzxTranslator>

#include "rzxtrayicon.h"
#include "rzxtrayconfig.h"

#ifdef Q_WS_MAC
void qt_mac_set_dock_menu( QMenu *menu );
#endif

///Exporte le module
RZX_MODULE_EXPORT(RzxTrayIcon)

///Initialisation de la config
RZX_CONFIG_INIT(RzxTrayConfig)

///Fonction auxiliaire pour trier une liste d'ordinateurs par ordre alphab�tique des noms
bool computerLessThan(RzxComputer *c1, RzxComputer *c2)
{
	if(c1 == NULL) return false;
	if(c2 == NULL) return true;
	return c1->name().toLower() < c2->name().toLower();
}

/*!
  Creates a RzxTrayIcon object displaying \a icon and \a tooltip, and opening
  \a popup when clicked with the right mousebutton. \a parent and \a name are
  propagated to the QObject constructor. The icon is initially invisible.
 
  \sa show
*/
RzxTrayIcon::RzxTrayIcon()
		: RzxModule(RZX_MODULE_NAME, QT_TRANSLATE_NOOP("RzxBaseModule", "Systray and Dock integration"), RZX_MODULE_VERSION), tip("qRezix"), d( 0 )
{
	beginLoading();
	setType(MOD_GUI);
#ifndef Q_WS_MAC
	setType(MOD_HIDE);
#endif
	setIcon(RZX_MODULE_ICON);
	new RzxTrayConfig(this);
#ifndef Q_OS_MAC
	ui = NULL;
	propWidget = NULL;
	RzxTranslator::connect(this, SLOT(translate()));
#endif
	v_isWMDock = FALSE;
	buildMenu();
	connect(RzxComputer::localhost(), SIGNAL(stateChanged(RzxComputer*)), this, SLOT(changeTrayIcon()));
	connect(RzxConnectionLister::global(), SIGNAL(countChange(const QString& )), this, SLOT(setToolTip(const QString& )));
	connect(this, SIGNAL(clicked(const QPoint&)), this, SIGNAL(wantToggleVisible()));
	RzxIconCollection::connect(this, SLOT(changeTrayIcon()));
	changeTrayIcon();
	endLoading();
}

/*!
  Removes the icon from the system tray and frees all allocated resources.
*/
RzxTrayIcon::~RzxTrayIcon()
{
	beginClosing();
	sysRemove();
	delete RzxTrayConfig::global();
	endClosing();
}

/** \reimp */
bool RzxTrayIcon::isInitialised() const
{
	return true;
}

/*!
  \property RzxTrayIcon::trayIcon
  \brief the system tray icon.
*/
void RzxTrayIcon::setTrayIcon( const QPixmap &icon )
{
	pm = icon;
	sysUpdateIcon();
}

///Mise � jour de l'ic�ne de qRezix
void RzxTrayIcon::changeTrayIcon()
{
	// Change l'icone dans la tray
	QPixmap trayIcon;
	if(!RzxConfig::autoResponder())
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYHERE);
	else
		trayIcon = RzxIconCollection::getPixmap(Rzx::ICON_SYSTRAYAWAY);
#ifdef Q_WS_MAC
	buildMenu();
#else
	changePropTheme();
#endif
	setTrayIcon(trayIcon);
}


QPixmap RzxTrayIcon::trayIcon() const
{
	return pm;
}

QString RzxTrayIcon::toolTip() const
{
	return tip;
}

void RzxTrayIcon::setVisible( bool yes )
{
	if ( yes )
		show();
	else
		hide();
}

///Contruit le menu contextuel
void RzxTrayIcon::buildMenu()
{
	if ( pop.actions().count() )
		pop.clear();

#ifndef Q_OS_MAC
	QList<RzxComputer*> list;
	QList<RzxComputer*> favList;
	RzxTrayConfig::QuickActions actions = (RzxTrayConfig::QuickAction)RzxTrayConfig::quickActions();

#define newMenu(mname, micon, text, filter, slot, favoris) \
	{ \
		mname.clear(); \
		mname.setTitle(text); \
		mname.setIcon(RzxIconCollection::getIcon(micon)); \
		list = RzxConnectionLister::global()->computerList(filter); \
		qSort(list.begin(), list.end(), computerLessThan); \
		favList.clear(); \
		if (favoris) \
			foreach(RzxComputer *computer, list) \
				if(testComputerFavorite(computer)) \
					favList << computer; \
		subMenu(mname, micon, slot, list, favList); \
	}

	if(actions & RzxTrayConfig::Chat)
		newMenu(chat, Rzx::ICON_CHAT, tr("Chat..."),testComputerChat, SLOT(chat()), (actions & RzxTrayConfig::ChatFav) );
	if(actions & RzxTrayConfig::Ftp)
		newMenu(ftp, Rzx::ICON_FTP, tr("Open FTP..."), testComputerFtp, SLOT(ftp()), (actions & RzxTrayConfig::ChatFav) );
	if(actions & RzxTrayConfig::Http)
		newMenu(http, Rzx::ICON_HTTP, tr("Open Web Page..."), testComputerHttp, SLOT(http()), 0);
	if(actions & RzxTrayConfig::Samba)
		newMenu(samba, Rzx::ICON_SAMBA, tr("Open Samba..."), testComputerSamba, SLOT(samba()), 0);
	if(actions & RzxTrayConfig::News)
		newMenu(news, Rzx::ICON_NEWS, tr("Read News..."), testComputerNews, SLOT(news()), 0);
	if(actions & RzxTrayConfig::Mail)
		newMenu(mail, Rzx::ICON_MAIL, tr("Send a Mail..."), testComputerMail, SLOT(mail()), 0);

#undef newMenu

	if(pop.actions().count())
		pop.addSeparator();
#endif //!Q_OS_MAC

#define newItem(name, trad, receiver, slot) pop.addAction(RzxIconCollection::getIcon(name), trad, receiver, slot)
	newItem(Rzx::ICON_PREFERENCES , tr( "&Preference" ), this, SIGNAL( wantPreferences() ) );
	if (RzxConfig::autoResponder())
		newItem(Rzx::ICON_HERE, tr( "&I'm back !" ), this, SIGNAL( wantToggleResponder() ) );
	else
		newItem(Rzx::ICON_AWAY, tr( "I'm &away !" ), this, SIGNAL( wantToggleResponder() ) );

#ifndef Q_OS_MAC
	newItem(Rzx::ICON_QUIT, tr( "&Quit" ), this, SIGNAL(wantQuit()) );
#else
	qt_mac_set_dock_menu( &pop );
#endif

#undef newItem
}

///Fonction auxiliaire pour faire les titres dynamiques des sous-menus
QString RzxTrayIcon::titleFromSplit(int j, QString splitPoints)
{
	QString titre;
	// Faut penser � une fa�on plus sympa de renvoyer cette erreur (qui ne doit jamais arriver...)
	if ( (j+1) >= splitPoints.size() )
		titre = "Sub";
	else {
		// Cas d'une seule lettre pour le sous-menu
		if ( (splitPoints.at(j).toAscii()+1) == splitPoints.at(j+1).toAscii() )
			titre = QString(QChar(splitPoints.at(j).toAscii() + 1).toUpper());

		else {
			titre = "  -  ";
			if (j == 0)
				titre[0] = splitPoints.at(j).toUpper();
			else
				titre[0] = QChar(splitPoints.at(j).toAscii() + 1).toUpper();
			titre[4] = splitPoints.at(j+1).toUpper();
		}
	}

	return titre;
}

///Fonction auxiliaire pour calculer les points de separation des sous-menus
int RzxTrayIcon::calculerSplit(QString& splitPoints, QList<RzxComputer*> list)
{
	// Changer �a (dans les prefs / calculer avec QtApplication::desktop()->size() )
	int tailleMenu = 50;

	// Le menu est petit, rien � faire
	if (list.count() <= tailleMenu) return 0;

	splitPoints[0] = list[0]->name().at(0).toLower();
	int j = 1;
	QChar c;
	int i = 1;
	int i2 = 1;
	for(; i < list.count(); i++, i2++)
	{
		c = list[i-1]->name().at(0).toLower();
		if ( c != list[i]->name().at(0).toLower() )
		{
			splitPoints[j] = c;
			if (i2 > tailleMenu)
			{
				j++;
				i2 = 0;
			}
		}
	}
	splitPoints[j] = list[i-1]->name().at(0).toLower();

	return 1;
}

///Fonction qui cree les menus du TrayIcon
void RzxTrayIcon::subMenu(QMenu& mname, Rzx::Icon micon, const char * slot, QList<RzxComputer*> list, QList<RzxComputer*> fav)
{
	if(list.count())
	{
		QString splitPoints;
		QMenu * mnsub;
		bool makeSubMenu = calculerSplit(splitPoints, list);
		if ( makeSubMenu || fav.count() )
		{
			if (fav.count())
			{
				mnsub = new QMenu(tr("Favorites"), &mname);
				mnsub->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
				mname.addMenu(mnsub);

				qSort(fav.begin(), fav.end(), computerLessThan);
				for(int i = 0; i < fav.count(); i++)
					mnsub->addAction(fav[i]->icon(), fav[i]->name(), fav[i], slot);
			}

			QString titre;
			if ( makeSubMenu )
				titre = titleFromSplit(0,splitPoints);
			else
				titre = tr("Everybody");
			mnsub = new QMenu(titre, &mname);
			mnsub->setIcon(RzxIconCollection::getIcon(micon));
			mname.addMenu(mnsub);
			int j = 1;
			for(int i = 0 ; i < list.count() ; i++)
			{
				if( list[i]->name().at(0).toLower() > splitPoints.at(j) )
				{
					titre = titleFromSplit(j,splitPoints);
					mnsub = new QMenu(titre, &mname);
					mnsub->setIcon(RzxIconCollection::getIcon(micon));
					mname.addMenu(mnsub);
					j++;
				}
				mnsub->addAction(list[i]->icon(), list[i]->name(), list[i], slot);
			}
		 }
		else {
			for(int i = 0 ; i < list.count() ; i++)
				mname.addAction(list[i]->icon(), list[i]->name(), list[i], slot);
		}
		pop.addMenu(&mname);
	}
}

/*!
  Shows the icon in the system tray.
 
  \sa hide
*/
void RzxTrayIcon::show()
{
	sysInstall();
}

/*!
  Hides the system tray entry.
*/
void RzxTrayIcon::hide()
{
	sysRemove();
}

/*!
  \reimp
*/
bool RzxTrayIcon::event( QEvent *e )
{
	switch ( e->type() )
	{
		case QEvent::MouseMove:
			mouseMoveEvent( ( QMouseEvent* ) e );
			break;

		case QEvent::MouseButtonPress:
			mousePressEvent( ( QMouseEvent* ) e );
			break;

		case QEvent::MouseButtonRelease:
			mouseReleaseEvent( ( QMouseEvent* ) e );
			break;

		case QEvent::MouseButtonDblClick:
			mouseDoubleClickEvent( ( QMouseEvent* ) e );
			break;
		default:
			return QObject::event( e );
	}

	return TRUE;
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse move events for the system tray entry.
 
  \sa mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),  QMouseEvent
*/
void RzxTrayIcon::mouseMoveEvent( QMouseEvent *e )
{
	e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse press events for the system tray entry.
 
  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void RzxTrayIcon::mousePressEvent( QMouseEvent *e )
{
#ifndef Q_WS_WIN 
	// This is for X11, menus appear on mouse press
	// I'm not sure whether Mac should be here or below.. Somebody check?
	switch ( e->button() )
	{
		case Qt::RightButton:
			if ( pop.actions().count() )
			{
				buildMenu();
				pop.popup( e->globalPos() );
				e->accept();
			}
			break;
		case Qt::LeftButton:
		case Qt::MidButton:
			emit clicked( e->globalPos() );
			break;
		default:
			break;
	}
#endif
	e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse release events for the system tray entry.
 
  The default implementations opens the context menu when the entry
  has been clicked with the right mouse button.
 
  \sa setPopup(), mousePressEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void RzxTrayIcon::mouseReleaseEvent( QMouseEvent *e )
{
#ifdef Q_WS_WIN 
	// This is for Windows, where menus appear on mouse release
	switch ( e->button() )
	{
		case Qt::RightButton:
			if ( true /*pop.count() #GUI_TEST*/ )
			{
				buildMenu();
				// Necessary to make keyboard focus
				// and menu closing work on Windows.
				//pop.setActiveWindow();	#GUI_TEST
				pop.popup( e->globalPos() );
				//pop.setActiveWindow();	#GUI_TEST
				e->accept();
			}
			break;
		case Qt::LeftButton:
		case Qt::MidButton:
			emit clicked( e->globalPos() );
			break;
		default:
			break;
	}
#endif
	e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse double click events for the system tray entry.
 
  Note that the system tray entry gets a mousePressEvent() and a
  mouseReleaseEvent() before the mouseDoubleClickEvent().
 
  \sa mousePressEvent(), mouseReleaseEvent(),
  mouseMoveEvent(), QMouseEvent
*/
void RzxTrayIcon::mouseDoubleClickEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
		emit doubleClicked( e->globalPos() );
	e->accept();
}

/*!
  \fn void RzxTrayIcon::clicked( const QPoint &p )
 
  This signal is emitted when the user clicks the system tray icon
  with the left mouse button, with \a p being the global mouse position
  at that moment.
*/

/*!
  \fn void RzxTrayIcon::doubleClicked( const QPoint &p )
 
  This signal is emitted when the user double clicks the system tray
  icon with the left mouse button, with \a p being the global mouse position
  at that moment.
*/

void RzxTrayIcon::gotCloseEvent()
{
	closed();
}

#ifdef WIN32 
/*
 * trayicon_win.cpp - Windows trayicon, adapted from Qt example
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <qt_windows.h>

static uint WM_TASKBARCREATED = 0;
#define WM_NOTIFYICON	(WM_APP+101)

typedef BOOL ( WINAPI *PtrShell_NotifyIcon ) ( DWORD, PNOTIFYICONDATA );
static PtrShell_NotifyIcon ptrShell_NotifyIcon = 0;

static void resolveLibs()
{
	QLibrary lib( "shell32" );
	//lib.setAutoUnload( FALSE );  #GUI_TEST � �viter Qt3 voir pour le manuel
	static bool triedResolve = FALSE;
	if ( !ptrShell_NotifyIcon && !triedResolve )
	{
		triedResolve = TRUE;
		ptrShell_NotifyIcon = ( PtrShell_NotifyIcon ) lib.resolve( "Shell_NotifyIconW" );
	}
}

class RzxTrayIcon::RzxTrayIconPrivate : public QWidget
{
	public:
		HICON	hIcon;
		HBITMAP hMask;
		RzxTrayIcon	*iconObject;

		RzxTrayIconPrivate( RzxTrayIcon *object )
				: QWidget( 0 ), hIcon( 0 ), hMask( 0 ), iconObject( object )
		{
			if ( !WM_TASKBARCREATED )
				WM_TASKBARCREATED = RegisterWindowMessage( TEXT( "TaskbarCreated" ) );
		}

		~RzxTrayIconPrivate()
		{
			if ( hMask )
				DeleteObject( hMask );
			if ( hIcon )
				DestroyIcon( hIcon );
		}

		// the unavoidable A/W versions. Don't forget to keep them in sync!
		bool trayMessageA( DWORD msg )
		{
			NOTIFYICONDATAA tnd;
			ZeroMemory( &tnd, sizeof( NOTIFYICONDATAA ) );
			tnd.cbSize	= sizeof( NOTIFYICONDATAA );
			tnd.hWnd	= winId();

			if ( msg != NIM_DELETE )
			{
				tnd.uFlags	= NIF_MESSAGE | NIF_ICON | NIF_TIP;
				tnd.uCallbackMessage = WM_NOTIFYICON;
				tnd.hIcon	= hIcon;
				if ( !iconObject->toolTip().isNull() )
				{
					// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
					QString tip = iconObject->toolTip().left( 63 ) + QChar();
					lstrcpynA( tnd.szTip, ( const char* ) tip.toLocal8Bit(), qMin( tip.length() + 1, 64 ) );
				}
			}

			return Shell_NotifyIconA( msg, &tnd );
		}

#ifdef UNICODE
		bool trayMessageW( DWORD msg )
		{
			resolveLibs();
			if ( ! ( ptrShell_NotifyIcon && QSysInfo::WinVersion() & QSysInfo::WV_NT_based ) )
				return trayMessageA( msg );

			NOTIFYICONDATAW tnd;
			ZeroMemory( &tnd, sizeof( NOTIFYICONDATAW ) );
			tnd.cbSize	= sizeof( NOTIFYICONDATAW );
			tnd.hWnd	= winId();

			if ( msg != NIM_DELETE )
			{
				tnd.uFlags	= NIF_MESSAGE | NIF_ICON | NIF_TIP;
				tnd.uCallbackMessage = WM_NOTIFYICON;
				tnd.hIcon	= hIcon;
				if ( !iconObject->toolTip().isNull() )
				{
					// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
					QString tip = iconObject->toolTip().left( 63 ) + QChar();
					lstrcpynW( tnd.szTip, ( TCHAR* ) tip.unicode(), qMin( tip.length() + 1, 64 ) );
					//		lstrcpynW(tnd.szTip, (TCHAR*)qt_winTchar( tip, FALSE ), QMIN( tip.length()+1, 64 ) );
				}
			}
			return ptrShell_NotifyIcon( msg, &tnd );
		}
#endif

		bool trayMessage( DWORD msg )
		{
			QT_WA(
			    return trayMessageW( msg );
			    ,
			    return trayMessageA( msg );
			)
		}

		bool iconDrawItem( LPDRAWITEMSTRUCT lpdi )
		{
			if ( !hIcon )
				return FALSE;

			DrawIconEx( lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon, 0, 0, 0, NULL, DI_NORMAL );
			return TRUE;
		}

		bool winEvent( MSG *m , long *result )
		{
			switch ( m->message )
			{
				case WM_DRAWITEM:
					return iconDrawItem( ( LPDRAWITEMSTRUCT ) m->lParam );
				case WM_NOTIFYICON:
					{
						QMouseEvent *e = 0;
						QPoint gpos = QCursor::pos();
						switch ( m->lParam )
						{
							case WM_MOUSEMOVE:
								e = new QMouseEvent( QEvent::MouseMove, mapFromGlobal( gpos ), gpos, Qt::NoButton, Qt::NoButton,Qt::NoModifier );
								break;
							case WM_LBUTTONDOWN:
								e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, Qt::LeftButton, Qt::LeftButton  ,  Qt::NoModifier);
								break;
							case WM_LBUTTONUP:
								e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, Qt::LeftButton, Qt::LeftButton  ,Qt::NoModifier );
								break;
							case WM_LBUTTONDBLCLK:
								e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, Qt::LeftButton, Qt::LeftButton , Qt::NoModifier);
								break;
							case WM_RBUTTONDOWN:
								e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, Qt::RightButton, Qt::RightButton ,Qt::NoModifier );
								break;
							case WM_RBUTTONUP:
								e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, Qt::RightButton, Qt::RightButton , Qt::NoModifier );
								break;
							case WM_RBUTTONDBLCLK:
								e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
								break;
							case WM_MBUTTONDOWN:
								e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, Qt::MidButton, Qt::MidButton , Qt::NoModifier);
								break;
							case WM_MBUTTONUP:
								e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, Qt::MidButton, Qt::MidButton , Qt::NoModifier);
								break;
							case WM_MBUTTONDBLCLK:
								e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, Qt::MidButton, Qt::MidButton , Qt::NoModifier);
								break;
							case WM_CONTEXTMENU:
								e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, Qt::RightButton, Qt::RightButton , Qt::NoModifier);
								break;
						}
						if ( e )
						{
							bool res = QApplication::sendEvent( iconObject, e );
							delete e;
							return res;
						}
					}
					break;
				default:
					if ( m->message == WM_TASKBARCREATED )
						trayMessage( NIM_ADD );
			}
			return QWidget::winEvent( m , result );
		}
};

static HBITMAP createIconMask( const QPixmap &qp )
{
	QImage bm = qp.toImage();
	int w = bm.width();
	int h = bm.height();
	int bpl = ( ( w + 15 ) / 16 ) * 2;			// bpl, 16 bit alignment
	uchar *bits = new uchar[ bpl * h ];
	bm.invertPixels();
	for ( int y = 0; y < h; y++ )
		memcpy( bits + y * bpl, bm.scanLine( y ), bpl );
	HBITMAP hbm = CreateBitmap( w, h, 1, 1, bits );
	delete [] bits;
	return hbm;
}

static HICON createIcon( const QPixmap &pm, HBITMAP &hbm )
{
	/* QPixmap maskpm( pm.size());
	    QBitmap mask( pm.size() );
	    if ( !pm.mask().isNull() ) {
	        maskpm.fill( Qt::black );			// make masked area black
	        bitBlt(&mask, 0, 0, &pm.mask() );
	    } else
	        maskpm.fill( Qt::color1 );
	 
	    bitBlt( &maskpm, 0, 0, &pm);
	*/
	ICONINFO iconInfo;
	iconInfo.fIcon = TRUE;
	iconInfo.hbmMask = pm.toWinHBITMAP( QPixmap::PremultipliedAlpha );
	hbm = iconInfo.hbmMask;
	iconInfo.hbmColor = pm.toWinHBITMAP( QPixmap::PremultipliedAlpha );

	return CreateIconIndirect( &iconInfo );
}

void RzxTrayIcon::sysInstall()
{
	if ( !d )
	{
		d = new RzxTrayIconPrivate( this );
		d->hIcon = createIcon( pm, d->hMask );

		d->trayMessage( NIM_ADD );
	}
}

void RzxTrayIcon::sysRemove()
{
	if ( d )
	{
		d->trayMessage( NIM_DELETE );

		delete d;
		d = 0;
	}
}

void RzxTrayIcon::sysUpdateIcon()
{
	if ( d )
	{
		if ( d->hMask )
			DeleteObject( d->hMask );
		if ( d->hIcon )
			DestroyIcon( d->hIcon );

		d->hIcon = createIcon( pm, d->hMask );
		d->trayMessage( NIM_MODIFY );
	}
}

void RzxTrayIcon::sysUpdateToolTip()
{
	if ( d )
		d->trayMessage( NIM_MODIFY );
}

#else
#ifdef Q_OS_MAC

void RzxTrayIcon::sysInstall()
{}

void RzxTrayIcon::sysRemove()
{}

void RzxTrayIcon::sysUpdateIcon()
{}

void RzxTrayIcon::sysUpdateToolTip()
{}

#else 
/*
* trayicon_x11.cpp - X11 trayicon (for use with KDE and GNOME)
* Copyright (C) 2003  Justin Karneges
* GNOME2 Notification Area support: Tomasz Sterna
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include<QX11Info>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/Xatom.h>

//#if QT_VERSION < 0x030200
extern Time qt_x_time;
//#endif

//----------------------------------------------------------------------------
// common stuff
//----------------------------------------------------------------------------

// for Gnome2 Notification Area
static XErrorHandler old_handler = 0;
static int dock_xerror = 0;
extern "C" int dock_xerrhandler( Display* dpy, XErrorEvent* err )
{
	dock_xerror = err->error_code;
	return old_handler( dpy, err );
}

static void trap_errors()
{
	dock_xerror = 0;
	old_handler = XSetErrorHandler( dock_xerrhandler );
}

static bool untrap_errors()
{
	XSetErrorHandler( old_handler );
	return ( dock_xerror == 0 );
}

static bool send_message(
	Display* dpy, 	/* display */
	Window w, 	/* sender (tray icon window) */
	long message, 	/* message opcode */
	long data1, 	/* message data 1 */
	long data2, 	/* message data 2 */
	long data3	/* message data 3 */
)
{
	XEvent ev;

	memset( &ev, 0, sizeof( ev ) );
	ev.xclient.type = ClientMessage;
	ev.xclient.window = w;
	ev.xclient.message_type = XInternAtom ( dpy, "_NET_SYSTEM_TRAY_OPCODE", False );
	ev.xclient.format = 32;
	ev.xclient.data.l[ 0 ] = CurrentTime;
	ev.xclient.data.l[ 1 ] = message;
	ev.xclient.data.l[ 2 ] = data1;
	ev.xclient.data.l[ 3 ] = data2;
	ev.xclient.data.l[ 4 ] = data3;

	trap_errors();
	XSendEvent( dpy, w, False, NoEventMask, &ev );
	XSync( dpy, False );
	return untrap_errors();
}

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

//----------------------------------------------------------------------------
// RzxTrayIcon::RzxTrayIconPrivate
//----------------------------------------------------------------------------

class RzxTrayIcon::RzxTrayIconPrivate : public QWidget
{
	public:
		RzxTrayIconPrivate( RzxTrayIcon *object, int size );
		~RzxTrayIconPrivate()
		{ }

		virtual void initWM( WId icon );

		virtual void setPixmap( const QPixmap &pm );

		virtual void paintEvent( QPaintEvent * );
		virtual void enterEvent( QEvent * );
		virtual void mouseMoveEvent( QMouseEvent *e );
		virtual void mousePressEvent( QMouseEvent *e );
		virtual void mouseReleaseEvent( QMouseEvent *e );
		virtual void mouseDoubleClickEvent( QMouseEvent *e );
		virtual void closeEvent( QCloseEvent *e );
		virtual void resizeEvent( QResizeEvent *e);

	private:
		RzxTrayIcon *iconObject;
		QPixmap pix;
		int size;
};

RzxTrayIcon::RzxTrayIconPrivate::RzxTrayIconPrivate( RzxTrayIcon *object, int _size )
		: QWidget()
{
	iconObject = object;
	size = _size;

	setFocusPolicy( Qt::NoFocus );

	setMinimumSize( size, size );
	setMaximumSize( size, size );
}

// This base stuff is required by both FreeDesktop specification and WindowMaker
void RzxTrayIcon::RzxTrayIconPrivate::initWM( WId icon )
{
	Display * dsp = QX11Info::display();
	WId leader = winId();

	// set the class hint
	XClassHint classhint;
	classhint.res_name = ( char* ) "psidock";
	classhint.res_class = ( char* ) "Psi";
	XSetClassHint( dsp, leader, &classhint );

	// set the Window Manager hints
	XWMHints *hints;
	hints = XGetWMHints( dsp, leader );	// init hints
	hints->flags = WindowGroupHint | IconWindowHint | StateHint;	// set the window group hint
	hints->window_group = leader;		// set the window hint
	hints->initial_state = WithdrawnState;	// initial state
	hints->icon_window = icon;		// in WM, this should be winId() of separate widget
	hints->icon_x = 0;
	hints->icon_y = 0;
	XSetWMHints( dsp, leader, hints );	// set the window hints for WM to use.
	XFree( hints );
}

///Construit l'ic�ne � utiliser
/** Cette ic�ne est centr�e et sa taille est celle d�finie par l'utilisateur
 * via la fen�tre de propri�t�s
 */
void RzxTrayIcon::RzxTrayIconPrivate::setPixmap( const QPixmap &pm )
{
	pix = QPixmap(width(), height());
	pix.fill(QColor(0,0,0,0));
	QPainter painter(&pix);
	int dim = RzxTrayConfig::traySize();
	if(RzxTrayConfig::autoScale() || dim == -1)
		dim = qMin(width(), height()) - 2;
	painter.drawPixmap((width() - dim) / 2, (height() - dim)/2,
		 pm.scaled( dim, dim, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ));
	setWindowIcon( pix );
	setMask( pix.mask() );
	repaint();
}

///Dessine l'ic�ne
void RzxTrayIcon::RzxTrayIconPrivate::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	p.drawPixmap( 0, 0, pix );
}

///Raffraichi l'ic�ne lorsque la tray est resiz�e
void RzxTrayIcon::RzxTrayIconPrivate::resizeEvent( QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	setPixmap(iconObject->pm);
}

void RzxTrayIcon::RzxTrayIconPrivate::enterEvent( QEvent *e )
{
	// Taken from KSystemTray..
	XEvent ev;
	memset( &ev, 0, sizeof( ev ) );
	ev.xfocus.display = QX11Info::display();
	ev.xfocus.type = FocusIn;
	ev.xfocus.window = winId();
	ev.xfocus.mode = NotifyNormal;
	ev.xfocus.detail = NotifyAncestor;
	Time time = QX11Info::appTime();
	QX11Info::setAppTime( 1 );
	qApp->x11ProcessEvent( &ev );
	QX11Info::setAppTime( time );
	QWidget::enterEvent( e );
}

void RzxTrayIcon::RzxTrayIconPrivate::mouseMoveEvent( QMouseEvent *e )
{
	QApplication::sendEvent( iconObject, e );
}

void RzxTrayIcon::RzxTrayIconPrivate::mousePressEvent( QMouseEvent *e )
{
	QApplication::sendEvent( iconObject, e );
}

void RzxTrayIcon::RzxTrayIconPrivate::mouseReleaseEvent( QMouseEvent *e )
{
	QApplication::sendEvent( iconObject, e );
}

void RzxTrayIcon::RzxTrayIconPrivate::mouseDoubleClickEvent( QMouseEvent *e )
{
	QApplication::sendEvent( iconObject, e );
}

void RzxTrayIcon::RzxTrayIconPrivate::closeEvent( QCloseEvent *e )
{
	iconObject->gotCloseEvent();
	e->accept();
}

//----------------------------------------------------------------------------
// RzxTrayIconFreeDesktop
//----------------------------------------------------------------------------

class RzxTrayIconFreeDesktop : public RzxTrayIcon::RzxTrayIconPrivate
{
	public:
		RzxTrayIconFreeDesktop( RzxTrayIcon *object, const QPixmap &pm );
};

RzxTrayIconFreeDesktop::RzxTrayIconFreeDesktop( RzxTrayIcon *object, const QPixmap &pm )
		: RzxTrayIconPrivate( object, 22 )
{
	initWM( winId() );

	// initialize NetWM
	Display *dsp = QX11Info::display();

	// dock the widget (adapted from SIM-ICQ)
	Screen *screen = XDefaultScreenOfDisplay( dsp ); // get the screen
	int screen_id = XScreenNumberOfScreen( screen ); // and it's number

	char buf[ 32 ];
	snprintf( buf, sizeof( buf ), "_NET_SYSTEM_TRAY_S%d", screen_id );
	Atom selection_atom = XInternAtom( dsp, buf, false );
	XGrabServer( dsp );
	Window manager_window = XGetSelectionOwner( dsp, selection_atom );
	if ( manager_window != None )
		XSelectInput( dsp, manager_window, StructureNotifyMask );
	XUngrabServer( dsp );
	XFlush( dsp );

	if ( manager_window != None )
		send_message( dsp, manager_window, SYSTEM_TRAY_REQUEST_DOCK, winId(), 0, 0 );

	// some KDE mumbo-jumbo... why is it there? anybody?
	Atom kwm_dockwindow_atom = XInternAtom( dsp, "KWM_DOCKWINDOW", false );
	Atom kde_net_system_tray_window_for_atom = XInternAtom( dsp, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false );

	long data = 0;
	XChangeProperty( dsp, winId(), kwm_dockwindow_atom, kwm_dockwindow_atom, 32, PropModeReplace, ( uchar* ) & data, 1 );
	XChangeProperty( dsp, winId(), kde_net_system_tray_window_for_atom, XA_WINDOW, 32, PropModeReplace, ( uchar* ) & data, 1 );

	setPixmap( pm );
}

//----------------------------------------------------------------------------
// RzxTrayIconWindowMaker
//----------------------------------------------------------------------------

class RzxTrayIconWharf : public RzxTrayIcon::RzxTrayIconPrivate
{
	public:
		RzxTrayIconWharf( RzxTrayIcon *object, const QPixmap &pm )
				: RzxTrayIconPrivate( object, 64 )
		{
			setPixmap( pm );
		}

		void setPixmap( const QPixmap &pm )
		{
			RzxTrayIconPrivate::setPixmap( pm.scaled( pm.width() * 2, pm.height() * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) );

			// thanks to Robert Spier for this:
			// for some reason the repaint() isn't being honored, or isn't for
			// the icon.  So force one on the widget behind the icon
			update();
			/*		QPaintEvent pe( rect() );
			paintEvent(&pe);*/
		}
};

class RzxTrayIconWindowMaker : public RzxTrayIcon::RzxTrayIconPrivate
{
	public:
		RzxTrayIconWindowMaker( RzxTrayIcon *object, const QPixmap &pm );
		~RzxTrayIconWindowMaker();

		void setPixmap( const QPixmap &pm );

	private:
		RzxTrayIconWharf *wharf;
};

RzxTrayIconWindowMaker::RzxTrayIconWindowMaker( RzxTrayIcon *object, const QPixmap &pm )
		: RzxTrayIconPrivate( object, 32 )
{
	wharf = new RzxTrayIconWharf( object, pm );

	initWM( wharf->winId() );
}

RzxTrayIconWindowMaker::~RzxTrayIconWindowMaker()
{
	delete wharf;
}

void RzxTrayIconWindowMaker::setPixmap( const QPixmap &pm )
{
	wharf->setPixmap( pm );
}

//----------------------------------------------------------------------------
// RzxTrayIcon
//----------------------------------------------------------------------------

void RzxTrayIcon::sysInstall()
{
	if ( d )
		return ;

	if ( v_isWMDock )
		d = ( RzxTrayIconPrivate * ) ( new RzxTrayIconWindowMaker( this, pm ) );
	else
		d = ( RzxTrayIconPrivate * ) ( new RzxTrayIconFreeDesktop( this, pm ) );

	sysUpdateToolTip();
	d->show();
}

void RzxTrayIcon::sysRemove()
{
	if ( !d )
		return ;

	delete d;
	d = 0;
}

void RzxTrayIcon::sysUpdateIcon()
{
	if ( !d )
		return ;

	QPixmap pix = pm;
	d->setPixmap( pix );
}

void RzxTrayIcon::sysUpdateToolTip()
{
	if ( !d )
		return ;

	setToolTip( tip );
}

#endif
#endif

/*!
  \property RzxTrayIcon::toolTip
  \brief the tooltip for the system tray entry
 
  On some systems, the tooltip's length is limited and will be truncated as necessary.
*/
void RzxTrayIcon::setToolTip( const QString &tooltip )
{
	tip = tooltip;
#ifndef Q_OS_MAC
	if(d)
		d->setToolTip(tip);
#endif
}

QPoint RzxTrayIcon::getPos()
{
#ifdef Q_OS_MAC
	return QPoint( 0, 0 );
#else

	return d->mapToGlobal( d->pos() );
#endif
}


#ifndef Q_OS_MAC
#include "ui_rzxtrayprop.h"

/** \reimp */
QList<QWidget*> RzxTrayIcon::propWidgets()
{
	if(!ui)
		ui = new Ui::RzxTrayProp;
	if(!propWidget)
	{
		propWidget = new QWidget;
		ui->setupUi(propWidget);
#ifndef Q_WS_X11
		ui->groupSize->hide();
#endif
	}
	return QList<QWidget*>() << propWidget;
}

/** \reimp */
QStringList RzxTrayIcon::propWidgetsName()
{
	return QStringList() << name();
}

///Change le th�me d'ic�ne dans le fen�tre de pr�f�rences
void RzxTrayIcon::changePropTheme()
{
	if(!ui) return;

	ui->cbQuickChat->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CHAT));
	ui->cbQuickFtp->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FTP));
	ui->cbQuickHttp->setIcon(RzxIconCollection::getIcon(Rzx::ICON_HTTP));
	ui->cbQuickSamba->setIcon(RzxIconCollection::getIcon(Rzx::ICON_SAMBA));
	ui->cbQuickNews->setIcon(RzxIconCollection::getIcon(Rzx::ICON_NEWS));
	ui->cbQuickMail->setIcon(RzxIconCollection::getIcon(Rzx::ICON_MAIL));
	ui->cbQuickFav->setIcon(RzxIconCollection::getIcon(Rzx::ICON_FAVORITE));
}

/** \reimp */
void RzxTrayIcon::propInit(bool def)
{
	ui->cbAutoScale->setChecked(RzxTrayConfig::autoScale());
	int size = RzxTrayConfig::traySize(def);
	if(size == -1)
	{
		size = 22;
		if(d) size = qMin(d->width(), d->height()) - 2;
	}
	ui->sbTraySize->setValue(size);

	RzxTrayConfig::QuickActions actions = (RzxTrayConfig::QuickAction)RzxTrayConfig::quickActions();
	ui->cbQuickChat->setChecked(actions & RzxTrayConfig::Chat);
	ui->cbQuickFtp->setChecked(actions & RzxTrayConfig::Ftp);
	ui->cbQuickHttp->setChecked(actions & RzxTrayConfig::Http);
	ui->cbQuickSamba->setChecked(actions & RzxTrayConfig::Samba);
	ui->cbQuickNews->setChecked(actions & RzxTrayConfig::News);
	ui->cbQuickMail->setChecked(actions & RzxTrayConfig::Mail);
	ui->cbQuickFav->setChecked(actions & RzxTrayConfig::ChatFav);
	
	changePropTheme();
}

/** \reimp */
void RzxTrayIcon::propUpdate()
{
	if(!ui) return;

	if(RzxTrayConfig::autoScale() ^ ui->cbAutoScale->isChecked())
	{
		RzxTrayConfig::setAutoScale(ui->cbAutoScale->isChecked());
		changeTrayIcon();
	}
	if(ui->sbTraySize->value() != RzxTrayConfig::traySize() && !RzxTrayConfig::autoScale())
	{
		RzxTrayConfig::setTraySize(ui->sbTraySize->value());
		changeTrayIcon();
	}

	RzxTrayConfig::QuickActions actions;
	if(ui->cbQuickChat->isChecked()) actions |= RzxTrayConfig::Chat;
	if(ui->cbQuickFtp->isChecked()) actions |= RzxTrayConfig::Ftp;
	if(ui->cbQuickHttp->isChecked()) actions |= RzxTrayConfig::Http;
	if(ui->cbQuickSamba->isChecked()) actions |= RzxTrayConfig::Samba;
	if(ui->cbQuickNews->isChecked()) actions |= RzxTrayConfig::News;
	if(ui->cbQuickMail->isChecked()) actions |= RzxTrayConfig::Mail;
	if(ui->cbQuickFav->isChecked()) actions |= RzxTrayConfig::ChatFav;
	RzxTrayConfig::setQuickActions(actions);
}

/** \reimp */
void RzxTrayIcon::propClose()
{
	if(propWidget)
	{
		delete propWidget;
		propWidget = NULL;
	}
	if(ui)
	{
		delete ui;
		ui = NULL;
	}
}

///Mise � jour de la traduction
void RzxTrayIcon::translate()
{
	if(ui)
		ui->retranslateUi(propWidget);
}

#endif

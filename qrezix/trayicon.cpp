#include "trayicon.h"
#include "qpopupmenu.h"
#include <qtooltip.h>

#ifdef WIN32
#include "qt_windows.h"
#include <qimage.h>
static uint MYWM_TASKBARCREATED = 0;
#define MYWM_NOTIFYICON	(WM_APP+101)
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

TrayIcon::TrayIcon( QObject *parent, const char *name ) : QObject(parent, name), pop(0), d(0) {
}

TrayIcon::TrayIcon( const QPixmap &icon, const QString &tooltip,
		QPopupMenu *popup, QObject *parent, const char *name )
		 : QObject(parent, name), pop(popup), pm(icon), tip(tooltip), d(0) {
}

TrayIcon::~TrayIcon() {
	sysRemove();
}

void TrayIcon::setPopup( QPopupMenu* p ) {
	pop = p;
}

QPopupMenu* TrayIcon::popup() const {
	return pop;
}

void TrayIcon::setIcon( const QPixmap &icon ) {
	pm = icon;
	sysUpdateIcon();
}

QPixmap TrayIcon::icon() const {
	return pm;
}

void TrayIcon::setToolTip( const QString &tooltip ) {
	tip = tooltip;
	sysUpdateToolTip();
}

QString TrayIcon::toolTip() const {
	return tip;
}

void TrayIcon::setVisible(bool yes) {
	if (yes) show();
	else hide();
}

void TrayIcon::show() {
	sysInstall();
}

void TrayIcon::hide() {
	sysRemove();
}

bool TrayIcon::event( QEvent *e )
{
	switch ( e->type() ) {
		case QEvent::MouseMove:
			mouseMoveEvent( (QMouseEvent*)e );
			break;
			
		case QEvent::MouseButtonPress:
			mousePressEvent( (QMouseEvent*)e );
			break;
			
		case QEvent::MouseButtonRelease:
			mouseReleaseEvent( (QMouseEvent*)e );
			break;
			
		case QEvent::MouseButtonDblClick:
			mouseDoubleClickEvent( (QMouseEvent*)e );
			break;
		default:
			return QObject::event( e );
	}
	return TRUE;
}

void TrayIcon::mouseMoveEvent( QMouseEvent *) { }

void TrayIcon::mousePressEvent( QMouseEvent *) { }

void TrayIcon::mouseReleaseEvent( QMouseEvent *e ) {
	switch ( e->button() ) {
	case RightButton:
		if ( pop ) {
			// Those lines are necessary to make keyboard focus
			// and menu closing work on Windows.
			pop->setActiveWindow();
			pop->grabMouse();
			pop->exec( e->globalPos() );
			pop->releaseMouse();
			pop->setActiveWindow();
		}
		break;
	case LeftButton:
		emit clicked( e->globalPos() );
		break;
		default: ;
	}
}

void TrayIcon::mouseDoubleClickEvent( QMouseEvent *e ) {
	if ( e->button() == LeftButton )
		emit doubleClicked( e->globalPos() );
}

/****************************************************************************************************
*
*   CLASSE PRIVATE WINDOWS
*
****************************************************************************************************/
#ifdef WIN32

TrayIconPrivate::TrayIconPrivate( TrayIcon *object )
: QWidget( 0 ), hIcon( 0 ), iconObject( object ) {
	if ( !MYWM_TASKBARCREATED ) {
#if defined(UNICODE)
		if ( QApplication::winVersion() & Qt::WV_NT_based )
			MYWM_TASKBARCREATED = RegisterWindowMessageW( (TCHAR*)"TaskbarCreated" );
		else
#endif /* defined(UNICODE) */
			MYWM_TASKBARCREATED = RegisterWindowMessageA( "TaskbarCreated" );
	}
}

TrayIconPrivate::~TrayIconPrivate() {if ( hIcon ) DestroyIcon( hIcon );}

bool TrayIconPrivate::trayMessageA( DWORD msg ) {
	bool res;
	NOTIFYICONDATAA tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATAA) );
	tnd.cbSize		= sizeof(NOTIFYICONDATAA);
	tnd.hWnd		= winId();
		
	if ( msg != NIM_DELETE ) {
		tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
		tnd.uCallbackMessage= MYWM_NOTIFYICON;
//		tnd.hIcon		= hIcon;
		 tnd.hIcon = LoadIcon(qWinAppInst(), MAKEINTRESOURCE(103)); // The icon that will be shown in the systray.

		if ( !iconObject->toolTip().isNull() ) {
			// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
			QString tip = iconObject->toolTip().left( 63 ) + QChar();
			lstrcpynA(tnd.szTip, (const char*)tip, QMIN( tip.length()+1, 64 ) );
		}
	}
	res = Shell_NotifyIconA(msg, &tnd);
	return res;
}

#if defined(UNICODE)
bool TrayIconPrivate::trayMessageW( DWORD msg ) {
	bool res;
	NOTIFYICONDATAW tnd;
	memset( &tnd, 0, sizeof(NOTIFYICONDATAW) );
	tnd.cbSize		= sizeof(NOTIFYICONDATAW);
	tnd.hWnd		= winId();
	
	if ( msg != NIM_DELETE ) {
		tnd.uFlags		= NIF_MESSAGE|NIF_ICON|NIF_TIP;
		tnd.uCallbackMessage= MYWM_NOTIFYICON;
		tnd.hIcon		= hIcon;
		if ( !iconObject->toolTip().isNull() ) {
			// Tip is limited to 63 + NULL; lstrcpyn appends a NULL terminator.
			QString tip = iconObject->toolTip().left( 63 ) + QChar();
			lstrcpynW(tnd.szTip, (TCHAR*)qt_winTchar( tip, FALSE ), QMIN( tip.length()+1, 64 ) );
		}
	}
	res = Shell_NotifyIconW(msg, &tnd);
	return res;
}
#endif /* defined(UNICODE) */

bool TrayIconPrivate::iconDrawItem(LPDRAWITEMSTRUCT lpdi) {
	if(!hIcon)return FALSE;
	DrawIconEx(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, hIcon, 0, 0, 0, NULL, DI_NORMAL );
	return TRUE;
}

bool TrayIconPrivate::winEvent( MSG *m ) {
	switch(m->message) {
		case WM_DRAWITEM:
			return iconDrawItem( (LPDRAWITEMSTRUCT)m->lParam );
		case MYWM_NOTIFYICON: {
			QMouseEvent *e = 0;
			QPoint gpos = QCursor::pos();
			switch (m->lParam) {
			case WM_MOUSEMOVE:
				e = new QMouseEvent( QEvent::MouseMove, mapFromGlobal( gpos ), gpos, 0, 0 );
				break;
			case WM_LBUTTONDOWN:
				e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
				break;
			case WM_LBUTTONUP:
				e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
				break;
			case WM_LBUTTONDBLCLK:
				e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, LeftButton, LeftButton );
				break;
			case WM_RBUTTONDOWN:
				e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
				break;
			case WM_RBUTTONUP:
				e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
				break;
			case WM_RBUTTONDBLCLK:
				e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
				break;
			case WM_MBUTTONDOWN:
				e = new QMouseEvent( QEvent::MouseButtonPress, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
				break;
			case WM_MBUTTONUP:
				e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
				
				break;
			case WM_MBUTTONDBLCLK:
				e = new QMouseEvent( QEvent::MouseButtonDblClick, mapFromGlobal( gpos ), gpos, MidButton, MidButton );
				break;
			case WM_CONTEXTMENU:
				e = new QMouseEvent( QEvent::MouseButtonRelease, mapFromGlobal( gpos ), gpos, RightButton, RightButton );
				break;
			default: ;
			}
			if ( e ) {
				bool res = QApplication::sendEvent( iconObject, e );
				delete e;
				return res;
			}
													}
			break;
		default:
			if ( m->message == MYWM_TASKBARCREATED ) {
#if defined(UNICODE)
				if ( QApplication::winVersion() & Qt::WV_NT_based )
					trayMessageW( NIM_ADD );
				else
#endif /*defined(UNICODE) */
					trayMessageA( NIM_ADD );
			}
			break;
	}
	return QWidget::winEvent( m );
}

void TrayIcon::sysInstall()
{
	if ( d )
		return;
	
	d = new TrayIconPrivate( this );
	QImage img = pm.convertToImage();
	d->hIcon = CreateIcon( qWinAppInst(), img.width(), img.height(), 1, img.depth(), 0, img.bits() );
	
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
		d->trayMessageW( NIM_ADD );
	else
#endif /*defined(UNICODE) */
		d->trayMessageA( NIM_ADD );
}

void TrayIcon::sysRemove()
{
	if ( !d )
		return;
	
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
		d->trayMessageW( NIM_DELETE );
	else
#endif /*defined(UNICODE) */
		d->trayMessageA( NIM_DELETE );
	
	delete d;
	d = 0;
}

void TrayIcon::sysUpdateIcon()
{
	if ( !d )
		return;
	
	if ( d->hIcon )
		DestroyIcon( d->hIcon );
	
	QImage img = pm.convertToImage();
	d->hIcon = CreateIcon( qWinAppInst(), img.width(), img.height(), 1, img.depth(), 0, img.bits() );
	
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
		d->trayMessageW( NIM_MODIFY );
	else
#endif /*defined(UNICODE) */
		d->trayMessageA( NIM_MODIFY );
}

void TrayIcon::sysUpdateToolTip()
{
	if ( !d )
		return;
	
#if defined(UNICODE)
	if ( QApplication::winVersion() & Qt::WV_NT_based )
		d->trayMessageW( NIM_MODIFY );
	else
#endif /*defined(UNICODE) */
		d->trayMessageA( NIM_MODIFY );
}

/****************************************************************************************************
*
*   CLASSE PRIVATE LINUX
*
****************************************************************************************************/

#else /* WIN32 */
void TrayIcon::sysInstall() {
	if ( d ) return;
	d = new TrayIconPrivate( this, pm );
	QToolTip::add(d, tip);
	d->show();
}

void TrayIcon::sysRemove() {
	if ( !d ) return;
	delete d;
	d = 0;
}

void TrayIcon::sysUpdateIcon() {
	if ( !d ) return;
	QPixmap pix = pm;
	d->setPixmap(pix);
	//d->setMask(*pix.mask());
	d->repaint();
}

void TrayIcon::sysUpdateToolTip() {
	if ( !d ) return;
	if(tip.isEmpty())
		QToolTip::remove(d);
	else
		QToolTip::add(d, tip);
}

TrayIconPrivate::TrayIconPrivate( TrayIcon *object, const QPixmap &pm )
: QLabel(0), iconObject(object) {
	setWFlags(WRepaintNoErase);
	QPixmap pix = pm;
	QImage img = pm.convertToImage();
	img = img.smoothScale(22, 22);
	pix.convertFromImage(img);
	setPixmap(pix);
	Display *dsp = x11Display(); // get the display
	WId win = winId();           // get the window
	int r;
	int data = 1;
	r = XInternAtom(dsp, "KWM_DOCKWINDOW", false);
	XChangeProperty(dsp, win, r, r, 32, 0, (uchar *)&data, 1);
	r = XInternAtom(dsp, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
	XChangeProperty(dsp, win, r, XA_WINDOW, 32, 0, (uchar *)&data, 1);
}

TrayIconPrivate::~TrayIconPrivate() { }

void TrayIconPrivate::mouseMoveEvent( QMouseEvent *e ) {QApplication::sendEvent(iconObject, e);}
void TrayIconPrivate::mousePressEvent( QMouseEvent *e ) {QApplication::sendEvent(iconObject, e);}
void TrayIconPrivate::mouseReleaseEvent( QMouseEvent *e ) {QApplication::sendEvent(iconObject, e);}
void TrayIconPrivate::mouseDoubleClickEvent( QMouseEvent *e ) {QApplication::sendEvent(iconObject, e);}

#endif /* WIN32 */


#ifndef TRAYICON_H
#define TRAYICON_H

#include <qapplication.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qwidget.h>

#include "rzxconfig.h"

class QPopupMenu;
class TrayIcon;
#if defined(WIN32)
#include <windows.h>
class TrayIconPrivate : public QWidget
{

public:
    TrayIconPrivate( TrayIcon *object );
    ~TrayIconPrivate();
   bool trayMessageA( DWORD msg );
#if defined(UNICODE)
    bool trayMessageW( DWORD msg );
#endif
    bool iconDrawItem(LPDRAWITEMSTRUCT lpdi);
    bool winEvent( MSG *m );

    HICON		hIcon;
    TrayIcon		*iconObject;
};
#else
class TrayIconPrivate : public QLabel
{

public:
	TrayIconPrivate( TrayIcon *object, const QPixmap &pm );
    ~TrayIconPrivate();

	void mouseMoveEvent( QMouseEvent *e );
	void mousePressEvent( QMouseEvent *e );
	void mouseReleaseEvent( QMouseEvent *e );
	void mouseDoubleClickEvent( QMouseEvent *e );
	
	TrayIcon *iconObject;
};
#endif

class TrayIconPrivate;

class TrayIcon : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString toolTip READ toolTip WRITE setToolTip )
    Q_PROPERTY( QPixmap icon READ icon WRITE setIcon )

public:
    TrayIcon( QObject *parent = 0, const char *name = 0 );
    TrayIcon( const QPixmap &, const QString &, QPopupMenu *popup = 0, QObject *parent = 0, const char *name = 0 );
    ~TrayIcon();

			// Set a popup menu to handle RMB
    void		setPopup( QPopupMenu * );
    QPopupMenu*		popup() const;
    QPixmap		icon() const;
    QString		toolTip() const;

public slots:
    void		setIcon( const QPixmap &icon );
    void		setToolTip( const QString &tip );
	 void 	setVisible(bool yes);
    void		show();
    void		hide();

signals:
    void		clicked( const QPoint& );
    void		doubleClicked( const QPoint& );

protected:
    bool		event( QEvent * );
    virtual void	mouseMoveEvent( QMouseEvent *e );
    virtual void	mousePressEvent( QMouseEvent *e );
    virtual void	mouseReleaseEvent( QMouseEvent *e );
    virtual void	mouseDoubleClickEvent( QMouseEvent *e );

private:
    QPopupMenu *pop;
    QPixmap pm;
    QString tip;

    // system-dependant part
    TrayIconPrivate *d;
    void sysInstall();
    void sysRemove();
    void sysUpdateIcon();
    void sysUpdateToolTip();
};

#endif //QTRAYICON_H

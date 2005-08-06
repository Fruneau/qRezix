/*
 * trayicon.h - system-independent trayicon class (adapted from Qt example)
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

#ifndef CS_TRAYICON_H
#define CS_TRAYICON_H

#include <QObject>
#include <QImage>
#include <QMenu>
#include <QPoint>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>

#include "../core/rzxmodule.h"


///Gestion de l'icône système (systray, 'tableau de bord'...)
/** Cette classe fournit une gestion avec tooltip et actions selon les clics */
class RzxTrayIcon : public RzxModule
{
	Q_OBJECT

	Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip )
	Q_PROPERTY(QPixmap trayIcon READ trayIcon WRITE setTrayIcon )

public:
	RzxTrayIcon();
	~RzxTrayIcon();

	// use WindowMaker dock mode.  ignored on non-X11 platforms
	void setWMDock(bool use) { v_isWMDock = use; }
	bool isWMDock() { return v_isWMDock; }

	QPixmap trayIcon() const;
	QString toolTip() const;

	void gotCloseEvent();
	virtual bool isInitialised() const;

	QPoint getPos();

public slots:
	void changeTrayIcon();
	void setTrayIcon( const QPixmap &icon );
	void setToolTip( const QString &tip );

	void buildMenu();
	void setVisible(bool yes);
	void show();
	void hide();

signals:
	void clicked(const QPoint&);
	void doubleClicked(const QPoint&);
	void closed();

protected:
	bool event( QEvent * );
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void mouseDoubleClickEvent( QMouseEvent *e );

private:
	QMenu pop;
	QPixmap pm;
	QString tip;
	bool v_isWMDock;

	// system-dependant part
public:
	class RzxTrayIconPrivate;
private:
	RzxTrayIconPrivate *d;
	void sysInstall();
	void sysRemove();
	void sysUpdateIcon();
	void sysUpdateToolTip();

	friend class RzxTrayIconPrivate;
};

#endif // CS_TRAYICON_H

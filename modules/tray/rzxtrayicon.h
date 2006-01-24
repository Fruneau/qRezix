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
 * Adaptation to qRezix by Florent Bruneau, Copyright (C) 2004-2005 Binet Réseau
 */

#ifndef CS_TRAYICON_H
#define CS_TRAYICON_H

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_TRAYICON_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <QObject>
#include <QImage>
#include <QMenu>
#include <QPoint>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>

#include <RzxModule>

#ifndef Q_OS_MAC
namespace Ui { class RzxTrayProp; }
#endif

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
	virtual void show();
	virtual void hide();

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

	QString titleFromSplit(int j, QString splitPoints);
	int     calculerSplit(QString& splitPoints, QList<RzxComputer*> list);
	void    subMenu(QMenu& mname, Rzx::Icon micon, const char * slot, QList<RzxComputer*> list, QList<RzxComputer*> fav);

private:
	QMenu pop;
	QMenu chat;
	QMenu ftp;
	QMenu http;
	QMenu news;
	QMenu samba;
	QMenu mail;
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

#ifndef Q_OS_MAC
private:
	Ui::RzxTrayProp *ui;
	QWidget *propWidget;
	void changePropTheme();

public:
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();

public slots:
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();

protected slots:
	void translate();
#endif
};

#endif // CS_TRAYICON_H

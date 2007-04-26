/*
 * trayicon.h - trayicon class
 * Florent Bruneau, Copyright (C) 2004-2005 Binet Réseau
 * Ported to Qt 4 by Guillaume Bandet, 2007
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

#ifndef RZXTRAYICON_H
#define RZXTRAYICON_H

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_TRAYICON_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif

#include <QSystemTrayIcon>
#include <QMenu>

#include <RzxModule>

//#ifndef Q_OS_MAC
namespace Ui { class RzxTrayProp; }
//#endif

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

	QPixmap trayIcon() const;
	QString toolTip() const;
	virtual bool isInitialised() const;	//TODO a voir

public slots:
	void changeTrayIcon();
	void buildMenu();
	void setTrayIcon( const QPixmap &icon );
	void setToolTip( const QString &tip );

protected:
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
#ifdef Q_OS_MAC
        QMenu dock;
#endif
        QSystemTrayIcon* tray;

// system-dependant part
public:

private:
	Ui::RzxTrayProp *ui;
	QWidget *propWidget;
	void changePropTheme();
	QColor bg;

public:
	virtual QList<QWidget*> propWidgets();
	virtual QStringList propWidgetsName();

public slots:
	virtual void propInit(bool def = false);
	virtual void propUpdate();
	virtual void propClose();

protected slots:
	void trayActivated(QSystemTrayIcon::ActivationReason reason);
	void translate();
	void updateBackground();
	void selectColor();
};

#endif

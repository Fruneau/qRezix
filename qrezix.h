/***************************************************************************
                          qrezix.h  -  description
                             -------------------
    begin                : lun jan 28 16:27:20 CET 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QREZIX_H
#define QREZIX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QWidget>
#include <QMenu>

#ifdef Q_OS_MAC
#include "ui_qrezixui_mac.h"
#else
#include "ui_qrezixui.h"
#endif

class TrayIcon;
class RzxProperty;
class RzxComputer;
class QEvent;
class QCloseEvent;
class QShortcut;

//class RzxChat;

/** QReziX is the base class of the project */
class QRezix : public QWidget, public Ui::qRezixUI
{
	Q_OBJECT
	
	RzxProperty * m_properties;
	QShortcut *accel;
	QMenu menuPlugins;
	static QRezix *object;
	bool statusFlag;
	bool favoriteWarn;
	
	
public:
    QRezix(QWidget* parent=0);
    ~QRezix();
	bool statusMax;
	bool alreadyOpened;
	TrayIcon * tray;
	bool byTray;
	bool wellInit;
	static QRezix *global();

	
signals:
	void setToolTip(const QString &);

protected:
	virtual void closeEvent(QCloseEvent * e);
	virtual bool event(QEvent * e);

public slots: // Public slots
	void status(const QString& msg, bool fatal);
	void toggleAutoResponder();
	void toggleButtonResponder();
	void activateAutoResponder( bool state );
	void chatSent();
	void languageChanged();
	void closeByTray();
	void changeTheme();
	void menuFormatChange();
	void saveSettings();
	void launchPlugins();
	void changeTrayIcon();
	void launchSearch();
	void boitePreferences();

protected slots: // Protected slots
	void delayedInit();
	/*affiche la boite de dialogue permettant de modifier les preferences*/
	void socketClosed();
	void toggleVisible();
	virtual void languageChange();
	void pluginsMenu(bool show = false);
	void switchTab();
	void warnForFavorite(RzxComputer *computer);
	void warnForDeparture(RzxComputer *computer);
	void newFavorite();
};

#endif

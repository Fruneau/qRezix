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

#include "qrezixui.h"
#include "trayicon.h"
#include <qdict.h>
#include <qtranslator.h>
#include <qpopupmenu.h>

class RzxProperty;

//class RzxChat;

/** QReziX is the base class of the project */
class QRezix : public QRezixUI
{
	Q_OBJECT
	
	RzxProperty * m_properties;
	QPopupMenu menuPlugins;
	static QRezix *object;
	
public:
    QRezix(QWidget* parent=0, const char *name=0);
    ~QRezix();
	bool statusMax;
	bool alreadyOpened;
	TrayIcon * tray;
	bool byTray;
	static QRezix *global();

signals:
	void setToolTip(const QString &);

protected:
	void closeEvent(QCloseEvent * e);
	bool event(QEvent * e);

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
	
protected slots: // Protected slots
	void delayedInit();
	/*affiche la boite de dialogue permettant de modifier les preferences*/
	void boitePreferences();
	void socketClosed();
	void toggleVisible();
	void languageChange();
	void pluginsMenu(bool show = false);
};

#endif

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
#include <QMainWindow>
#include <QMenu>

#include "ui_qrezixui.h"

class TrayIcon;
class RzxProperty;
class RzxComputer;
class QEvent;
class QCloseEvent;
class QShortcut;
class QAction;
class QLineEdit;


/** QRezix is the base class of the project */
class QRezix : public QMainWindow, public Ui::qRezixUI
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)
	
	QLineEdit *leSearch;
	QShortcut *accel;
	QMenu menuPlugins;
	bool statusFlag;
	bool statusMax;
	bool alreadyOpened;
	bool wellInit;
	
	QAction *pluginsAction;
	QAction *prefAction;
	QAction *columnsAction;
	QAction *searchAction;
	QAction *awayAction;

	void buildActions();

	static QRezix *object;
	QRezix(QWidget* parent = NULL);

public:
	static QRezix *global(QWidget *parent = NULL);
	~QRezix();
	bool isInitialised() const;

signals:
	void wantQuit();
	void wantPreferences();
	void wantToggleResponder();

protected:
	virtual void closeEvent(QCloseEvent * e);
	virtual void changeEvent(QEvent *e);
	virtual bool event(QEvent * e);

public slots: // Public slots
	void status(const QString& msg, bool fatal);
	void toggleAutoResponder();
	void activateAutoResponder( bool state );
	void changeTheme();
	void menuFormatChange();
	void launchSearch();
	void showSearch(bool state);
	void toggleVisible();

protected slots: // Protected slots
	void delayedInit();
	/*affiche la boite de dialogue permettant de modifier les preferences*/
	void socketClosed();
	void pluginsMenu();
	void switchTab();
};


///Retourne la fenêtre principale
/** La fenêtre est construite en cas de besoin.
 * Cette fonction est la seule qui permet d'obtenir une référence vers cette fenêtre.
 */
inline QRezix *QRezix::global(QWidget *parent)
{
	if(!object)
		new QRezix(parent);
	return object;
}

///Indique si l'objet est bien initialisé
inline bool QRezix::isInitialised() const
{
	return wellInit;
}

#endif

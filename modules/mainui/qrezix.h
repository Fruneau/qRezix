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
#include <QModelIndex>
#include <QPointer>

#include <RzxBaseLoader>

#include "rzxrezal.h"
#include "ui_rzxstatusui.h"
#include "rzxrezalpopup.h"

class RzxComputer;
class QEvent;
class QCloseEvent;
class QAction;
class QLineEdit;
class RzxRezalPopup;

/** QRezix is the base class of the project */
class Q_DECL_EXPORT QRezix : public QMainWindow, public RzxBaseLoader<RzxRezal>
{
	Q_OBJECT
	Q_PROPERTY(bool initialised READ isInitialised)
	RZX_GLOBAL(QRezix)

	RzxRezal *central;
	RzxRezal *index;
	
	QLineEdit *leSearch;
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

	Ui::RzxStatusUI *statusui;

	void buildActions();

	QRezix(QWidget* parent = NULL);
public:
	~QRezix();
	bool isInitialised() const;

signals:
	void wantQuit();
	void wantPreferences();
	void wantToggleResponder();

protected:
	void saveState();
	virtual void closeEvent(QCloseEvent * e);
	virtual bool event(QEvent * e);

	virtual void loadBuiltins();
	virtual bool installModule(RzxRezal*);
	virtual void linkModules();

public slots: // Public slots
	void status(const QString& msg, bool fatal);
	void toggleAutoResponder();
	void activateAutoResponder( bool state );
	void changeTheme();
	void menuFormatChange();
	void launchSearch();
	void showSearch(bool state);
	void setSearchPattern(const QString&);
	void toggleVisible();
	void updateLayout();

protected slots: // Protected slots
	void pluginsMenu();

#ifdef Q_OS_MAC
	private:
		QPointer<RzxRezalPopup> popup;

	protected slots:
		void setMenu(const QModelIndex& = QModelIndex(), const QModelIndex& = QModelIndex());
#endif
};

///Indique si l'objet est bien initialisé
inline bool QRezix::isInitialised() const
{
	return wellInit;
}

#endif

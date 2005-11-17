/***************************************************************************
                          rzxquit.h  -  description
                             -------------------
    begin                : Thu Jun 24 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXQUIT_H
#define RZXQUIT_H

#include <QDialog>

#include "ui_rzxquitui.h"

class RzxConfig;

class Q_DECL_EXPORT RzxQuit : public QDialog, private Ui::RzxQuitUI
{
	Q_OBJECT
	Q_PROPERTY(QuitMode quitMode READ quitMode)
	Q_ENUMS(QuitMode)

	public:
		enum QuitMode {
			selectQuit = 0x01,
			selectMinimize = 0x02,
			selectAbort = 0x04
		};

	private:
		QuitMode selection;

	public:
		RzxQuit(QWidget* parent=0);
	   ~RzxQuit();

		QuitMode quitMode() const;

	protected slots:
		void quitOptionChange();
		void on_btnApply_clicked();
};

#endif //RZXQUIT_H

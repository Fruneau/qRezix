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

#include "rzxmainuiglobal.h"
#include "ui_rzxquit.h"

///Fen�tre de confirmation fermeture
/** Cette fen�tre demande quelle action effectuer lorsque l'utilisateur ferme
 * la fen�tre principale...
 */
class RZX_MAINUI_EXPORT RzxQuit : public QDialog, private Ui::RzxQuit
{
	Q_OBJECT
	Q_PROPERTY(QuitMode quitMode READ quitMode)
	Q_ENUMS(QuitMode)

	public:
		enum QuitMode
		{
			None = 0x00,
			Quit = 0x01,
			Minimize = 0x02,
			Abort = 0x04
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

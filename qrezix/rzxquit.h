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

#include "rzxquitui.h"

class RzxConfig;

class RzxQuit : public RzxQuitUI
{
	Q_OBJECT
	friend class RzxConfig;

	int selection;

	public:
		enum QuitMode
		{
			selectQuit = 0x01,
			selectMinimize = 0x02,
			selectAbort = 0x04
		};

		 RzxQuit(QWidget* parent=0, const char *name=0);
	    ~RzxQuit();

	protected slots:
		void quitOptionChange(void);
		void done();
};


#endif //RZXQUIT_H

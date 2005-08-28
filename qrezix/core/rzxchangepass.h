/***************************************************************************
                         rzxchangepass  -  description
                            -------------------
   begin                : Sat Aug 27 2005
   copyright            : (C) 2005 by Florent Bruneau
   email                : florent.bruneau@m4x.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXCHANGEPASS_H
#define RZXCHANGEPASS_H

#include <QDialog>
#include "ui_rzxchangepassui.h"

class RzxNetwork;

/**
@author Florent Bruneau
*/

///Interface de changement de mot de passe
class RzxChangePass: public QDialog, private Ui::RzxChangePassUI
{
	Q_OBJECT

	RzxNetwork *network;

	private:
		void init(const QString&);

	public:
		RzxChangePass(RzxNetwork*, const QString& = QString());
		RzxChangePass(const QString& = QString());
		~RzxChangePass();

		QString newPass() const;

	protected slots:
		virtual void accept();
		void analyseNewPass();
};

#endif

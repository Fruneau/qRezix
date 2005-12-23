/***************************************************************************
                          rzxwrongpass  -  description
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
#ifndef RZXWRONGPASS_H
#define RZXWRONGPASS_H

#include <QDialog>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

class RzxNetwork;
namespace Ui { class RzxWrongPass; };

///Syst�me centralis� pour permettre � l'utilisateur d'�tre alert� que son mot de passe est erron�
/** Cette classe est fortement li�e � RzxNetwork mais peut tr�s bien �tre
 * utilis�e s�par�ment
 */
class RZX_CORE_EXPORT RzxWrongPass: public QDialog
{
	Q_OBJECT

	RzxNetwork *network;
	Ui::RzxWrongPass *ui;

	public:
		RzxWrongPass(RzxNetwork* = NULL);
		~RzxWrongPass();

		QString newPass() const;

	public slots:
		virtual void accept();
};

#endif

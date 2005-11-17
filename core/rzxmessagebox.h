/***************************************************************************
                          rzxmessagebox.h  -  description
                             -------------------
    begin                : Fri Jan 24 2003
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXMSGBOX_H
#define RZXMSGBOX_H

#include <QString>

class QWidget;

///Surcharge de certaines fonctions de QMessageBox
/** Le but est de fournir une API simple qui respecte les règles internes
 * de qRezix en permettant d'afficher des messages sans entraîner des
 * actions dommageables pour l'exécution du programme.
 */
namespace RzxMessageBox
{
	Q_DECL_EXPORT int information( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );

	Q_DECL_EXPORT int warning( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );

	Q_DECL_EXPORT int critical( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );
}


#endif //RZXMSGBOX_H

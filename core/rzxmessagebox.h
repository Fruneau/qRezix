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

#include <RzxGlobal>

class QWidget;

///Surcharge de certaines fonctions de QMessageBox
/** Le but est de fournir une API simple qui respecte les règles internes
 * de qRezix en permettant d'afficher des messages sans entraîner des
 * actions dommageables pour l'exécution du programme.
 */
namespace RzxMessageBox
{
	RZX_CORE_EXPORT QWidget *information( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );

	RZX_CORE_EXPORT QWidget *warning( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );

	RZX_CORE_EXPORT QWidget *critical( QWidget *parent, const QString& caption,
		const QString& text, bool modal =false );
}


#endif //RZXMSGBOX_H

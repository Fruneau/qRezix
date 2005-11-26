/***************************************************************************
                          rzxintro  -  description
                             -------------------
    begin                : Mon Nov 14 2005
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
#ifndef RZXINTRO_H
#define RZXINTRO_H

#include <QDialog>
#include <RzxGlobal>

#include "ui_rzxintroui.h"

///Fen�tre de choix de la configuration initiale
/** Affiche une fen�tre qui permet de choisir la langue et le th�me d'ic�ne
 * � utiliser dans qRezix.
 *
 * Cette fen�tre est automatiquement affich�e au premier d�marrage
 */
class RZX_CORE_EXPORT RzxIntro : public QDialog, private Ui::RzxIntroUI
{
	Q_OBJECT

	QList<Rzx::Icon> icons;

	public:
		RzxIntro();
		~RzxIntro();

	protected slots:
		void changeTheme(const QString& text);
		void changeLanguage(const QString& language);
};

#endif

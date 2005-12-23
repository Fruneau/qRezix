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

/**
 @author Florent Bruneau
 */

namespace Ui { class RzxIntro; };

///Fenêtre de choix de la configuration initiale
/** Affiche une fenêtre qui permet de choisir la langue et le thème d'icône
 * à utiliser dans qRezix.
 *
 * Cette fenêtre est automatiquement affichée au premier démarrage
 */
class RZX_CORE_EXPORT RzxIntro : public QDialog
{
	Q_OBJECT

	Ui::RzxIntro *ui;
	QList<Rzx::Icon> icons;

	public:
		RzxIntro();
		~RzxIntro();

	protected slots:
		void changeTheme(const QString& text);
		void changeLanguage(const QString& language);
};

#endif

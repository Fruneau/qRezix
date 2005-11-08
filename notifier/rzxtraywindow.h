/***************************************************************************
                               rzxtraywindow.h
         Gestion des fenêtres popups de notification de la trayicon
                             -------------------
    begin                : Tue Nov 16 2004
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

#include <QFrame>
#include <QTimer>

class RzxComputer;
class QEvent;

///Gestion de la fenêtre qui s'affiche lorsque les favoris changent d'état
/** Construit, affiche et détruit la fenêtre.
 *
 * Dans l'état actuel la fenêtre apparaît en haut à gauche pour une durée de 5s
 * et est constituée des éléments suivant :
 * 	- un fond dont la couleur correspond à l'action (ces couleurs sont définies
 * dans les données de configuration de RzxConfig)
 * 	- l'icône de la personne dont l'état à changé
 * 	- le nom de la personne concerné
 * 	- un texte décrivant le nouvel état
 */
class RzxTrayWindow: public QFrame
{
	Q_OBJECT
	
	QTimer timer;
	
	public:
		RzxTrayWindow(RzxComputer *computer, unsigned int time = 5);
		~RzxTrayWindow();
};

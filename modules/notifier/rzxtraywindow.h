/***************************************************************************
                               rzxtraywindow.h
         Gestion des fen�tres popups de notification de la trayicon
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

///Gestion de la fen�tre qui s'affiche lorsque les favoris changent d'�tat
/** Construit, affiche et d�truit la fen�tre.
 *
 * Dans l'�tat actuel la fen�tre appara�t en haut � gauche pour une dur�e de 5s
 * et est constitu�e des �l�ments suivant :
 * 	- un fond dont la couleur correspond � l'action (ces couleurs sont d�finies
 * dans les donn�es de configuration de RzxConfig)
 * 	- l'ic�ne de la personne dont l'�tat � chang�
 * 	- le nom de la personne concern�
 * 	- un texte d�crivant le nouvel �tat
 */
class RzxTrayWindow: public QFrame
{
	Q_OBJECT
	
	QTimer timer;
	
	public:
		RzxTrayWindow(RzxComputer *computer, unsigned int time = 5);
		~RzxTrayWindow();
};

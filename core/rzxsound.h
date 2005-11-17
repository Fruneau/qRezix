/***************************************************************************
                          rzxsound  -  description
                             -------------------
    begin                : Sat Nov 5 2005
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
#ifndef RZXSOUND_H
#define RZXSOUND_H

/**
 @author Florent Bruneau
 */

///Lecture des sons
/** Ce namespace est uniquement conçu pour simplifier la lecture des sons sous qRezix
 * étant donnée en effet que plusieurs paramètres entrent en jeux :
 * 	- OS
 * 	- présence d'un son
 * 	- définition d'un lecteur
 */
namespace RzxSound
{
	RZX_CORE_EXPORT void play(const QString& = QString());
};

#endif

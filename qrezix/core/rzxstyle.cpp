/***************************************************************************
                          rzxstyle  -  description
                             -------------------
    begin                : Sun Nov 6 2005
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
#include <RzxConfig>

#include <RzxStyle>

RZX_GLOBAL_INIT(RzxStyle)


///Ajoute la fenêtre à la liste des fenêtres qui utilisent le style de qRezix
/** Les fenêtres ajoutées sont automatiquement stylisée. Mais pour qu'une fenêtre
 * soit valide, il faut que ce soit une window
 */
void RzxStyle::useStyleOnWindow(QWidget *window)
{
	if(window && window->isWindow())
	{
		global()->styledWidgets << window;
		global()->applyStyle(window);
	}
}

///Applique le style courant sur une fenêtre
void RzxStyle::applyStyle(QWidget *widget)
{
	if(widget)
		widget->setAttribute(Qt::WA_MacMetalStyle, macMetalStyle());
}

///Applique le style sur toutes les fenêtres enregistrées
void RzxStyle::applyStyle()
{
	styledWidgets.removeAll(NULL);
	foreach(QWidget *widget, styledWidgets)
		applyStyle(widget);
}

///Lit le style à utiliser
bool RzxStyle::macMetalStyle()
{
	return RzxConfig::macMetalStyle();
}

///Défini le style à utiliser
void RzxStyle::setMacMetalStyle(bool style)
{
	bool isChanged = style ^ macMetalStyle();
	if(isChanged)
	{
		RzxConfig::setMacMetalStyle(style);
		global()->applyStyle();
		emit global()->useMacMetalStyle(style);
	}
}

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
#include <QStyleFactory>
#include <QStyle>
#include <QApplication>

#include <RzxConfig>

#include <RzxStyle>

RZX_GLOBAL_INIT(RzxStyle)

///Initialisation du thème
RzxStyle::RzxStyle()
{
	Rzx::beginModuleLoading("Style");

	object = this;
	current = NULL;

	styles << "default";
#ifdef Q_OS_MAC
	styles << "Mac Metal";
#endif
	styles += QStyleFactory::keys();
	qDebug() << "Found" << styles.size() << "styles";

	setStyle(RzxConfig::style());

	Rzx::endModuleLoading("Style");
}

///Fermeture bien méritée...
RzxStyle::~RzxStyle()
{
	styles.clear();
}

///Retourne la liste des styles disponibles
QStringList RzxStyle::styleList()
{
	return global()->styles;
}

///Retourne le style actuel
QString RzxStyle::style()
{
	return global()->currentName;
}

///Défini le style à utiliser
void RzxStyle::setStyle(const QString& newStyle)
{
	if(global()->styles.contains(newStyle) && newStyle != global()->currentName)
	{
		global()->currentName = newStyle;
		RzxConfig::setStyle(newStyle);
		global()->applyStyle();
		emit global()->styleChanged(newStyle);
		qDebug() << "Style set to" << newStyle;
	}
}

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
	if(!widget) return;

	widget->setStyle(current);

	if(!current)
		widget->setAttribute(Qt::WA_MacMetalStyle, currentName == "Mac Metal");
}

///Applique le style sur toutes les fenêtres enregistrées
void RzxStyle::applyStyle()
{
	if(currentName == "default" || currentName == "Mac Metal")
		current = NULL;
	else
		current = QStyleFactory::create(currentName);

	if(RzxConfig::useStyleForAll())
		QApplication::setStyle(current);
	
	if(!RzxConfig::useStyleForAll() || currentName == "Mac Metal")
	{
		styledWidgets.removeAll(NULL);
		foreach(QWidget *widget, styledWidgets)
			applyStyle(widget);
	}
}

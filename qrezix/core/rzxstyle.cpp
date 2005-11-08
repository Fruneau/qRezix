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
#include <QStyle>
#include <QApplication>

#include <RzxConfig>

#include <RzxStyle>

RZX_GLOBAL_INIT(RzxStyle)

///Initialisation du th�me
RzxStyle::RzxStyle()
{
	Rzx::beginModuleLoading("Style");

	object = this;

	styles << "default";
#ifdef Q_OS_MAC
	styles << "Mac Metal";
#endif
	styles += QStyleFactory::keys();
	qDebug() << "Found" << styles.size() << "styles";

	setStyle(RzxConfig::style());

	Rzx::endModuleLoading("Style");
}

///Fermeture bien m�rit�e...
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

///D�fini le style � utiliser
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

///Ajoute la fen�tre � la liste des fen�tres qui utilisent le style de qRezix
/** Les fen�tres ajout�es sont automatiquement stylis�e. Mais pour qu'une fen�tre
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

///Retire la fen�tre de la liste des fen�tres skinn�es
/** Cette fonction ne rend pas � la fen�tre son style d'origine mais permet juste
 * de s'assurer que la fen�tre ne sera plus concern�e par les changements de styles
 * non globaux.
 */
void RzxStyle::freeStyleOnWindow(QWidget *window)
{
	global()->styledWidgets.removeAll(window);
}

///Applique le style courant sur une fen�tre
void RzxStyle::applyStyle(QWidget *widget)
{
	if(!widget) return;
	QStyle *style = current();

	widget->setStyle(style);
	if(!style && widget->isWindow())
		widget->setAttribute(Qt::WA_MacMetalStyle, currentName == "Mac Metal");


	QList<QWidget*> children = widget->findChildren<QWidget*>();
	foreach(QWidget *child, children)
		applyStyle(child);
}

///Applique le style sur toutes les fen�tres enregistr�es
void RzxStyle::applyStyle()
{
	if(RzxConfig::useStyleForAll())
		QApplication::setStyle(current());
	
	if(!RzxConfig::useStyleForAll() || currentName == "Mac Metal")
	{
		styledWidgets.removeAll(NULL);
		foreach(QWidget *widget, styledWidgets)
			applyStyle(widget);
	}
}

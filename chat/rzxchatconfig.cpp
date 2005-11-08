/***************************************************************************
                       rzxchatconfig  -  description
                             -------------------
    begin                : Sat Nov 5 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include <QFontDatabase>

#include "rzxchatconfig.h"

RzxChatConfig::FontProperty::FontProperty(bool b, bool i, const QList<int> &pS)
	: bold(b), italic(i), sizes(pS)
{ }

RzxChatConfig::FontProperty::~FontProperty()
{ }

///Chargement de la liste des polices de caractères
void RzxChatConfig::loadFontList()
{
	Rzx::beginModuleLoading("Fonts");
	QFontDatabase fdb;
	fontFamilies = fdb.families();
	for( QStringList::Iterator f = fontFamilies.begin(); f != fontFamilies.end();)
	{
		QString family = *f;
		QStringList styles = fdb.styles( family );
		if(styles.contains("Normal")) 
		{
			QList<int> size = fdb.smoothSizes(family, "Normal");
			bool b = styles.contains("Bold")!=0;
			bool i = styles.contains("Italic")!=0 || styles.contains("Oblique")!=0;
			FontProperty fp( b, i, size);
			fontProperties.insert(family, fp);
			++f;
		}
		else
			f = fontFamilies.erase(f);
	}
	qDebug("Found %d fonts families", fontFamilies.count());
	Rzx::endModuleLoading("Fonts");
}

/// Renvoie la liste des familles de fonte initialisée au début
QStringList RzxChatConfig::getFontList()
{
	return global()->fontFamilies;
}

/// Renvoie la liste des tailles acceptées par cette police
const QList<int> RzxChatConfig::getSizes(const QString& family)
{
	const FontProperty &fp = global()->fontProperties[family];
	return fp.sizes;
}

///Indique si la police supporte l'italique
bool RzxChatConfig::isItalicSupported(const QString& family)
{
	const FontProperty &fp = global()->fontProperties[family];
	return fp.italic;
}

///Indique si la police supporte le gras
bool RzxChatConfig::isBoldSupported(const QString& family)
{
	const FontProperty &fp = global()->fontProperties[family];
	return fp.bold;
}

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
#include <RzxTranslator>
#include "rzxchatconfig.h"

/// Initialisation
void RzxChatConfig::init()
{
	loadFontList();
	loadSmileysList();
	RzxTranslator::connect(this, SLOT(loadSmileysList()));
}

/// Fermeture
void RzxChatConfig::destroy()
{
}

///Chargement de la liste des thèmes
void RzxChatConfig::loadSmileysList()
{
	smileyDir.clear();
	smileyDir.insert(tr("none"),0);
	
	//Recherche des thèmes de smileys installés
	qDebug("Searching smileys themes...");
	QList<QDir> path = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "smileys");

	foreach(QDir dir, path)
	{
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(QString subDir, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc à rajouter des clés parasites dans la liste
			if(!smileyDir.keys().contains(subDir))
			{
				QDir *theme = new QDir(dir);
				theme->cd(subDir);
				if(theme->exists("theme"))
				{
					qDebug() << "*" << subDir << "in" << theme->path();
					smileyDir.insert(subDir, theme);
				}
				else
					delete theme;
			}
		}
	}
	loadSmileys();
}

/// Chargement des correspondances motif/smiley
void RzxChatConfig::loadSmileys()
{
	global()->smileys.clear();
	// chargement de la config
	QDir *dir = global()->smileyDir[RzxChatConfig::smileyTheme()];
	if (dir)
	{
		QString text;
		QFile file(dir->absolutePath()+"/theme");
		if(file.exists())
		{
			file.open(QIODevice::ReadOnly);
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			while(!stream.atEnd())
			{
				text = stream.readLine();
				QStringList list = text.split("###");
				if(list.count() == 2)
				{
					QStringList rep = list[0].split("$$");
					for (int i = 0; i < rep.size(); ++i)
					{
						QStringList item;
						item << rep[i].trimmed();
						item << list[1];
						global()->smileys << item;
					}
				}
			}
			file.close();
		}
	}
}

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

///Retourne la police dont le nom semble le plus proche
/** En espérant que ce soit une police du même type que celui
 * qu'on recherche
 */
QString RzxChatConfig::nearestFont(const QString& family)
{
	QStringList keys = global()->fontProperties.keys();
	if(keys.contains(family))
		return family;

	foreach(QString key, keys)
	{
		if(key.contains(family, Qt::CaseInsensitive))
			return key;
	}

	foreach(QString key, keys)
	{
		if(family.contains(key, Qt::CaseInsensitive))
			return key;
	}
	return keys[0];
}

///Retourne la taille la plus proche de celle indiquée pour la police donnée
/** Si la police n'est pas valide, la fonction retourne -1
 */
int RzxChatConfig::nearestSize(const QString& family, int size)
{
	if(!global()->fontProperties.keys().contains(family))
		return -1;

	QList<int> sizes = global()->fontProperties[family].sizes;
	if(sizes.isEmpty())
		return -1;

	if(sizes.contains(size))
		return size;

	//128 maximum... pour éviter les ennuis
	//mais normalement on peut monter à l'infini, ça changerait rien
	//la fonction devrait toujours finir
	for(int i = 1 ; i < 128 ; i++)
	{
		if(i < size && sizes.contains(size - i))
			return size - i;
		if(sizes.contains(size + i))
			return size + i;
	}
	return -1;
}

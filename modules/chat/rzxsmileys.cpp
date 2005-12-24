/***************************************************************************
                          rzxsmiley -  description
                             -------------------
    begin                : Sat Dec 24 2005
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
#include <QDir>

#include "rzxsmileys.h"

#include "rzxchatconfig.h"

RZX_GLOBAL_INIT(RzxSmileys)

///Construction : chargement de base
RzxSmileys::RzxSmileys()
{
	object = this;
	loadSmileysList();
	setTheme(theme());
}

///Destruction
RzxSmileys::~RzxSmileys()
{
	RZX_GLOBAL_CLOSE
}

///Change le th�me
void RzxSmileys::setTheme(const QString& theme)
{
	RzxChatConfig::setSmileyTheme(theme);
	global()->loadSmileys();
}

///Retourne le th�me actuel
QString RzxSmileys::theme()
{
	return RzxChatConfig::smileyTheme();
}

///Chargement de la liste des th�mes
void RzxSmileys::loadSmileysList()
{
	smileyDir.clear();
	
	//Recherche des th�mes de smileys install�s
	qDebug("Searching smileys themes...");
	QList<QDir> path = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "smileys");

	foreach(QDir dir, path)
	{
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(QString subDir, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc � rajouter des cl�s parasites dans la liste
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
void RzxSmileys::loadSmileys()
{
	smileys.clear();
	baseSmileys.clear();
	// chargement de la config
	QDir *dir = smileyDir[theme()];
	if(dir)
	{
		QString text;
		QFile file(dir->absoluteFilePath("theme"));
		if(file.exists())
		{
			file.open(QIODevice::ReadOnly);
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			while(!stream.atEnd())
			{
				text = stream.readLine();
				QStringList list = text.split("###");
				if(list.count() == 3 && QFile::exists(dir->absoluteFilePath(list[2])))
				{
					baseSmileys << list[0];
					smileys.insert(list[1], dir->absoluteFilePath(list[2]));
				}
			}
			file.close();
		}
	}
}

/// Retourne la liste des th�mes de smiley
QStringList RzxSmileys::themeList()
{
	QStringList list;
	list << tr("None");
	list += global()->smileyDir.keys();
	return list;
}

/// Retourne la liste des smileys de d�monstration
QStringList RzxSmileys::baseSmileyList()
{
	return global()->baseSmileys;
}

/// Retourne le smiley associ� au texte indiqu�
void RzxSmileys::replace(QString & msg)
{
	foreach(QString text, global()->smileys.keys())
	{
		QRegExp mask(text);
		msg.replace(mask,  "<img src=\"" + global()->smileys[text] + "\" alt=\"\\1\">");
	}
}

///Retourne le chemin associ� au smiley donn�
QString RzxSmileys::smiley(const QString& sml)
{
	foreach(QString text, global()->smileys.keys())
	{
		QRegExp mask(text);
		if(mask.indexIn(sml) != -1)
			return global()->smileys[text];
	}
	return QString();
}

///Idem que pr�c�dent mais retourne le pixmap associ�
QPixmap RzxSmileys::pixmap(const QString& sml)
{
	return QPixmap(smiley(sml));
}

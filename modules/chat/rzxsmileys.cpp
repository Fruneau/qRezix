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
	Rzx::beginModuleLoading("Smileys");
	object = this;
	currentTheme = NULL;
	loadSmileysList();
	setTheme(theme());
	Rzx::endModuleLoading("Smileys");
}

///Destruction
RzxSmileys::~RzxSmileys()
{
	qDeleteAll(themes);
	RZX_GLOBAL_CLOSE
}

///Change le thème
void RzxSmileys::setTheme(const QString& theme)
{
	RzxChatConfig::setSmileyTheme(theme);
	global()->currentTheme = global()->smileyTheme(theme);
	emit global()->themeChanged(theme);
}

///Retourne le thème actuel
QString RzxSmileys::theme()
{
	QString thm = RzxChatConfig::smileyTheme();
	if(thm.isNull())
		thm = themeList()[0];
	return thm;
}

///Indique si le thème est valide
bool RzxSmileys::isValid(const QString& thm)
{
	SmileyTheme *theme = global()->smileyTheme(thm);
	if(!theme)
		return false;

	return theme->baseSmileys.size();
}

///Chargement de la liste des thèmes
void RzxSmileys::loadSmileysList()
{
	themes.clear();
	
	//Recherche des thèmes de smileys installés
	qDebug("Searching smileys themes...");
	QList<QDir> path = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "smileys");

	foreach(const QDir &dir, path)
	{
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(const QString &theme, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc à rajouter des clés parasites dans la liste
			if(!themes.keys().contains(theme))
			{
				QDir *subDir = new QDir(dir);
				subDir->cd(theme);
				if(subDir->exists("theme"))
				{
					qDebug() << "*" << theme << "in" << subDir->path();
					SmileyTheme *sml = new SmileyTheme;
					loadSmileys(sml, subDir);
					themes.insert(theme, sml);
				}
				delete subDir;
			}
		}
	}
}

/// Chargement des correspondances motif/smiley
void RzxSmileys::loadSmileys(SmileyTheme *thm, QDir *dir)
{
	if(!thm || !dir) return;

	thm->smileys.clear();
	thm->baseSmileys.clear();

	// chargement de la config
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
				if(list.count() == 3 && dir->exists(list[2]))
				{
					thm->baseSmileys << list[0];
					thm->smileys.insert(list[1], dir->absoluteFilePath(list[2]));
				}
			}
			file.close();
		}
	}
}

/// Retourne la liste des thèmes de smiley
QStringList RzxSmileys::themeList()
{
	QStringList list;
	list << tr("None");
	list += global()->themes.keys();
	return list;
}

/// Retourne la liste des smileys de démonstration
QStringList RzxSmileys::baseSmileyList(const QString& thm)
{
	SmileyTheme *theme = global()->smileyTheme(thm);
	if(!theme)
		return QStringList();

	return theme->baseSmileys;
}

/// Retourne le smiley associé au texte indiqué
void RzxSmileys::replace(QString & msg, const QString& thm)
{
	SmileyTheme *theme = global()->smileyTheme(thm);
	if(!theme)
		return;

	foreach(const QString &text, theme->smileys.keys())
	{
		QRegExp mask("(" + text + ")");
		msg.replace(mask,  "<img src=\"" + theme->smileys[text] + "\" alt=\"\\1\">");
	}
}

///Retourne le chemin associé au smiley donné
QString RzxSmileys::smiley(const QString& sml, const QString& thm)
{
	SmileyTheme *theme = global()->smileyTheme(thm);
	if(!theme)
		return QString();

	foreach(const QString &text, theme->smileys.keys())
	{
		QRegExp mask(text);
		if(mask.indexIn(sml) != -1)
			return theme->smileys[text];
	}
	return QString();
}

///Idem que précédent mais retourne le pixmap associé
QPixmap RzxSmileys::pixmap(const QString& sml, const QString& thm)
{
	return QPixmap(smiley(sml, thm));
}

///Donne un aperçu du theme
QList<QPixmap> RzxSmileys::preview(const QString& thm)
{
	QList<QPixmap> ret;
	QStringList base = baseSmileyList(thm);
	foreach(const QString &sml, base)
		ret << pixmap(sml, thm);
	return ret;
}

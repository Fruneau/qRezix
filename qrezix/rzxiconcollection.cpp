/***************************************************************************
                          rzxiconcollection  -  description
                             -------------------
    begin                : Sun Jul 24 2005
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
#include <QList>

#include "rzxiconcollection.h"

#include "rzxconfig.h"

#include QREZIX_ICON
#include QREZIX_AWAY_ICON

///Données d'information des icônes
/** Chaque icône est représentée par un IconData qui contient l'identifiant,
 * le nom du fichier correspondant et un flag indiquant si l'icône est ou non
 * une icône système. Une icône système étant une icône sans laquelle l'interface
 * perdrait beaucoup de son intérêt...
 */
const RzxIconCollection::IconData RzxIconCollection::data[] = {
	{ Rzx::ICON_ACTION, "action", true },
	{ Rzx::ICON_APPLY, "apply", false },
	{ Rzx::ICON_OK, "ok", false },
	{ Rzx::ICON_CANCEL, "cancel", false },
	{ Rzx::ICON_AWAY, "away", true },
	{ Rzx::ICON_HERE, "here", true },
	{ Rzx::ICON_CHAT, "chat", false },
	{ Rzx::ICON_FTP , "ftp", true },
	{ Rzx::ICON_HTTP, "http", true },
	{ Rzx::ICON_SAMBA, "samba", true },
	{ Rzx::ICON_NEWS, "news", true },
	{ Rzx::ICON_HOTLINE, "hotline", false },
	{ Rzx::ICON_NOFTP, "no_ftp", false },
	{ Rzx::ICON_NOHTTP, "no_http", false },
	{ Rzx::ICON_NOSAMBA, "no_samba", false },
	{ Rzx::ICON_NONEWS, "no_news", false },
	{ Rzx::ICON_NOHOTLINE, "no_hotline", false },
	{ Rzx::ICON_BAN, "ban", false },
	{ Rzx::ICON_UNBAN, "unban", false },
	{ Rzx::ICON_FAVORITE, "favorite", true },
	{ Rzx::ICON_NOTFAVORITE, "not_favorite", true },
	{ Rzx::ICON_SAMEGATEWAY, "same_gateway", true },
	{ Rzx::ICON_OTHERGATEWAY, "diff_gateway", false },
	{ Rzx::ICON_COLUMN, "column", true },
	{ Rzx::ICON_SEARCH, "search", true },
	{ Rzx::ICON_PREFERENCES, "pref", true },
	{ Rzx::ICON_PLUGIN, "plugin", true },
	{ Rzx::ICON_SOUNDON, "haut_parleur1", true },
	{ Rzx::ICON_SOUNDOFF, "haut_parleur2", true },
	{ Rzx::ICON_PROPRIETES, "prop", true },
	{ Rzx::ICON_HISTORIQUE, "historique", true },
	{ Rzx::ICON_JONE, "jone", true },
	{ Rzx::ICON_ROUJE, "rouje", true },
	{ Rzx::ICON_ORANJE, "orange", true },
	{ Rzx::ICON_QUIT, "quit", false },
	{ Rzx::ICON_ON, "on", false },
	{ Rzx::ICON_OFF, "off", false },
	{ Rzx::ICON_SEND, "send", true },
	{ Rzx::ICON_SYSTRAYHERE, "systray", false },
	{ Rzx::ICON_SYSTRAYAWAY, "systrayAway", false },
	{ Rzx::ICON_LAYOUT, "layout", true },
	{ Rzx::ICON_NETWORK, "network", true },
	{ Rzx::ICON_OS0, "os_0", true },
	{ Rzx::ICON_OS0_LARGE, "os_0_large", true },
	{ Rzx::ICON_OS1, "os_1", true },
	{ Rzx::ICON_OS1_LARGE, "os_1_large", true },
	{ Rzx::ICON_OS2 , "os_2", true },
	{ Rzx::ICON_OS2_LARGE, "os_2_large", true},
	{ Rzx::ICON_OS3, "os_3", true },
	{ Rzx::ICON_OS3_LARGE, "os_3_large", true },
	{ Rzx::ICON_OS4, "os_4", true },
	{ Rzx::ICON_OS4_LARGE, "os_4_large", true },
	{ Rzx::ICON_OS5, "os_5", true },
	{ Rzx::ICON_OS5_LARGE, "os_5_large", true },
	{ Rzx::ICON_OS6, "os_6", true },
	{ Rzx::ICON_OS6_LARGE, "os_6_large", true }
};

RzxIconCollection *RzxIconCollection::object = NULL;

///Construction de la collection d'icône
RzxIconCollection::RzxIconCollection()
{
	Rzx::beginModuleLoading("Icon Collection");
	//Définition de l'icône de l'application
	hereIcon = QPixmap(q);
	awayIcon = QPixmap(t);

	//Recherche des thèmes installés
	qDebug("Searching icon themes...");
	QList<QDir> path;
	path << RzxConfig::userDir() << RzxConfig::systemDir() << RzxConfig::libDir();

	foreach(QDir dir, path)
	{
		dir.cd("themes");
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(QString subDir, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc à rajouter des clés parasites dans la liste
			if(!themeDir.keys().contains(subDir))
			{
				QDir *theme = new QDir(dir);
				theme->cd(subDir);
				if(isValid(*theme))
				{
					qDebug("* %s in %s", subDir.toAscii().constData(), theme->absolutePath().toAscii().constData());
					themeDir.insert(subDir, theme);
				}
			}
		}
	}

	//Chargement du thème de démarrage
	qDebug("Loading theme...");
	activeTheme = QString();
	setTheme(RzxConfig::iconTheme());
	Rzx::endModuleLoading("Icon Collection");
}

///Destruction...
RzxIconCollection::~RzxIconCollection()
{
	qDeleteAll(themeDir);
	themeDir.clear();
	icons.clear();
	userIcons.clear();
}

///Teste si le répertoire contient un thème valide
/** Un thème valide est un thème qui contient toutes les icônes qui sont référencées comme
 * needed dans data
 */
bool RzxIconCollection::isValid(const QDir& dir) const
{
	for(int i = 0 ; i < Rzx::ICON_NUMBER ; i++)
		if(data[i].needed && !dir.exists(QString(data[i].filename) + ".png"))
			return false;
	return true;
}

///Change le thème actif
/** Si le thème demandé n'existe pas, on ne change pas de thème */
void RzxIconCollection::setTheme(const QString& theme)
{
	if(theme == activeTheme) return;
	if(!themeDir.keys().contains(theme))
	{
		if(!activeTheme.isNull()) return;
		if(themeDir.keys().contains(DEFAULT_THEME))
			activeTheme = DEFAULT_THEME;
		else if(themeDir.count())
			activeTheme = themeDir.keys()[0];
	}
	else
		activeTheme = theme;
	icons.clear();
	qDebug("Icons theme set to %s", activeTheme.toAscii().constData());
	emit themeChanged(activeTheme);
}

///Retourne le thème actuel
const QString &RzxIconCollection::theme() const
{
	return activeTheme;
}

///Retourne la liste des thèmes
QStringList RzxIconCollection::themeList() const
{
	QStringList list = themeDir.keys();
	qSort(list.begin(), list.end(), Rzx::caseInsensitiveLessThan);
	return list;
}

///Chargement d'une icône située à un endroit donné
QPixmap RzxIconCollection::loadIcon(const QString& name, const QString& theme) const
{
	if(!themeDir.keys().contains(theme)) return QPixmap();
	QDir *dir = themeDir[theme];

	if(dir->exists(name + ".png"))
		return QPixmap(dir->absoluteFilePath(name + ".png"), "PNG");
	return QPixmap();
}

///Récupération d'une icône
const QPixmap &RzxIconCollection::pixmap(Rzx::Icon icon)
{
	QString name;
	if(icon < Rzx::ICON_NUMBER)
	{
		name = data[icon].filename;
		if(icons[name].isNull())
			icons.insert(name, loadIcon(name, activeTheme));
	}
	return icons[name];
}

///Récupération d'une icône
QIcon RzxIconCollection::icon(Rzx::Icon icon)
{
	return QIcon(pixmap(icon));
}

///Récupération des icônes des OS
const QPixmap &RzxIconCollection::osPixmap(Rzx::SysEx sysex, bool large)
{
	QString name = "os_" + QString::number(sysex);
	if(large)
		name += "_large";
	if(icons[name].isNull())
		icons.insert(name, loadIcon(name, activeTheme));
	return icons[name];
}

///Récupération de l'icône de l'OS
QIcon RzxIconCollection::osIcon(Rzx::SysEx sysex)
{
	QIcon icon;
	icon.addPixmap(osPixmap(sysex, false));
	icon.addPixmap(osPixmap(sysex, true));
	return icon;
}

///Récupération des icônes de promo
const QPixmap &RzxIconCollection::promoPixmap(Rzx::Promal promo)
{
	switch(promo)
	{
		case Rzx::PROMAL_JONE: return pixmap(Rzx::ICON_JONE);
		case Rzx::PROMAL_ROUJE: return pixmap(Rzx::ICON_ROUJE);
		default: return pixmap(Rzx::ICON_ORANJE);
	}
}

///Récupération de l'icône de la promo
QIcon RzxIconCollection::promoIcon(Rzx::Promal promo)
{
	return promoPixmap(promo);
}

///Récupération de l'icône du répondeur
QIcon RzxIconCollection::responderIcon()
{
	QIcon responder;
	responder.addPixmap(pixmap(Rzx::ICON_AWAY), QIcon::Normal, QIcon::On);
	responder.addPixmap(pixmap(Rzx::ICON_HERE), QIcon::Normal, QIcon::Off);
	return responder;
}

///Récupération de l'icône du son
QIcon RzxIconCollection::soundIcon()
{
	QIcon sound;
	sound.addPixmap(pixmap(Rzx::ICON_SOUNDON), QIcon::Normal, QIcon::On);
	sound.addPixmap(pixmap(Rzx::ICON_SOUNDOFF), QIcon::Normal, QIcon::Off);
	return sound;
}

///Récupération de l'icône on/off
QIcon RzxIconCollection::onOffIcon()
{
	QIcon onoff;
	onoff.addPixmap(pixmap(Rzx::ICON_ON), QIcon::Normal, QIcon::On);
	onoff.addPixmap(pixmap(Rzx::ICON_OFF), QIcon::Normal, QIcon::Off);
	return onoff;
}

///Récupération de l'icône favorit/non favorit
QIcon RzxIconCollection::favoriteIcon()
{
	QIcon favorite;
	favorite.addPixmap(pixmap(Rzx::ICON_FAVORITE), QIcon::Normal, QIcon::On);
	favorite.addPixmap(pixmap(Rzx::ICON_NOTFAVORITE), QIcon::Normal, QIcon::Off);
	return favorite;
}

///Récupération de l'icône ban/unban
QIcon RzxIconCollection::banIcon()
{
	QIcon ban;
	ban.addPixmap(pixmap(Rzx::ICON_BAN), QIcon::Normal, QIcon::On);
	ban.addPixmap(pixmap(Rzx::ICON_UNBAN), QIcon::Normal, QIcon::Off);
	return ban;
}

///Récupération d'une icône utilisateur
/** Les icônes utilisateurs sont hashées par le serveur et stockées
 * brutes sous un nom de fichier de la forme hash.img
 */
const QPixmap &RzxIconCollection::hashedIcon(quint32 hash)
{
	QString filename = QString::number(hash, 16) + ".png";
	if(hash)
		userIcons.insert(hash, QPixmap(RzxConfig::computerIconsDir().absoluteFilePath(filename)));
	return userIcons[hash];
}

///Enregistrement d'une icône utilisateur
const QPixmap &RzxIconCollection::setHashedIcon(quint32 hash, const QImage& image)
{
	QString filename = QString::number(hash, 16) + ".png";
	image.save(RzxConfig::computerIconsDir().absoluteFilePath(filename), "PNG");
	userIcons.insert(hash, QPixmap::fromImage(image));
	return userIcons[hash];
}

///Récupération de localhost
QPixmap RzxIconCollection::localhostPixmap()
{
	return QPixmap(RzxConfig::userDir().absoluteFilePath("localhost.png"), "PNG");
}

///Sauvegarde le l'icône de localhost
void RzxIconCollection::setLocalhostPixmap(const QPixmap& icon)
{
	icon.save(RzxConfig::userDir().absoluteFilePath("localhost.png"), "PNG");
}

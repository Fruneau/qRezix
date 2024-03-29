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
#include <QBitmap>

#include <RzxIconCollection>

#include <RzxConfig>

RZX_GLOBAL_INIT(RzxIconCollection)

///Donn�es d'information des ic�nes
/** Chaque ic�ne est repr�sent�e par un IconData qui contient l'identifiant,
 * le nom du fichier correspondant et un flag indiquant si l'ic�ne est ou non
 * une ic�ne syst�me. Une ic�ne syst�me �tant une ic�ne sans laquelle l'interface
 * perdrait beaucoup de son int�r�t...
 */
const RzxIconCollection::IconData RzxIconCollection::data[] = {
	{ Rzx::ICON_ACTION, "action", false },
	{ Rzx::ICON_APPLY, "apply", false },
	{ Rzx::ICON_OK, "ok", false },
	{ Rzx::ICON_CANCEL, "cancel", false },
	{ Rzx::ICON_AWAY, "away", true },
	{ Rzx::ICON_HERE, "here", true },
	{ Rzx::ICON_CHAT, "chat", true },
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
	{ Rzx::ICON_BAN, "ban", true },
	{ Rzx::ICON_UNBAN, "unban", true },
	{ Rzx::ICON_FAVORITE, "favorite", true },
	{ Rzx::ICON_NOTFAVORITE, "not_favorite", true },
	{ Rzx::ICON_SAMEGATEWAY, "same_gateway", true },
	{ Rzx::ICON_OTHERGATEWAY, "diff_gateway", false },
	{ Rzx::ICON_COLUMN, "column", false },
	{ Rzx::ICON_SEARCH, "search", false },
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
#ifndef Q_OS_WIN
	{ Rzx::ICON_SYSTRAYHERE, "systray", false },
	{ Rzx::ICON_SYSTRAYAWAY, "systrayAway", false },
#else
	{ Rzx::ICON_SYSTRAYHERE, "systray_win", false },
	{ Rzx::ICON_SYSTRAYAWAY, "systrayAway_win", false },
#endif
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
	{ Rzx::ICON_OS6_LARGE, "os_6_large", true },
	{ Rzx::ICON_PRINTER, "printer", true },
	{ Rzx::ICON_NOPRINTER, "no_printer", false },
	{ Rzx::ICON_LOAD, "load", false },
	{ Rzx::ICON_UNLOAD, "unload", false },
	{ Rzx::ICON_RELOAD, "reload", false },
	{ Rzx::ICON_MAIL, "mail", true },
	{ Rzx::ICON_PHONE, "phone", false },
	{ Rzx::ICON_FILE, "file", true },
#ifndef Q_OS_WIN
	{ Rzx::ICON_SYSTRAYDISCON, "systrayDiscon", false },
#else
	{ Rzx::ICON_SYSTRAYDISCON, "systrayDiscon_win", false },
#endif
	{ Rzx::ICON_FILETRANSFER, "file_transfer", false},
	{ Rzx::ICON_REFUSE, "refuse", false}
};

///Construction de la collection d'ic�ne
RzxIconCollection::RzxIconCollection()
{
	Rzx::beginModuleLoading("Icon Collection");
	//D�finition de l'ic�ne de l'application
	qIcon = QPixmap(":/qrezix_icon.png");

	//Recherche des th�mes install�s
	qDebug("Searching icon themes...");
	QList<QDir> path = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "themes");

	foreach(const QDir &dir, path)
	{
		QStringList subDirs = dir.entryList(QDir::Dirs, QDir::Name | QDir::IgnoreCase);
		foreach(const QString &subDir, subDirs)
		{
			//on utilise .keys().contain() car value[] fait un insert dans le QHash
			//ce qui tendrait donc � rajouter des cl�s parasites dans la liste
			if(!themeDir.keys().contains(subDir))
			{
				QDir *theme = new QDir(dir);
				theme->cd(subDir);
				if(isValid(*theme) || subDir == "none")
				{
					qDebug() << "*" << subDir << "in" << theme->path();
					themeDir.insert(subDir, theme);
				}
				else
					delete theme;
			}
		}
	}

	//Chargement du th�me de d�marrage
	qDebug("Loading theme...");
	activeTheme = QString();
	local_setTheme(RzxConfig::iconTheme());
	Rzx::endModuleLoading("Icon Collection");
}

///Destruction...
RzxIconCollection::~RzxIconCollection()
{
	qDeleteAll(themeDir);
	themeDir.clear();
	icons.clear();
	userIcons.clear();
	RZX_GLOBAL_CLOSE
}

///Teste si le r�pertoire contient un th�me valide
/** Un th�me valide est un th�me qui contient toutes les ic�nes qui sont r�f�renc�es comme
 * needed dans data
 */
bool RzxIconCollection::isValid(const QDir& dir) const
{
	for(int i = 0 ; i < Rzx::ICON_NUMBER ; i++)
		if(data[i].needed && !dir.exists(QString(data[i].filename) + ".png"))
			return false;
	return true;
}

///Change le th�me actif
/** Si le th�me demand� n'existe pas, on ne change pas de th�me */
void RzxIconCollection::setTheme(const QString& theme)
{
	global()->local_setTheme(theme);
}

///Impl�mentation de \ref setTheme
void RzxIconCollection::local_setTheme(const QString& theme)
{
	if(theme == activeTheme || theme == "none") return;
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
	
	RzxConfig::setIconTheme(activeTheme);
	emit themeChanged(activeTheme);
}

///Retourne le th�me actuel
QString RzxIconCollection::theme()
{
	return global()->activeTheme;
}

///Retourne la liste des th�mes
QStringList RzxIconCollection::themeList()
{
	QStringList list = global()->themeDir.keys();
	list.removeAll("none");
	qSort(list.begin(), list.end(), Rzx::caseInsensitiveLessThan);
	return list;
}

///Retourne le nom du fichier associ� � l'image
QString RzxIconCollection::pixmapFileName(const QString& name, const QString& theme) const
{
	QDir *dir;
	if(theme.isNull()) dir = themeDir[activeTheme];
	else dir = themeDir[theme];
	if(dir && dir->exists(name + ".png"))
		return dir->absoluteFilePath(name + ".png");
	if(theme != "none")
		return pixmapFileName(name, "none");
	return QString();
}

///Retourne le nom du fichier associ� � l'image
QString RzxIconCollection::pixmapFileName(Rzx::Icon icon, const QString& theme) const
{
	if(icon < Rzx::ICON_NUMBER)
		return pixmapFileName(data[icon].filename, theme);
	return QString();
}

///Chargement d'une ic�ne situ�e � un endroit donn�
QPixmap RzxIconCollection::loadIcon(const QString& name, const QString& theme) const
{
	if(!themeDir.keys().contains(theme)) return QPixmap();
	const QString file = pixmapFileName(name, theme);
	if(file.isNull())
		return QPixmap();
	return QPixmap(file, "PNG");
}

///R�cup�ration d'une ic�ne
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

///R�cup�ration d'une ic�ne
/** Surcharge permettant de charger une fonction � partir d'un nom
 * de fichier plut�t que d'un identifiant.
 *
 * L'int�r�t de cette fonction est surtout pour les modules qui ont
 * des ic�nes personnalis�es
 */
const QPixmap &RzxIconCollection::pixmap(const QString& name)
{
	if(icons[name].isNull())
		icons.insert(name, loadIcon(name, activeTheme));
	return icons[name];
}

///R�cup�ration d'une ic�ne
QIcon RzxIconCollection::icon(Rzx::Icon icon)
{
	return QIcon(pixmap(icon));
}

///R�cup�ration d'une ic�ne
/** Surcharge permettant de charger une fonction � partir d'un nom
 * de fichier plut�t que d'un identifiant.
 *
 * L'int�r�t de cette fonction est surtout pour les modules qui ont
 * des ic�nes personnalis�es
 */
QIcon RzxIconCollection::icon(const QString& icon)
{
	return QIcon(pixmap(icon));
}

///Retourne le nom de l'ic�ne
QString RzxIconCollection::getIconName(Rzx::Icon icon)
{
	return data[icon].filename;
}

///R�cup�ration des ic�nes des OS
const QPixmap &RzxIconCollection::osPixmap(Rzx::SysEx sysex, bool large)
{
	return pixmap(toIcon(sysex, large));
}

///R�cup�ration de l'ic�ne de l'OS
QIcon RzxIconCollection::osIcon(Rzx::SysEx sysex)
{
	QIcon icon;
	icon.addPixmap(osPixmap(sysex, false));
	icon.addPixmap(osPixmap(sysex, true));
	return icon;
}

///R�cup�ration des ic�nes de promo
const QPixmap &RzxIconCollection::promoPixmap(Rzx::Promal promo)
{
	return pixmap(Rzx::toIcon(promo));
}

///R�cup�ration de l'ic�ne de la promo
QIcon RzxIconCollection::promoIcon(Rzx::Promal promo)
{
	return promoPixmap(promo);
}

///R�cup�ration de l'ic�ne du r�pondeur
QIcon RzxIconCollection::responderIcon()
{
	QIcon responder;
	responder.addPixmap(pixmap(Rzx::ICON_AWAY), QIcon::Normal, QIcon::On);
	responder.addPixmap(pixmap(Rzx::ICON_HERE), QIcon::Normal, QIcon::Off);
	return responder;
}

///R�cup�ration de l'ic�ne du son
QIcon RzxIconCollection::soundIcon()
{
	QIcon sound;
	sound.addPixmap(pixmap(Rzx::ICON_SOUNDON), QIcon::Normal, QIcon::On);
	sound.addPixmap(pixmap(Rzx::ICON_SOUNDOFF), QIcon::Normal, QIcon::Off);
	return sound;
}

///R�cup�ration de l'ic�ne on/off
QIcon RzxIconCollection::onOffIcon()
{
	QIcon onoff;
	onoff.addPixmap(pixmap(Rzx::ICON_ON), QIcon::Normal, QIcon::On);
	onoff.addPixmap(pixmap(Rzx::ICON_OFF), QIcon::Normal, QIcon::Off);
	return onoff;
}

///R�cup�ration de l'ic�ne favorit/non favorit
QIcon RzxIconCollection::favoriteIcon()
{
	QIcon favorite;
	favorite.addPixmap(pixmap(Rzx::ICON_FAVORITE), QIcon::Normal, QIcon::On);
	favorite.addPixmap(pixmap(Rzx::ICON_NOTFAVORITE), QIcon::Normal, QIcon::Off);
	return favorite;
}

///R�cup�ration de l'ic�ne ban/unban
QIcon RzxIconCollection::banIcon()
{
	QIcon ban;
	ban.addPixmap(pixmap(Rzx::ICON_BAN), QIcon::Normal, QIcon::On);
	ban.addPixmap(pixmap(Rzx::ICON_UNBAN), QIcon::Normal, QIcon::Off);
	return ban;
}

///Surcharge
/** \sa hereIcon
 */
const QPixmap& RzxIconCollection::qRezixIcon()
{
	return global()->qIcon;
}

///R�cup�ration d'une ic�ne utilisateur
/** Les ic�nes utilisateurs sont hash�es par le serveur et stock�es
 * brutes sous un nom de fichier de la forme hash.img
 */
const QPixmap &RzxIconCollection::hashedIcon(quint32 hash)
{
	QString filename = QString::number(hash, 16) + ".png";
	if(hash)
		userIcons.insert(hash, QPixmap(hashedIconFileName(hash, true)));
	return userIcons[hash];
}

///Enregistrement d'une ic�ne utilisateur
const QPixmap &RzxIconCollection::setHashedIcon(quint32 hash, const QImage& image)
{
	image.save(hashedIconFileName(hash, true), "PNG");
	userIcons.insert(hash, QPixmap::fromImage(image));
	return userIcons[hash];
}

///Retourne le nom de fichier pour l'ic�ne indiqu�e
/** Si force est faux, on teste l'existence du fichier avant de r�pondre
 */
QString RzxIconCollection::hashedIconFileName(quint32 hash, bool force) const
{
	QString filename = QString::number(hash, 16) + ".png";
	if(force || RzxConfig::computerIconsDir().exists(filename))
		filename = RzxConfig::computerIconsDir().absoluteFilePath(filename);
	else
		filename = "";
	return filename;
}

///R�cup�ration de localhost
QPixmap RzxIconCollection::localhostPixmap()
{
	return QPixmap(RzxConfig::userDir().absoluteFilePath("localhost.png"), "PNG");
}

///Sauvegarde le l'ic�ne de localhost
void RzxIconCollection::setLocalhostPixmap(const QPixmap& icon)
{
	icon.save(RzxConfig::userDir().absoluteFilePath("localhost.png"), "PNG");
}

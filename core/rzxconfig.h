/***************************************************************************
                          rzxconfig.h  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXCONFIG_H
#define RZXCONFIG_H

#include <QVector>
#include <QDir>
#include <QList>

#include <RzxGlobal>
#include <RzxSubnet>

#include <RzxAbstractConfig>

class QTreeWidget;
class RzxHostAddress;
class RzxComputer;

/**
  *@author Sylvain Joyeux
  */

///Gestion des données de configuration
/** Classe à épurer !!! */
class RZX_CORE_EXPORT RzxConfig : public RzxAbstractConfig
{
	Q_OBJECT
	Q_ENUMS(DirFlags)
	Q_FLAGS(Dir)
	RZX_CONFIG_EXPANDED(RzxConfig)

//Centre de la classe...
private:
	QDir m_systemDir;
	QDir m_userDir;
	QDir m_libDir;
	static const char* propStrings[][2];

public:
	// Données
	RZX_STRINGPROP("theme", iconTheme, setIconTheme, DEFAULT_THEME)
	RZX_STRINGPROP("style", style, setStyle, "Default")
	RZX_BOOLPROP("styleforall", useStyleForAll, setUseStyleForAll, true)
	RZX_STRINGPROP("language", language, setLanguage, "English")
	RZX_STRINGPROP("txtBeepCmd", beepCmd, setBeepCmd, "play")

	RZX_STRINGPROP("FTPPath", ftpPath, setFtpPath, QString())
	RZX_STRINGPROP("samba_cmd", sambaCmd, setSambaCmd, DEFAULT_SAMBACMD)
	RZX_STRINGPROP("ftp_cmd", ftpCmd, setFtpCmd, DEFAULT_FTPCMD)
	RZX_STRINGPROP("http_cmd", httpCmd, setHttpCmd, DEFAULT_HTTPCMD)
	RZX_STRINGPROP("news_cmd", newsCmd, setNewsCmd, DEFAULT_NEWSCMD)
	RZX_STRINGPROP("mail_cmd", mailCmd, setMailCmd, DEFAULT_MAILCMD)

	// Configuration de base de l'ordinateur
	RZX_STRINGPROP("dnsname", dnsName, setDnsName, QString())
	RZX_ENUMPROP(Rzx::Promal, "promo", promo, setPromo, Rzx::PROMAL_UNK)
	RZX_ENUMPROP(Rzx::ConnectionState, "repondeur", repondeur, setRepondeur, Rzx::STATE_HERE)
	RZX_INTPROP("servers", servers, setServers, 0)
	RZX_PROP_DECLARE(QString, remarque, setRemarque, QString())

	// proprietes de l'ordinateur
	RZX_STRINGPROP("txtFirstname", propLastName, setPropLastName, QString())
	RZX_STRINGPROP("txtName", propName, setPropName, QString())
	RZX_STRINGPROP("txtSurname", propSurname, setPropSurname, QString())
	RZX_STRINGPROP("txtCasert", propCasert, setPropCasert, QString())
	RZX_STRINGPROP("txtMail", propMail, setPropMail, DEFAULT_MAIL)
	RZX_STRINGPROP("txtWeb", propWebPage, setPropWebPage, QString())
	RZX_STRINGPROP("txtPhone", propTel, setPropTel, QString())
	RZX_STRINGPROP("txtMobile", propMobile, setPropMobile, QString())
	RZX_STRINGPROP("txtSport", propSport, setPropSport, QString())
	RZX_UINTPROP("numSport", numSport, setNumSport, 0)
	static QString propPromo();

	static void addCache(const RzxHostAddress&, const QString&);
	static QList<RzxHostAddress> cachedList();
	static QStringList rawCache(const RzxHostAddress&);
	static QString cache(const RzxHostAddress&);
	static void fillWithCache(const RzxHostAddress&, QTreeWidget*);
	static QString getEmailFromCache(const RzxHostAddress&);
	static QString getNameFromCache(const RzxHostAddress&, const QString&);
	static QString getCacheDate(const RzxHostAddress&);
	static void deleteCache(const RzxHostAddress&);

	static void emitIconFormatChanged();
	RZX_INTPROP("menuTextPos", menuTextPosition, setMenuTextPosition, 2)
	RZX_INTPROP("menuIconSize", menuIconSize, setMenuIconSize, 2)
	RZX_BOOLPROP("hideMainuiOnStartup", hideMainuiOnStartup, setHideMainuiOnStartup, true)

	//Etat du répondeur
	RZX_BOOLPROP("refuseAway", refuseWhenAway, setRefuseWhenAway, true)
	RZX_PROP_DECLARE(bool, autoResponder, setAutoResponder, false)
	RZX_STRINGPROP("txtAutoResponderMsg", autoResponderMsg, setAutoResponderMsg, "Répondeur automatique")
		
	RZX_RGBPROP("repondeur_highlight", repondeurHighlight, setRepondeurHighlight, 0xFD3D3D)
	RZX_RGBPROP("repondeur_base", repondeurBase, setRepondeurBase, 0xFFEE7C)
	RZX_RGBPROP("repondeur_highlightedtext", repondeurHighlightedText, setRepondeurHighlightedText, 0xFFFFFF)
	RZX_RGBPROP("repondeur_normaltext", repondeurNormalText, setRepondeurNormalText, 0x000000)
	RZX_RGBPROP("ignoredBGColor", ignoredBGColor, setIgnoredBGColor, 0xCCCCCC)
	RZX_RGBPROP("ignoredtext", ignoredText, setIgnoredText, 0xEEEEEE)
		
	RZX_RGBPROP("error_back", errorBackgroundColor, setErrorBackgroundColor, 0xFF0000)
	RZX_RGBPROP("error_text", errorTextColor, setErrorTextColor, 0xFFFFFF)
		
	static bool infoCompleted();
	static bool firstLaunch();

	
//Gestion des sous-réseaux
private:
	QStringList rezalNames;
	QStringList rezalLongNames;
	QList< QList<RzxSubnet> > rezalSubnets;

public:
	void loadRezals();
	static uint rezalNumber();
	static int rezal(const RzxSubnet&);
	static int rezal(const QHostAddress&);
	static QString rezalName(int, bool = true);
	static QString rezalName(const QHostAddress&, bool = true);
	static RzxHostAddress rezalBase(int);


//Gestion des répertoires et du stockage des données
public:
	enum DirFlags {
		UserDir = 1,
		SystemDir = 2,
		LibDir = 4,
		CurrentDir = 8,
		TempDir = 16,
		ProgramDirs = UserDir | SystemDir | LibDir,
		AllDirsExceptTemp = ProgramDirs | CurrentDir,
		AllDirs = AllDirsExceptTemp | TempDir,
		DefSearchDirs = ProgramDirs
	};
	Q_DECLARE_FLAGS(Dir, DirFlags);

protected:
	void loadDirs();
	static void addDir(QList<QDir>&, QDir);

public:
	static QDir computerIconsDir();
	static const QDir &userDir();
	static const QDir &systemDir();
	static const QDir &libDir();
	static QList<QDir> dirList(Dir = DefSearchDirs, const QString& = QString(), bool = false, bool = false);
	static QDir dir(DirFlags, const QString& = QString(), bool = false, bool = false);
	static QString findFile(const QString&, Dir = DefSearchDirs, const QString& = QString());

//Quelques signaux...
signals:
	void updateResponder();
	void iconFormatChange();
};

#endif

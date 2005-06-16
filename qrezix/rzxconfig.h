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

#include <QObject>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QVector>
#include <QPixmap>
#include <QDir>
#include <QTranslator>
#include <QSettings>
#include <QPoint>
#include <QList>

class RzxComputer;
class QPixmap;
class RzxProperty;
class RzxHostAddress;

/**
  *@author Sylvain Joyeux
  */

class RzxConfig : public QObject  {
	Q_OBJECT
	friend class RzxProperty;
	friend class RzxProtocole;
	friend class RzxPlugInLoader;
	friend class QRezix;
	
	class FontProperty {
	public:
		bool bold;
		bool italic;
		QList<int> sizes;
		
		FontProperty() { sizes = QList<int>(); }
		FontProperty(bool b, bool i, const QList<int> &pS);
		~FontProperty();
	};
	
	static RzxConfig * Config;
	RzxConfig();
	QHash<QString,QPixmap> allIcons;
	QHash<QString,QPixmap> progIcons;
	QHash<QString,QString> fileEntries;
	QDir m_systemDir;
	QDir m_userDir;
	QDir m_themeDir;
	QDir m_libDir;
	QStringList fontFamilies;
	QHash<QString,FontProperty> fontProperties;

public:
	enum ToolTip
	{
		Enable = 1,
		Ftp = 2,
		Http = 4,
		News = 8,
		Samba = 16,
		Promo = 32,
		OS = 64,
		Client = 128,
		IP = 256,
		Resal = 512,
		Features = 1024,
		Properties = 2048
	};
	
	QSettings *settings;
	static QHash<QString,QTranslator*> translations;
	void loadTranslators();
	static void setLanguage(QString language);
	static RzxConfig * globalConfig();
	~RzxConfig();

// Favoris et BanList
private :
        QSet<QString> favorites;
	QSet<QString> ignoreList;

public:
	bool isFavorite(const QString&) const;
	void addToFavorites(const QString&);
	void delFromFavorites(const QString&);
	void writeFavorites();
	
	bool isBan(const QString&) const;
	void addToBanlist(const QString&);
	void delFromBanlist(const QString&);
	void writeIgnoreList();

// Polices de caractères
	QStringList getFontList();
	const QList<int> getSizes(const QString&) const;
	bool isItalicSupported(const QString&) const;
	bool isBoldSupported(const QString&) const;


	void flush();
	void closeSettings();

	void setPass(const QString& passcode);
	void setOldPass(const QString& oldPass = QString::null);
	
	static int useSystray();
	static int traySize();
	static int useSearch();
	static int defaultTab();
	static int warnCheckingProperties();
	static int printTime();
	static int beep();
	static int beepConnection();
	static bool showConnection();
	static QString beepCmd();
	static QString beepSound();
	static QString connectionSound();
	static int doubleClicRole();
	static int reconnection();
	static int pingTimeout();
	static int chatPort();
	static int serverPort();
	static QString serverName();
	static QString iconTheme(bool def = true);
	static QString FTPPath();
	static int quitMode();
	static bool showQuit();
	void readFavorites();
	void readIgnoreList();
	static void addCache(const RzxHostAddress&, const QString&);
	static QString cache(const RzxHostAddress&);
	static QString getCacheDate(const RzxHostAddress&);

	static QString sambaCmd();
	static QString ftpCmd();
	static QString httpCmd();
	static QString newsCmd();

	static void sambaCmd(QString newstr);
	static void ftpCmd(QString newstr);
	static void httpCmd(QString newstr);
	static void newsCmd(QString newstr);

	// proprietes de l'ordinateur
	static QString propLastName();
	static QString propName();
	static QString propSurname();
	static QString propCasert();
	static QString propMail();
	static QString propWebPage();
	static QString propTel();
	static QString propSport();
	static int numSport();
	static QString propPromo();

	static QString pass();
	static QString oldPass();
	static int colonnes();	
	static int computerIconSize();
	static bool computerIconHighlight();
	static bool refuseWhenAway();
	
	static int menuTextPosition();
	static int menuIconSize();
	static int tooltip();
	static QStringList ignoredPluginsList();
	
	static int autoResponder();
	static QString autoResponderMsg();
	static void setAutoResponder(bool val);
	static QColor repondeurHighlight();
	static QColor repondeurBase();
	static QColor repondeurHighlightedText();
	static QColor repondeurNormalText();
	static QColor ignoredBGColor();
	static QColor ignoredText();
	static void setIconTheme(QObject* parent, const QString& name);
	static QString readWindowSize();
	static void writeWindowSize(QString ws);
	static QPoint readWindowPosition();
	static void writeWindowPosition(const QPoint&);
	static void writeQuitMode(int mode);
	static void writeShowQuit(bool mode);
	static void writeIgnoredPluginsList(const QStringList& list);

	static bool find();
	static QString findData(const QString& name, const QString& relative = QString::null, bool important = false);
	/** Returns the dir where all system-wide data are saved */
	static QDir computerIconsDir();
	static QDir logDir();
	static QDir userDir();
	static QDir systemDir();
	static QDir libDir();
	/** the name of the log's subdirectory */
	static const QString logPath;
	static const QString themePath;

	static RzxComputer * localHost();
	
	static QColor errorTextColor();
	static QColor errorBackgroundColor();
	static const QVector<QPixmap*> yesnoIcons();
	static const QVector<QPixmap*> gatewayIcons();
	static const QVector<QPixmap*> promoIcons();
	static const QPixmap& soundIcon(bool sound=true);
	
	static const QVector<QPixmap*> osIcons(bool large= false);
	static const QPixmap& localhostIcon();
	static void saveIcon(const QString& name, const QPixmap& image);
	
	static QString historique(unsigned long ip, const QString& hostname);
	
	static QString buildLocalhost();

	static const QPixmap& themedIcon(const QString& name);

signals:
	void languageChanged();
	void updateResponder();
	void themeChanged();
	void iconFormatChange();
	
protected: // Protected attributes
	void loadLocalHost();
	
	static const QPixmap &icon(const QString& name);
	static const QPixmap &icon(const QString& name, QHash<QString,QPixmap>& cache, const QString& subdir = QString::null);
	QString readEntry(const QString& name, const QString& def);
	int readEntry(const QString& name, int def);
	QStringList readEntry(const QString& name);
	void writeEntry(const QString& name, const QString& val);
	void writeEntry(const QString& name, int val);
	void writeEntry(const QString& name, const QStringList & list);
	
	static QString name();

private: // Public attributes
	void loadTranslatorsInDir(const QDir &rep);
	static QTranslator* currentTranslator;
	RzxComputer * computer;
};

#endif

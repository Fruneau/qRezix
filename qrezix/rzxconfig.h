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

#include <qobject.h>
#include <qdict.h>
#if (QT_VERSION >= 0x030000)
#include <qptrvector.h>
#include <qmemarray.h>
#else
#include <qvector.h>
#include <qarray.h>
template<class T> class QPtrVector : public QVector<T> {};
template<class T> class QMemArray : public QArray<T> {};
#endif
#include <qpixmap.h>
#include <qdir.h>
#include <qtranslator.h>

class RzxComputer;
class QPixmap;
class RzxProperty;

/**
  *@author Sylvain Joyeux
  */

class RzxConfig : public QObject  {
	Q_OBJECT
	friend class RzxProperty;
	friend class RzxRezal;

	static RzxConfig * Config;
	RzxConfig();
	QDict<QPixmap> allIcons;
	QDict<QPixmap> progIcons;
	QDict<QString> fileEntries;
	QDir m_systemDir;
	QDir m_userDir;
	QDir m_themeDir;
	
public:
	static QDict<QTranslator> translations;
	static void loadTranslators();
	static void setLanguage(QString language);
	QDict<QString> * favorites;
	static RzxConfig * globalConfig();
	~RzxConfig();
	void parse();
	void write();
	void writeFavorites();
	
	static int favoritesMode();
	static void setFavoritesMode(int);
	static int useSystray();
	static int beep();
	static QString beepCmd();
	static QString beepSound();
	static int doubleClicRole();
	static int reconnection();
	static int pingTimeout();
	static int chatPort();
	static int serverPort();
	static QString serverName();
	static QString iconTheme();
	static QString FTPPath();
	void readFavorites();

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

	static int pass();
	static int colonnes();	
	static int autoColonne();
	static int computerIconSize();
	
	static bool autoResponder();
	static QString autoResponderMsg();
	static void setAutoResponder(bool val);
	static QColor repondeurHighlight();
	static QColor repondeurBase();
	static QColor repondeurHighlightedText();
	static QColor repondeurNormalText();
	static void setIconTheme(QObject* parent, const QString& name);
	static QString readWindowSize();
	static void writeWindowSize(QString ws);

	static QString find();
	static QString findData(const QString& name, const QString& relative = QString::null);
	/** Returns the dir where all system-wide data are saved */
	static QDir computerIconsDir();
	static QDir logDir();
	static QDir userDir();
	static QDir systemDir();
	/** the name of the log's subdirectory */
	static const QString logPath;
	static const QString themePath;

	static RzxComputer * localHost();
	
	static QColor errorTextColor();
	static QColor errorBackgroundColor();
	static QMemArray<QPixmap *> yesnoIcons();
	static QMemArray<QPixmap *> gatewayIcons();
	static QMemArray<QPixmap *> promoIcons();
	static QPixmap * soundIcon(bool sound=true);
	
	static QMemArray<QPixmap *> osIcons(bool large= false);
	static QPixmap * localhostIcon();
	static void saveIcon(const QString& name, const QPixmap& image);
	
	static QString historique(unsigned long ip, const QString& hostname);
	
	static QString buildLocalhost();

protected: // Protected attributes
	void loadLocalHost();
	
	static QPixmap * icon(const QString& name);
	static QPixmap * themedIcon(const QString& name);
	static QPixmap * icon(const QString& name, QDict<QPixmap>& cache, const QString& subdir = QString::null);
	QString readEntry(const QString& name, const QString& def);
	int readEntry(const QString& name, int def);
	void writeEntry(const QString& name, const QString& val);
	void writeEntry(const QString& name, int val);
	
	static QString name();

private: // Public attributes
	static void loadTranslatorsInDir(QDir rep);
	static QTranslator* currentTranslator;
	RzxComputer * computer;
};

#endif

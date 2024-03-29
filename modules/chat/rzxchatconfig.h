/***************************************************************************
                       rzxchatconfig.h  -  description
                             -------------------
    begin                : Sat Aug 13 2005
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
#ifndef RZXCHATCONFIG_H
#define RZXCHATCONFIG_H

#include <QDir>
#include <QList>
#include <QHash>
#include <RzxConfig>
#include <RzxAbstractConfig>

/**
 @author Florent Bruneau
 */

///Propri�t�s du module de chat
class RzxChatConfig:public RzxAbstractConfig
{
	Q_OBJECT
	RZX_CONFIG_EXPANDED(RzxChatConfig)

	public:
		RZX_BOOLPROP("beep", beep, setBeep, true)
		RZX_STRINGPROP("beepSound", beepSound, setBeepSound, QString())

		RZX_BOOLPROP("warnWhenChecked", warnWhenChecked, setWarnWhenChecked, false)
		RZX_BOOLPROP("printTime", printTime, setPrintTime, true)
		RZX_BOOLPROP("printName", printName, setPrintName, false)
		RZX_BOOLPROP("printIcon", printIcon, setPrintIcon, true)
		RZX_INTPROP("iconSize", iconSize, setIconSize, 16)
		RZX_BOOLPROP("printPrompt", printPrompt, setPrintPrompt, true)
		RZX_STRINGPROP("prompt", prompt, setPrompt, ">")
		RZX_UINTPROP("chatPort", chatPort, setChatPort, 5050)
		RZX_UINTPROP("filePort", filePort, setFilePort, 5055)
		RZX_BOOLPROP("refuseFileTransfer", refuseFile, setRefuseFile , false)
		RZX_BOOLPROP("displayIconText", displayIconText, setDisplayIconText , true)

		RZX_WIDGETPROP("chat", restoreChatWidget, saveChatWidget, QPoint(150,100), QSize(500, 300))

		static QDir logDir();
		static QString historique(quint32, const QString&);

// Smileys
		RZX_STRINGPROP("smileyTheme", smileyTheme, setSmileyTheme, "MSNLike")


// Polices de caract�res
	private:
		class FontProperty
		{
			public:
				bool bold;
				bool italic;
				QList<int> sizes;

				FontProperty() { sizes = QList<int>(); }
				FontProperty(bool b, bool i, const QList<int> &pS): bold(b), italic(i), sizes(pS) { }
				~FontProperty() { }
		};
		QStringList fontFamilies;
		QHash<QString,FontProperty> fontProperties;
		void loadFontList();

	public:
		static QStringList getFontList();
		static QString nearestFont(const QString&);
		static int nearestSize(const QString&, int);
		static const QList<int> getSizes(const QString&);
		static bool isItalicSupported(const QString&);
		static bool isBoldSupported(const QString&);

};

///Retourne le r�pertoire o� rechercher les logs de conversations...
inline QDir RzxChatConfig::logDir()
{
	return RzxConfig::dir(RzxConfig::UserDir, "log", true, true);
}

///Retourne le nom du fichier d'historique
inline QString RzxChatConfig::historique(quint32 ip, const QString& hostname)
{
	QString filename = QString::number(ip, 16);
	
	QDir logdir = logDir();
	filename = filename + "[" + hostname + "].html";
	
	if (logdir.exists(filename))
		return logdir.absoluteFilePath(filename);

	// anciens formats
	QString olds[2];
	olds[0] = QString::number(ip, 16) + "[" + hostname + "].txt";
	olds[1] = QString::number(ip, 16) + ".txt";
	for (int oldidx = 0; oldidx < 2; oldidx++)
	{
		QString& prevFileName = olds[oldidx];
		if (logdir.exists(prevFileName))
		{
			logdir.rename(prevFileName, filename);
			break;
		}
	}
	
	return logdir.absoluteFilePath(filename);
}

#endif

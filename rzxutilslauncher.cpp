/***************************************************************************
                          rzxutilslauncher.cpp  -  description
                             -------------------
    begin                : Fri Sep 10 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QProcess>

#include "rzxutilslauncher.h"

#include "rzxcomputer.h"
#include "rzxconfig.h"

///Objet global pour un accès facile
RzxUtilsLauncher *RzxUtilsLauncher::object = NULL;

///Construction : on référencie juste un rezazl
/** Le RzxRezal fournit les outils de correspondance IP-Nom */
RzxUtilsLauncher::RzxUtilsLauncher()
{
	lister = RzxConnectionLister::global();
}

RzxUtilsLauncher::~RzxUtilsLauncher()
{
	object = NULL;
}

// lance le client ftp
void RzxUtilsLauncher::ftp(const QString& s_login) const
{
	QString login = s_login.section("/", 0,0);
	QString path = s_login.section("/",1);
	
	RzxComputer *computer = lister->getComputerByName(login);
	if(!computer) return;

	QString tempPath = RzxConfig::globalConfig()->FTPPath();
	QString tempip = computer->getIP().toString();
	QString ip=tempip;
	tempip="ftp://"+tempip+"/"+path;

	QString cmd;
#ifdef WIN32
	QString sFtpClient=RzxConfig::globalConfig()->ftpCmd();

	// LeechFTP
	if(sFtpClient == "LeechFTP")
	{
		QSettings regReader(QSettings::UserScope, "LeechFTP");
		if(regReader.contains("AppDir"))
		{
			//Réglage des paramètres du proxy
			regReader.setValue("ProxyMode", 0);
			regReader.setValue("LocalDir", tempPath);
			
			//Récupération du chemin d'accès
			cmd=regReader.value("AppDir").toString()+"leechftp.exe";
		}
	}
	// SmartFTP
	else if(sFtpClient == "SmartFTP")
	{
		QSettings regReader(QSettings::SystemScope, "SmartFTP");
		if(regReader.contains("Install Directory"))
		{
			//Réglage des paramètres du proxy
			regReader.beginGroup("ProxySettings");
			if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
			{
				QString proxyExcpt = regReader.value("Proxy Exceptions").toString();
				if(!proxyExcpt.contains(ip + ";"))
					regReader.setValue("Proxy Exceptions", ip + ";" + proxyExcpt);
			}
			else
				reader.setValue("Proxy Type", 0);
			regReader.endGroup();

			//Récupération du chemin d'accès
			regReader.setValue("Network/Default Path", tempPath);
			cmd = regReader.value("Install Directory").toString() + "smartftp.exe";
		}
	}
	// Internet Explorer
	else if (sFtpClient == "iExplore")
		cmd = "explorer";
	else if( sFtpClient == "standard")
		cmd = tempip;
	else
		cmd =  sFtpClient;
#else
#ifdef Q_OS_MAC
	if(RzxConfig::globalConfig()->ftpCmd() == "Default")
		cmd = "open";
	else
		cmd = RzxConfig::globalConfig()->ftpCmd();
#else
	if(RzxConfig::globalConfig()->ftpCmd() == "lftp")
	{
		// on lance le client dans un terminal
#ifdef WITH_KDE
		cmd = "konsole -e lftp";
#else
		cmd = "xterm -e lftp";
#endif
	}
	else
		cmd = RzxConfig::globalConfig()->ftpCmd();
#endif //MAC
#endif //WIN32
	
	QProcess process;
	process.setWorkingDirectory(path);
	process.start(cmd + " " + tempip);
	
}

void RzxUtilsLauncher::samba(const QString& login) const
{
	RzxComputer *computer = lister->getComputerByName(login);
	if(!computer) return;
	
	QString ip = computer->getIP().toString();
	QString path = RzxConfig::globalConfig()->FTPPath();

	// Composition de la ligne de commande
	QString cmd;
	QStringList args;

#ifdef WIN32
	cmd = "explorer";
	args << "\\\\" + ip;
#else
	#ifdef Q_OS_MAC
	cmd = "open";
	#else
	cmd = "konqueror";
	#endif //MAC
	args << "smb://" + ip + "/";
#endif
		
	QProcess process;
	process.setWorkingDirectory(path);
	process.start(cmd, args);
}

// lance le client http
void RzxUtilsLauncher::http(const QString& login) const
{
	RzxComputer *computer = lister->getComputerByName(login);
	if(!computer) return;
	
	QString ip = computer->getIP().toString();
	QString path = RzxConfig::globalConfig()->FTPPath();
	QString tempip = "http://" + ip;
	
	// Composition de la ligne de commande
	QString cmd = RzxConfig::globalConfig()->httpCmd();
	QStringList args;

#ifdef WIN32
	if(cmd == "Firefox")
	{
		cmd = "firefox";
		args << tempip;
	}
	else if(cmd == "Opera")
	{
		QSettings regReader(QSettings::UserScope, "Opera Software");
		if(regReader.contains("Last CommandLine"))
		{
			cmd = regReader.value("Last CommandLine").toString();
			args << tempip;
		}
	}
	else if(cmd == "iExplore")
	{
		cmd = "explorer"
		args << tempip;
	}
	else if(cmd == "standard")
		cmd = tempip;
	else
	{
		args << tempip;
	}
#else
	cmd = RzxConfig::globalConfig()->httpCmd();
	#ifdef Q_OS_MAC
	if(cmd == "Default")
		cmd = "open";
	#endif
	args << tempip;
#endif
	
	QProcess process;
	process.setWorkingDirectory(path);
	process.start(cmd, args);
}

// lance le client news
void RzxUtilsLauncher::news(const QString& login) const
{
	RzxComputer *computer = lister->getComputerByName(login);
	if(!computer) return;
	
	QString ip = computer->getIP().toString();
	QString path = RzxConfig::globalConfig()->FTPPath();
	QString tempip = "news://" + ip;
	
	// Composition de la ligne de commande
	QString cmd = RzxConfig::globalConfig()->newsCmd();
	QStringList args;

#ifdef WIN32
	if(cmd == "standard" )
		cmd = tempip
	else
		args << tempip;
#else
	#ifdef Q_OS_MAC
	if(cmd == "Default")
		cmd = "open";
	#endif
	args << tempip;
#endif
	
	QProcess process;
	process.setWorkingDirectory(path);
	process.start(cmd, args);
}

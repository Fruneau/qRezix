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
#include <QUrl>

#include <RzxUtilsLauncher>

#include <RzxHostAddress>
#include <RzxConfig>

///Tente d'ouvrir une URL avec le bon client
void RzxUtilsLauncher::run(const QUrl& m_url)
{
	const QString url = m_url.toString();

	static QRegExp mailMatch("^(mailto:)?([^/\\@]+\\@[^\\@]+)");
	if(mailMatch.indexIn(url) != -1)
	{
		mail(mailMatch.cap(2));
		return;
	}

	static QRegExp sambaMatch("\\\\\\\\([^\\\\]+)(\\\\.*)?");
	if(sambaMatch.indexIn(url) != -1)
	{
		samba(sambaMatch.cap(1), sambaMatch.cap(2));
		return;
	}

	static QRegExp fullMatch("([^:]+://)?([^/:]+)(/.*)?");
	if(fullMatch.indexIn(url) != -1)
	{
		const QString proto = fullMatch.cap(1);
		const QString host = fullMatch.cap(2);
		const QString path = fullMatch.cap(3);
		if(proto == "http://" || proto == "https://" || proto.isEmpty())
			http(host, path);
		else if(proto == "ftp://")
			ftp(host, path);
		else if(proto == "news://")
			news(host, path);
		else if(proto == "smb://")
			samba(host, path);
	}
}

///Lance un client ftp
void RzxUtilsLauncher::ftp(const RzxHostAddress& m_ip, const QString& path)
{
	ftp(m_ip.toString(), path);
}

///Lance le client ftp
void RzxUtilsLauncher::ftp(const QString& host, const QString& path)
{
	const QString tempPath = RzxConfig::ftpPath();
	const QString tempip= "ftp://"+host+"/"+path;
	QString cmd;
	QStringList args;
#ifdef WIN32
	QString sFtpClient=RzxConfig::ftpCmd();

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
	// FileZilla
	else if(sFtpClient == "FileZilla")
	{
		cmd = "filezilla.exe";
		QSettings regReader("HKEY_CURRENT_USER\\Software\\FileZilla", QSettings::NativeFormat);
		if(regReader.contains("Install_Dir"))
		{
			cmd = regReader.value("Install_Dir").toString() + "\\filezilla.exe";
		}
	}
	// SmartFTP
	else if(sFtpClient == "SmartFTP")
	{
		QSettings regReaderUser("HKEY_CURRENT_USER\\Software\\SmartFTP", QSettings::NativeFormat);
		QSettings regReader("HKEY_LOCAL_MACHINE\\Software\\SmartFTP", QSettings::NativeFormat);
		
		if(regReader.contains("Install Directory"))
		{
			//Réglage des paramètres du proxy
			regReaderUser.beginGroup("ProxySettings");
			if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
			{
				QString proxyExcpt = regReaderUser.value("Proxy Exceptions").toString();
				if(!proxyExcpt.contains(host + ";"))
					regReaderUser.setValue("Proxy Exceptions", host + ";" + proxyExcpt);
			}
			else
				regReaderUser.setValue("Proxy Type", 0);
			regReaderUser.endGroup();

			//Récupération du chemin d'accès
			regReaderUser.setValue("Network/Default Path", tempPath);
			cmd = regReader.value("Install Directory").toString() + "smartftp.exe";
		}
	}
	// Internet Explorer
	else if (sFtpClient == "IExplorer")
		cmd = "explorer";
	else if( sFtpClient == "standard")
		cmd = "explorer";
	else
		cmd =  sFtpClient;
	args << tempip;
	launchCommand(cmd , args, path);
#else
#ifdef Q_OS_MAC
	if(RzxConfig::global()->ftpCmd() == "Default")
		cmd = "open";
	else
		cmd = RzxConfig::global()->ftpCmd();
#else
	if(RzxConfig::global()->ftpCmd() == "lftp")
	{
		// on lance le client dans un terminal
#ifdef WITH_KDE
		cmd = "konsole -e lftp";
#else
		cmd = "xterm -e lftp";
#endif
	}
	else
		cmd = RzxConfig::global()->ftpCmd();
#endif //MAC
	launchCommand(cmd + " " + tempip, QStringList(), path);
#endif //WIN32
}

///Lance le client samba
void RzxUtilsLauncher::samba(const RzxHostAddress& m_ip, const QString& m_path)
{
	samba(m_ip.toString(), m_path);
}

///Lance le client samba
void RzxUtilsLauncher::samba(const QString& host, const QString& m_path)
{
	const QString path = RzxConfig::ftpPath();

	// Composition de la ligne de commande
	QString cmd;
	QStringList args;

#ifdef WIN32
	cmd = "explorer";
	args << "\\\\" + host + "\\" + m_path;
#else
	#ifdef Q_OS_MAC
	cmd = "open";
	#else
	cmd = "konqueror";
	#endif //MAC
	args << "smb://" + host + "/" + m_path;
#endif
		
	launchCommand(cmd, args, path);
}

///Lance le client http
void RzxUtilsLauncher::http(const RzxHostAddress& m_ip, const QString& m_path)
{
	http(m_ip.toString(), m_path);
}

///Lance le client http
void RzxUtilsLauncher::http(const QString& host, const QString& m_path)
{
	const QString path = RzxConfig::ftpPath();
	const QString tempip = "http://" + host + "/" + m_path;
	
	// Composition de la ligne de commande
	QString cmd = RzxConfig::httpCmd();
	QStringList args;

#ifdef WIN32
	if(cmd == "Firefox")
	{
		QSettings regReader(QSettings::UserScope, "Mozilla", "Mozilla Firefox");
		cmd = "firefox";
		if(regReader.contains("CurrentVersion"))
		{
			QString version = regReader.value("CurrentVersion").toString();
			regReader.beginGroup(version);
			regReader.beginGroup("Main");
			if(regReader.contains("PathToExe"))
				cmd = regReader.value("PathToExe").toString();
			regReader.endGroup();
			regReader.endGroup();
		}
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
	else if(cmd == "IExplorer")
	{
		cmd = "explorer";
		args << tempip;
	}
	else if(cmd == "standard")
	{
		cmd = "explorer";
		args << tempip;
	}
	else
	{
		args << tempip;
	}
#else
	cmd = RzxConfig::global()->httpCmd();
#	ifdef Q_OS_MAC
	if(cmd == "Default")
		cmd = "open";
#	endif
	args << tempip;
#endif
	
	launchCommand(cmd, args, path);
}

///Lance le client news
void RzxUtilsLauncher::news(const RzxHostAddress& m_ip, const QString& m_path)
{
	news(m_ip.toString(), m_path);
}

///Lance le client news
void RzxUtilsLauncher::news(const QString& host, const QString& m_path)
{
	const QString path = RzxConfig::ftpPath();
	const QString tempip = "news://" + host + "/" + m_path;
	
	// Composition de la ligne de commande
	QString cmd = RzxConfig::newsCmd();
	QStringList args;

#ifdef WIN32
	if(cmd == "standard" )
		cmd = tempip;
	else if(cmd == "Thunderbird")
	{
		QSettings regReader(QSettings::UserScope, "Mozilla", "Mozilla Thunderbird");
		cmd = "thunderbird.exe";
		if(regReader.contains("CurrentVersion"))
		{
			QString version = regReader.value("CurrentVersion").toString();
			regReader.beginGroup(version);
			regReader.beginGroup("Main");
			if(regReader.contains("PathToExe"))
				cmd = regReader.value("PathToExe").toString();
			regReader.endGroup();
			regReader.endGroup();
		}
		args << tempip;
	}
	else
		args << tempip;
#else
#	ifdef Q_OS_MAC
	if(cmd == "Default")
		cmd = "open";
#	endif
	args << tempip;
#endif
	
	launchCommand(cmd, args, path);
}

///Demande l'envoie d'un mail à l'adresse indiquée
void RzxUtilsLauncher::mail(const QString& email)
{
	const QString mailto = "mailto:" + email;
	QStringList args;
	QString cmd = RzxConfig::mailCmd();

#ifdef WIN32
	if(cmd == "standard")
		cmd = mailto;
	else if(cmd == "Thunderbird")
	{
		QSettings regReader(QSettings::UserScope, "Mozilla", "Mozilla Firefox");
		cmd = "thunderbird.exe";
		if(regReader.contains("CurrentVersion"))
		{
			QString version = regReader.value("CurrentVersion").toString();
			regReader.beginGroup(version);
			regReader.beginGroup("Main");
			if(regReader.contains("PathToExe"))
				cmd = regReader.value("PathToExe").toString();
			regReader.endGroup();
			regReader.endGroup();
		}
		args << mailto;
	}
	else
		args << email;
#else
#	ifdef Q_OS_MAC
	if(cmd == "Default")
	{
		cmd = "open";
		args << mailto;
	}
	else
#	else
	if(cmd == "mail")
	{
		// on lance le client dans un terminal
#ifdef WITH_KDE
		cmd = "konsole -e mail";
#else
		cmd = "xterm -e mail";
#endif //WITH_KDE
	}
#endif // Q_OS_MAC
		args << email;
#endif //WIN32

	launchCommand(cmd, args);
}

///Lance une commande
/** En lui définissant la commande à passer, ses paramètres, et le répertoire de travail.
 *
 * La commande lancée sera killée à la fermeture et le QProcess sera automatique détruit
 * à la fin de l'exécution de la commande indiquée...
 */
QProcess *RzxUtilsLauncher::launchCommand(const QString& cmd, const QStringList& args, const QString& path)
{
	QProcess *process = new QProcess();
	QObject::connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
	if(!path.isNull())
		process->setWorkingDirectory(path);
	qDebug()<<cmd << "  " << args;
	if(args.isEmpty())
		process->start(cmd);
	else
		process->start(cmd, args);
	return process;
}

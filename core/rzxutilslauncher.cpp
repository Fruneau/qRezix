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
	QRegExp fullMatch("([^:]+://)?([^/:]+)(/.*)?");
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
#ifdef WIN32
	QString sFtpClient=RzxConfig::ftpCmd();
	
	// LeechFTP
	if(sFtpClient == "LeechFTP")
	{
		QSettings regReader(QSettings::UserScope, "LeechFTP");
		if(regReader.contains("AppDir"))
		{
			//R�glage des param�tres du proxy
			regReader.setValue("ProxyMode", 0);
			regReader.setValue("LocalDir", tempPath);
			
			//R�cup�ration du chemin d'acc�s
			cmd=regReader.value("AppDir").toString()+"leechftp.exe";
		}
	}
	// SmartFTP
	else if(sFtpClient == "SmartFTP")
	{
		QSettings regReaderUser("HKEY_CURRENT_USER\\Software\\SmartFTP", QSettings::NativeFormat);
		QSettings regReader("HKEY_LOCAL_MACHINE\\Software\\SmartFTP", QSettings::NativeFormat);
		
		if(regReader.contains("Install Directory"))
		{
			//R�glage des param�tres du proxy
			regReaderUser.beginGroup("ProxySettings");
			if(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)
			{
				QString proxyExcpt = regReaderUser.value("Proxy Exceptions").toString();
				if(!proxyExcpt.contains(ip + ";"))
					regReaderUser.setValue("Proxy Exceptions", ip + ";" + proxyExcpt);
			}
			else
				regReaderUser.setValue("Proxy Type", 0);
			regReaderUser.endGroup();

			//R�cup�ration du chemin d'acc�s
			regReaderUser.setValue("Network/Default Path", tempPath);
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
#endif //WIN32
	
	launchCommand(cmd + " " + tempip, QStringList(), path);
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
		cmd = "explorer";
		args << tempip;
	}
	else if(cmd == "standard")
		cmd = tempip;
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

///Lance une commande
/** En lui d�finissant la commande � passer, ses param�tres, et le r�pertoire de travail.
 *
 * La commande lanc�e sera kill�e � la fermeture et le QProcess sera automatique d�truit
 * � la fin de l'ex�cution de la commande indiqu�e...
 */
QProcess *RzxUtilsLauncher::launchCommand(const QString& cmd, const QStringList& args, const QString& path)
{
	QProcess *process = new QProcess();
	QObject::connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
	if(!path.isNull())
		process->setWorkingDirectory(path);
	if(args.isEmpty())
		process->start(cmd);
	else
		process->start(cmd, args);
	return process;
}

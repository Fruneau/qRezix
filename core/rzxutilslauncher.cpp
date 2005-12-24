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

#include <RzxUtilsLauncher>

#include <RzxHostAddress>
#include <RzxConfig>

///Lance le client ftp
void RzxUtilsLauncher::ftp(const RzxHostAddress& m_ip, const QString& path)
{
	QString tempPath = RzxConfig::ftpPath();
	QString tempip = m_ip.toString();
	QString ip=tempip;
	tempip="ftp://"+tempip+"/"+path;

	QString cmd;
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
				regReader.setValue("Proxy Type", 0);
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
	QString ip = m_ip.toString();
	QString path = RzxConfig::ftpPath();

	// Composition de la ligne de commande
	QString cmd;
	QStringList args;

#ifdef WIN32
	cmd = "explorer";
	args << "\\\\" + ip + "\\" + m_path;
#else
	#ifdef Q_OS_MAC
	cmd = "open";
	#else
	cmd = "konqueror";
	#endif //MAC
	args << "smb://" + ip + "/" + m_path;
#endif
		
	launchCommand(cmd, args, path);
}

///Lance le client http
void RzxUtilsLauncher::http(const RzxHostAddress& m_ip, const QString& m_path)
{
	QString ip = m_ip.toString();
	QString path = RzxConfig::ftpPath();
	QString tempip = "http://" + ip + "/" + m_path;
	
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
	QString ip = m_ip.toString();
	QString path = RzxConfig::ftpPath();
	QString tempip = "news://" + ip + "/" + m_path;
	
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
	if(args.isEmpty())
		process->start(cmd);
	else
		process->start(cmd, args);
	return process;
}

/***************************************************************************
                          rzxclientlistener.cpp  -  description
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

#ifdef WIN32
	#include <windows.h>
	#include <tchar.h>
	#include <malloc.h>
	#include <stdlib.h>
	#ifdef UNICODE
		#define RzxShellExecute(a, b, str, c, d, e) \
			ShellExecute( a, b, (LPCWSTR)(str.unicode()), c, d, e )
	#else
		#define RzxShellExecute(a, b, str, c, d, e) \
			ShellExecute( a, b, (LPCSTR)(str.latin1()), c, d, e )
	#endif
	#define RzxWinExec(str, a) \
		WinExec((LPCSTR)(str.latin1()), a)
#else
	#include <stdlib.h>
#endif

#include "rzxutilslauncher.h"
#include "rzxconfig.h"
#include "rzxrezal.h"
#include "rzxitem.h"

RzxUtilsLauncher *RzxUtilsLauncher::object = NULL;

RzxUtilsLauncher::RzxUtilsLauncher(RzxRezal *m_rezal)
{
	object = this;
	rezal = m_rezal;
}

RzxUtilsLauncher::~RzxUtilsLauncher()
{
	object = NULL;
}

// lance le client ftp
void RzxUtilsLauncher::ftp(const QString& login)
{
	qDebug(login);
	int offset = login.find("/");
	QString path;
	if(offset == -1) path = "";
	else path = login.mid(offset+1);
	
	QString m_login;
	m_login = login.left(offset);
	
	RzxItem *item = (RzxItem*)object->rezal->findItem(m_login, RzxRezal::ColNom, RzxRezal::ExactMatch);
	if(!item) return;
	// int serveurs=item->servers;
	QString tempPath = RzxConfig::globalConfig()->FTPPath();
	QString tempip = (item->ip).toString();
	QString ip=tempip;
	tempip="ftp://"+tempip+"/"+path;
	qDebug(tempip);

#ifdef WIN32
	int iRegValue = 0;
	TCHAR strRegValue[] = TEXT("0");
	HKEY hKey;

	QString sFtpClient=RzxConfig::globalConfig()->ftpCmd();

	// leechftp :
	if( (!sFtpClient.compare("LeechFTP")) &&
		!RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\LeechFTP"), 0, KEY_ALL_ACCESS, &hKey) )
	{
		RegSetValueEx(hKey, TEXT("ProxyMode"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
		RegSetValueEx(hKey, TEXT("LocalDir"),0,REG_SZ,
			(unsigned char*)(QDir::convertSeparators(RzxConfig::globalConfig()->FTPPath()).latin1()),
			RzxConfig::globalConfig()->FTPPath().length() * sizeof(unsigned char));

		unsigned char buffer[MAX_PATH];
		unsigned long KeyType = 0;
		unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
		RegQueryValueEx(hKey, TEXT("AppDir"), 0, &KeyType, buffer, &KeySize);
		RegCloseKey(hKey);
		QString temp=(char*)buffer;
		QString cmd=temp+"leechftp.exe " + tempip;

		RzxWinExec(cmd, 1);
	}
	// smartftp :
	else if( (!sFtpClient.compare("SmartFTP")) &&
		!RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SmartFTP"), 0, KEY_ALL_ACCESS, &hKey))
	{
		RegCloseKey(hKey);
		HKEY hKey2;
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SmartFTP\\ProxySettings"), 0, KEY_ALL_ACCESS, &hKey2);
		unsigned char buffer[MAX_PATH];
		unsigned char * pointer;
		unsigned long KeyType = 0;
		unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
  
		if ( QApplication::winVersion() & Qt::WV_NT_based ){
			unsigned long size;
			RegQueryInfoKey(hKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&size,NULL,NULL);
			size+=ip.length() * sizeof(unsigned char);
			unsigned char *buffer2;
			buffer2 = (unsigned char *)malloc(size);
			strcpy((char*)buffer2,ip.latin1());
			pointer=buffer2;
			buffer2[ip.length()]=';';
			pointer+=ip.length()+1;
			RegQueryValueEx(hKey2, TEXT("Proxy Exceptions"), 0, &KeyType, pointer, &size);
			QString test((char*)pointer);
   
			if(!test.contains(ip+";")){
				RegSetValueEx(hKey2,TEXT("Proxy Exceptions"),0,REG_SZ,(const unsigned char*)buffer2,KeySize+ip.length()+1);
			}
			free(buffer2);
		}
		else RegSetValueEx(hKey, TEXT("Proxy Type"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
		RegCloseKey(hKey2);

		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SmartFTP\\Network"), 0, KEY_ALL_ACCESS, &hKey2);
		RegSetValueEx(hKey2,TEXT("Default Path"),0,REG_SZ,
				(unsigned char*)(QDir::convertSeparators(RzxConfig::globalConfig()->FTPPath()).latin1()),
				RzxConfig::globalConfig()->FTPPath().length() * sizeof(unsigned char));
		RegCloseKey(hKey2);
  
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SmartFTP"), 0, KEY_ALL_ACCESS, &hKey);
		RegQueryValueEx(hKey, TEXT("Install Directory"), 0, &KeyType, buffer, &KeySize);
		QString temp=(char*)buffer;
		RegCloseKey(hKey);
		QString cmd=temp+"smartftp.exe " + tempip;

		RzxWinExec(cmd, 1);

	}

	// bulletproof FTP
/* else if (iFtpClient == 3)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\BPFTP\\Bullet Proof FTP\\Options"), 0, KEY_ALL_ACCESS, &hKey);
		RegSetValueEx(hKey, TEXT("FirewallEnabled"), 0, REG_SZ, LPBYTE(& strRegValue), 2 * sizeof(TCHAR));
		RegCloseKey(hKey);
	}*/

	else{ // client FTP standard
		QString cmd="explorer " + tempip;
		RzxWinExec(cmd, 1);
	}
#else
	QString cmd = "cd "+tempPath+"; "+RzxConfig::globalConfig()->ftpCmd()+" "+tempip;
	if(RzxConfig::globalConfig()->ftpCmd() == "lftp")
	// on lance le client dans un terminal
	#ifdef WITH_KDE
		cmd = "konsole -e \"" + cmd + "\" &";
	#else
		cmd = "xterm -e \"" + cmd + "\" &";
	#endif
	else
	//client graphique
		cmd = cmd + " &";

	system(cmd.latin1());
#endif
}

void RzxUtilsLauncher::samba(const QString& login){
	RzxItem *item = (RzxItem*)object->rezal->findItem(login, RzxRezal::ColNom, RzxRezal::ExactMatch);
	if(!item) return;

	QString tempip = (item -> ip).toString();
	QString tempPath = RzxConfig::globalConfig()->FTPPath();

#ifdef WIN32
	int serveurs=item->servers;
	QString cmd = "explorer \\\\" + (item -> ip).toString();
	RzxWinExec(cmd, 1);
#else
	QString cmd = "cd "+tempPath+"; konqueror smb://" + (item ->ip).toString() + "/" +" &";
	system(cmd.latin1());
#endif
}

/*void RzxRezal::fermetureLeechFTP(){
	int iRegValue = 2;
	HKEY hKey;

	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\LeechFTP"), 0, KEY_ALL_ACCESS, &hKey);
	RegSetValueEx(hKey, TEXT("ProxyMode"), 0, REG_DWORD, LPBYTE(& iRegValue), 4);
	RegCloseKey(hKey);
}*/

// lance le client http
void RzxUtilsLauncher::http(const QString& login)
{
	RzxItem *item = (RzxItem*)object->rezal->findItem(login, RzxRezal::ColNom, RzxRezal::ExactMatch);
	if(!item) return;
	QString tempip = "http://" + (item -> ip).toString();
	QString cmd=RzxConfig::globalConfig()->httpCmd();

#ifdef WIN32
	if( cmd == "standard" )
	RzxShellExecute( NULL, NULL, tempip, NULL, NULL, SW_SHOW );
	else
	{
		int serveurs = item->servers;
		QString cmd;
		HKEY hKey;
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, KEY_ALL_ACCESS, &hKey);

		if ( cmd == "Opera" && !RegOpenKeyEx(hKey, TEXT("Opera Software"), 0, KEY_ALL_ACCESS, &hKey) ) {
  
			unsigned char buffer[MAX_PATH];
			unsigned long KeyType = 0;
			unsigned long KeySize = sizeof(TCHAR) * MAX_PATH;
			RegQueryValueEx(hKey, TEXT("Last CommandLine"), 0, &KeyType, buffer, &KeySize);
			RegCloseKey(hKey);
			QString temp=(char *)buffer;
			cmd = temp + tempip;
		}
		else
		cmd = "explorer " + tempip;

 
		RzxWinExec(cmd, 1);
	}
#else
	cmd = cmd + " " + tempip + " &";
	system(cmd.latin1());
#endif
}

// lance le client news
void RzxUtilsLauncher::news(const QString& login)
{
	RzxItem *item = (RzxItem*)object->rezal->findItem(login, RzxRezal::ColNom, RzxRezal::ExactMatch);
	if(!item) return;
	QString tempip = "news://" + (item -> ip).toString();
	QString cmd = RzxConfig::globalConfig()->newsCmd();

#ifdef WIN32
	int serveurs=item->servers;
	if( cmd == "standard" )
		cmd = "explorer " + tempip;
	else
		cmd = cmd + " " + tempip;

	RzxWinExec(cmd,1);
#else
	cmd = cmd + " " + tempip + " &";
	system(cmd.latin1());
#endif
}

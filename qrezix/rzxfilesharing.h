/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef RZXFILESHARING_H
#define RZXFILESHARING_H
/*Gestion du partage des fichiers du ftp*/

#include <qobject.h>
#include <qurlinfo.h>
#include <qftp.h>
#include <qstring.h>
#include <qvaluestack.h>
#include <qvaluelist.h>
#include <qfile.h>
#include <qtimer.h>

class FileTable
{
	public:
		QString path;
		QString name;
		QString hash;
		QString date;
		bool old;

		FileTable();
		FileTable(const QString& p, const QString& n, const QString& h, const QString& d, bool o = TRUE);
		~FileTable();

		bool operator==(const FileTable& t);
		bool operator>(const FileTable& t);
		bool operator>=(const FileTable& t);
		bool operator<(const FileTable& t);
		bool operator<=(const FileTable& t);
};

class RzxFileSharing:public QFtp
{
	Q_OBJECT
	
	QString ftpname;

	int listingId;
	int command;
	bool isRunning;
	QValueStack<int> commandPile;
	QValueStack<QString> currentUrlPile;
	QValueStack<QUrlInfo> infoPile;
	QString currentUrl;
	QString saveRepName;
	
//Donn�es pour la r�ception des donn�es

//Donn�es pour la g�n�ration du hash et des donn�es de la base
	QUrlInfo info;
	unsigned int size;
	unsigned int hash;
	QFile *logFile;

//Donn�e du fichier
	QValueList<FileTable> logs;
	QTimer *timer;

//Fichier de log des ftps
	public:
		RzxFileSharing(const QString& ftpn, const QString& ftpentete, QObject* parent=0, const char *name=0);
		~RzxFileSharing();
		void stop();
		void runNextCommand();
		void changeDns(const QString& dnsname);

	public slots:	
		void launch();

	protected:
		void fileHash(const QString& filename);
		void changeUrl(const QString& cdi);
		bool loadLogFile();
		FileTable parse(const QString&);
		QValueListIterator<FileTable> existEntry(const QString& path, const QString& name);
		void addEntry(const QString& path, const QString& name, const QString& hash, const QString& date);
		void cleanLogs();
		bool writeLogs(bool clean = TRUE);
		QString getPath();

	protected slots:
		void index(const QUrlInfo& i);
		void stateChange(int state);
		void commandFinish(int id, bool err);
		void commandDone(bool err);
		void dataGet(int done, int size);
		void readData();
};

#endif



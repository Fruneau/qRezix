#include <qobject.h>
#include <qvaluestack.h>
#include <qvaluelist.h>
#include <qurlinfo.h>
#include <qftp.h>
#include <qstring.h>
#include <qfile.h>
#include <qdir.h>
#include "rzxmessagebox.h"

#include "rzxfilesharing.h"

#define FTP_FILEINDEX "ftpindex"

/**Classe qui contient les �l�ments pour enregistrer les logs**/
FileTable::FileTable():path(), name(), hash(), date() { old = TRUE; }
FileTable::FileTable(const QString& p, const QString& n, const QString& h, const QString& d, bool o)
	:path(p), name(n), hash(h), date(d)
{
	old = o;
}

FileTable::~FileTable()
{ }

bool FileTable::operator==(const FileTable& t)
{
	return path == t.path && name == t.name && hash == t.hash && date == t.date;
}

bool FileTable::operator>(const FileTable& t)
{
	return hash > t.hash;
}

bool FileTable::operator>=(const FileTable& t)
{
	return hash >= t.hash;
}

bool FileTable::operator<(const FileTable& t)
{
	return hash < t.hash;
}

bool FileTable::operator<=(const FileTable& t)
{
	return hash <= t.hash;
}


/**Classe d'indexation du ftp**/
RzxFileSharing::RzxFileSharing(QObject* parent, const char *name)
	:QFtp(parent, name), commandPile(), urlPile(), infoPile(), url(), info(), logs()
{
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChange(int)));
	connect(this, SIGNAL(done(bool)), this, SLOT(commandDone(bool)));
	connect(this, SIGNAL(commandFinished(int, bool)), this, SLOT(commandFinish(int, bool)));
	connect(this, SIGNAL(listInfo(const QUrlInfo&)), this, SLOT(index(const QUrlInfo&)));
	connect(this, SIGNAL(dataTransferProgress(int, int)), this, SLOT(dataGet(int, int)));
	hash = 0;
	logFile = NULL;
}

RzxFileSharing::~RzxFileSharing()
{ }

void RzxFileSharing::launch(const QString& ftpname)
{
	loadLogFile();
	qDebug("ftp connecting to " + ftpname);
	connectToHost(ftpname);
}

void RzxFileSharing::stateChange(int state)
{
	switch(state)
	{
		case QFtp::Connected:
			qDebug("ftp connection OK... logging in");
			login();
			break;

		case QFtp::LoggedIn:
			qDebug("ftp logging in OK... indexing");
			listingId = list();
			break;

		case QFtp::Unconnected:
			qDebug("ftp unconnected");
			break;

		case QFtp::HostLookup:
			qDebug("ftp looking for host");
			break;

		case QFtp::Connecting:
			qDebug("ftp host found OK... connecting");
			break;

		case QFtp::Closing:
			qDebug("ftp closing connection");
			writeLogs();
			break;

		default: qDebug("ftp state changed");
	}
}

void RzxFileSharing::changeUrl(const QString& cdi)
{
	if(cdi != "..")
		url += cdi + "/";
	else
	{
		int offset = url.findRev('/', -2);
		if(offset == -1) url = "";
		else
		url = url.left(offset + 1);
	}
}

void RzxFileSharing::commandFinish(int id, bool err)
{
	if(err)
		getError(error());
	else if(id == listingId && !bytesAvailable())
		runNextCommand();
}

void RzxFileSharing::runNextCommand()
{
	do
	{
		if(commandPile.isEmpty())
		{
			abort();
			close();
			return;
		}
		QString u = urlPile.pop();
		int c = commandPile.pop();
		switch(c)
		{
			case QFtp::Cd:
				changeUrl(u);
				qDebug("ftp cd " + url);
				cd(u);
				if(u != QString(".."))
				{
					qDebug("ftp listing " + url);
					listingId = list();
				return;
				}
				break;

			case QFtp::Get:
				fileHash(u);
				return;
		}
	}
	while(1);
}

void RzxFileSharing::fileHash(const QString& filename)
{
	info = infoPile.pop();
	QValueListIterator<FileTable> it = existEntry(url, filename);
	if(it != (QValueListIterator<FileTable>)0)
	{
		QString date = info.lastModified().toString("yyMMddhhmmss");
		unsigned int asize = (*it).hash.left(8).toUInt(0, 16);
		if(date == (*it).date && info.size() == asize)
		{
			(*it).old = FALSE;
			qDebug("ftp file up-to-date in database");
			runNextCommand();
		}
		return;
	}

	qDebug("ftp hashing "+ filename);
	listingId = get(filename);
}

void RzxFileSharing::dataGet(int done, int size)
{
	readData();
	if(done == size)
	{
		QString hashStr = QString("%1%2").arg((unsigned int)size, 8, 16).arg(hash, 8, 16).replace(' ', '0');
		qDebug("ftp hash generated " + hashStr + "... adding entry");
		QString date = info.lastModified().toString("yyMMddhhmmss");
		addEntry(url, info.name(), hashStr, date);
		runNextCommand();
	}
}

void RzxFileSharing::readData()
{
	char *buf;
	int size;
	buf = new char[0x100000];

	while((size = bytesAvailable()))
	{
		if((size = readBlock(buf, 0x100000)) == -1)
			qDebug("ftp read error");

		for(int i=0 ; i< size>>2 ; i++)
			hash += *(((int*)buf)+i);
	}
	delete buf;
}

void RzxFileSharing::commandDone(bool err)
{
	if(err)
		getError(error());
}

void RzxFileSharing::index(const QUrlInfo& i)
{
	qDebug("ftp find " + i.name());
	if(i.isFile())
	{
		if(i.isReadable())
		{
			urlPile.push(i.name());
			commandPile.push(QFtp::Get);
			infoPile.push(i);
		}
	}
	else
	{
		if(i.isReadable())
		{
			urlPile.push("..");
			commandPile.push(QFtp::Cd);
			urlPile.push(i.name());
			commandPile.push(QFtp::Cd);
		}
	}
}

void RzxFileSharing::getError(Error err)
{
	if(err == QFtp::HostNotFound) qDebug("ftp host not found");
	else if(err == QFtp::ConnectionRefused) qDebug("ftp connection refused");
	else if(err == QFtp::NotConnected) qDebug("ftp not connected");
	else if(err == QFtp::UnknownError)
	{
		qDebug("ftp unknown error");
		commandFinish(listingId, FALSE);
	}
}

//Gestion des entr�es sorties sur le fichier de configuration
bool RzxFileSharing::loadLogFile()
{
	QDir userDir;

#ifdef WIN32
	userDir = QDir::currentDirPath();
#else
	userDir = QDir::home();
	if (!userDir.cd(".rezix"))
	{
		if (!userDir.mkdir(".rezix"))
		{
			QString msg;
			msg = tr("Septembre cannot create %1, which is the folder\nin which its configuration is saved\n")
				.arg(userDir.absFilePath(".rezix"));
			msg += tr("You will not be able to save your configuration");
			RzxMessageBox::critical(0, "Septembre", msg);
		}
		else
			userDir.cd(".rezix");
	}
#endif
	logFile = new QFile(userDir.filePath(FTP_FILEINDEX));
	if(!logFile->exists())
		return TRUE;

	if(!logFile->open(IO_ReadOnly))
	{
		qDebug("ftp can't open log file");
		delete logFile;
		return FALSE;
	}

	while(!logFile->atEnd())
	{
		QString ligne;
		if(logFile->readLine(ligne, 1024) == -1)
		{
			qDebug("ftp can't read log file");
			logFile->close();
			delete logFile;
			return FALSE;
		}
		logs << parse(ligne);
	}

	logFile->close();
	return TRUE;
}

bool RzxFileSharing::writeLogs()
{
	if(!logFile) return FALSE;
	if(!logFile->open(IO_WriteOnly))
	{
		qDebug("ftp can't open/create log file");
		delete logFile;
		return FALSE;
	}

	if(!logs.isEmpty())
	{
		qDebug("ftp cleaning database");
		cleanLogs();
		QValueListIterator<FileTable> it;
		for(it = logs.begin() ; it != logs.end() ; it++)
		{

			QString msg =  QString((*it).path + (*it).name +  (*it).hash + (*it).date + "\n");
			const char *buf = msg.latin1();
			qDebug(msg);
			if(logFile->writeBlock(buf, msg.length()) == -1)
			{
				qDebug("ftp can't write in log file");
				logFile->close();
				delete logFile;
				return FALSE;
			}
		}
	}
	else
	{
		qDebug("ftp logs empty");
	}
	logFile->close();
	delete logFile;
	return TRUE;
}

//format "path/filenamehashdate" avec date yyMMddhhmmss
FileTable RzxFileSharing::parse(const QString &ligne)
{
	QString msg = ligne;
	QString path, date, hash;
	if(msg.at(msg.length()-1) == '\n') msg = msg.left(msg.length() -1);
	date = msg.right(12);
	msg = msg.left(msg.length() - 12);
	hash = msg.right(16);
	msg = msg.left(msg.length() - 16);

	int offset = msg.findRev('/', -1);
	if(offset != -1)
	{
		path = msg.left(offset+1);
		msg = msg.mid(offset+1);
	}
	else
		path = "";

	qDebug("ftp load " + path + " " + msg + " " + hash + " " + date);

	return FileTable(path, msg, hash, date);
}

QValueListIterator<FileTable> RzxFileSharing::existEntry(const QString& path, const QString& name)
{
	if(logs.isEmpty()) return (QValueListIterator<FileTable>)0;

	QValueListIterator<FileTable> it;
	for(it = logs.begin() ; it != logs.end() ; it++)
		if((*it).path == path && (*it).name == name) return it;
	return (QValueListIterator<FileTable>)0;
}

void RzxFileSharing::addEntry(const QString& path, const QString& name, const QString& hash, const QString& date)
{
	QValueListIterator<FileTable> it;
	if((it = existEntry(path, name)) == (QValueListIterator<FileTable>)0)
		logs << FileTable(path, name, hash, date, FALSE);
	else
	{
		(*it).hash = hash;
		(*it).date = date;
		(*it).old = FALSE;
	}
}

void RzxFileSharing::cleanLogs()
{
	QValueListIterator<FileTable> it = logs.begin();
	while(it != logs.end())
	{
		if((*it).old)
			it = logs.remove(it);
		else
			it++;
	}
}


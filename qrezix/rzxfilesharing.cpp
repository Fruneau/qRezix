#include <qobject.h>
#include <qvaluestack.h>
#include <qvaluelist.h>
#include <qurlinfo.h>
#include <qftp.h>
#include <qstring.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>
#include "rzxmessagebox.h"

#include "rzxfilesharing.h"

#define FTP_FILEINDEX "ftpindex"

/**Classe qui contient les éléments pour enregistrer les logs**/
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
RzxFileSharing::RzxFileSharing(const QString& ftpn, const QString& ftpentete, QObject* parent, const char *name)
	:QFtp(parent, name), ftpname(ftpn), commandPile(), urlPile(), infoPile(), url(), info(), logs()
{
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChange(int)));
	connect(this, SIGNAL(done(bool)), this, SLOT(commandDone(bool)));
	connect(this, SIGNAL(commandFinished(int, bool)), this, SLOT(commandFinish(int, bool)));
	connect(this, SIGNAL(listInfo(const QUrlInfo&)), this, SLOT(index(const QUrlInfo&)));
	connect(this, SIGNAL(dataTransferProgress(int, int)), this, SLOT(dataGet(int, int)));
	hash = 0;
	isRunning = FALSE;
	logFile = NULL;
	timer = NULL;
	url = QString("ftp://" + ftpentete.lower() + "/");
}

RzxFileSharing::~RzxFileSharing()
{ }

void RzxFileSharing::changeDns(const QString& dnsname)
{
	url = url.mid(6);
	int offset = url.find('/');
	url = url.mid(offset);
	url = "ftp://" + dnsname.lower() + url;
	if(!isRunning)
		loadLogFile();
		
	QValueListIterator<FileTable> it;
	for(it = logs.begin() ; it != logs.end() ; it++)
		(*it).path = "ftp://" + dnsname.lower() + (*it).path.mid(offset + 6);
	
	if(!isRunning)
		writeLogs(FALSE);
}

void RzxFileSharing::launch()
{
	if(isRunning || !loadLogFile()) return;
	if(!timer)
	{
		timer = new QTimer();
		connect(timer, SIGNAL(timeout()), this, SLOT(launch())); 
	}
	else
		return;
	qDebug("ftp connecting to " + ftpname);
	isRunning = TRUE;
	connectToHost(ftpname);
}

void RzxFileSharing::stop()
{
	if(timer)
	{
		timer->stop();
		delete timer;
		timer = NULL;
	}
	abort();
	close();
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
			isRunning = FALSE;
			break;

		case QFtp::HostLookup:
			qDebug("ftp looking for host");
			break;

		case QFtp::Connecting:
			qDebug("ftp host found OK... connecting");
			break;

		case QFtp::Closing:
			qDebug("ftp closing connection");
			commandPile.erase(commandPile.begin(), commandPile.end());
			urlPile.erase(urlPile.begin(), urlPile.end());
			infoPile.erase(infoPile.begin(), infoPile.end());
			if(timer)
			{
				timer->start(6*3600*1000);
				writeLogs();
			}
			else
				writeLogs(FALSE);
			logs.erase(logs.begin(), logs.end());
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

//Gestion des entrées sorties sur le fichier de configuration
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

bool RzxFileSharing::writeLogs(bool clean)
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
		if(clean) cleanLogs();
		QValueListIterator<FileTable> it;
		for(it = logs.begin() ; it != logs.end() ; it++)
		{

			QString msg =  QString((*it).path + (*it).name +  (*it).hash + (*it).date + "\n");
			const char *buf = msg.latin1();
			if(logFile->writeBlock(buf, msg.length()) == -1)
			{
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


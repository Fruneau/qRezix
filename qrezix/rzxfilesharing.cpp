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
	:QFtp(parent, name), ftpname(ftpn), commandPile(), currentUrlPile(), infoPile(), currentUrl(), saveRepName(), info(), logs()
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
	currentUrl = QString("ftp://" + ftpentete.lower() + "/");
}

RzxFileSharing::~RzxFileSharing()
{
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
	if(currentId()) abort();
	if(state() != QFtp::Unconnected) close();
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
			saveRepName = "";
			command = QFtp::List;
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
			currentUrlPile.erase(currentUrlPile.begin(), currentUrlPile.end());
			infoPile.erase(infoPile.begin(), infoPile.end());
			if(isRunning)
			{
				if(timer)
				{
					timer->start(6*3600*1000);
					writeLogs();
				}
				else
					writeLogs(FALSE);
			}
			logs.erase(logs.begin(), logs.end());
			break;

		default: qDebug("ftp state changed");
	}
}

void RzxFileSharing::changeUrl(const QString& cdi)
{
	if(cdi != "..")
		currentUrl += cdi + "/";
	else
	{
		int offset = currentUrl.findRev('/', -2);
		if(offset == -1) currentUrl = "";
		else
		currentUrl = currentUrl.left(offset + 1);
	}
}

QString RzxFileSharing::getPath()
{
	QString path;
	path = currentUrl.mid(6);
	int offset = path.find('/');
	path = path.mid(offset);
	return path;
}

void RzxFileSharing::changeDns(const QString& dnsname)
{
	currentUrl = currentUrl.mid(6);
	int offset = currentUrl.find('/');
	currentUrl = currentUrl.mid(offset);
	currentUrl = "ftp://" + dnsname.lower() + currentUrl;
	if(!isRunning)
		loadLogFile();
		
	QValueListIterator<FileTable> it;
	for(it = logs.begin() ; it != logs.end() ; it++)
		(*it).path = "ftp://" + dnsname.lower() + (*it).path.mid(offset + 6);
	
	if(!isRunning)
		writeLogs(FALSE);
}

void RzxFileSharing::commandFinish(int id, bool err)
{
	if(!err && id == listingId)
	{
		if(command == QFtp::List && saveRepName != "")
		{
			commandPile.push(QFtp::Cd);
			currentUrlPile.push(saveRepName);
			saveRepName = "";
		}
		runNextCommand();
	}
}

void RzxFileSharing::runNextCommand()
{
	if(commandPile.isEmpty())
	{
		abort();
		close();
		return;
	}
	QString u = currentUrlPile.pop();
	int c = commandPile.pop();
	command = c;
	switch(c)
	{
		case QFtp::Cd:
			changeUrl(u);
			qDebug("ftp cd " + currentUrl);
			listingId = cd(u);
			return;
			
		case QFtp::List:
			qDebug("ftp listing " + currentUrl + u);
			currentUrlPile.push("..");
			commandPile.push(QFtp::Cd);
			saveRepName = u;
			listingId = list(u);
			return;
			
		case QFtp::Get:
			fileHash(u);
			return;
	}
}

void RzxFileSharing::fileHash(const QString& filename)
{
	info = infoPile.pop();
	QValueListIterator<FileTable> it = existEntry(currentUrl, filename);
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
	size = 0;
	get(filename);
	command = QFtp::Get;
}

void RzxFileSharing::dataGet(int done, int sizeF)
{
	if(size < 1024) readData();
	else readAll();
	if((done == sizeF && size != 1025) || size == 1024)
	{
		QString hashStr = QString("%1%2").arg((unsigned int)sizeF, 8, 16).arg(hash, 8, 16).replace(' ', '0');
		qDebug("ftp hash generated " + hashStr + "... adding entry");
		QString date = info.lastModified().toString("yyMMddhhmmss");
		addEntry(currentUrl, info.name(), hashStr, date);
		if(size == 1024) abort();
		size = 1025;
		runNextCommand();
	}
}

void RzxFileSharing::readData()
{
	char *buf;
	int sizeHere;
	buf = new char[1024];

	while((sizeHere = bytesAvailable()) && size < 1024)
	{
		if(size + sizeHere > 1024) sizeHere = 1024 - size;
		if((sizeHere = readBlock(buf, sizeHere)) == -1)
			qDebug("ftp read error");

		for(int i=0 ; i< sizeHere>>2 ; i++)
			hash += *(((int*)buf)+i);
		size += sizeHere;
	}
	delete buf;
}

void RzxFileSharing::commandDone(bool err)
{
	if(!err) return;
	
	int er = error();
	if(er == QFtp::HostNotFound) qDebug("ftp " + errorString());
	else if(er == QFtp::ConnectionRefused) qDebug("ftp " + errorString());
	else if(er == QFtp::NotConnected)
	{
		qDebug("ftp NC " + errorString());
		if(state() == QFtp::LoggedIn) qDebug("Menteur");
		runNextCommand();
	}
	else if(er == QFtp::UnknownError)
	{
		qDebug("ftp UE " + errorString());
		if(command == QFtp::List) //cas de Permission Denied
		{
			QString u;
			int c;
			do
			{
				c = commandPile.pop();
				u = currentUrlPile.pop();
				switch(c)
				{
					case QFtp::Cd:
						changeUrl(u);
						qDebug("ftp cd(" + u + ") skip " + currentUrl);
						break;
						
					case QFtp::List:
						qDebug("ftp can't list " + currentUrl);
						break;
				}
			}
			while(c != QFtp::Cd || u != "..");
		}
		runNextCommand();
	}
}

void RzxFileSharing::index(const QUrlInfo& i)
{
	qDebug("ftp find " + i.name());
	if(i.isFile())
	{
		if(i.permissions() & QUrlInfo::ReadOther)
		{
			currentUrlPile.push(i.name());
			commandPile.push(QFtp::Get);
			infoPile.push(i);
		}
	}
	else
	{
		if(i.permissions() & QUrlInfo::ReadOther)
		{
			currentUrlPile.push(i.name());
			commandPile.push(QFtp::List);
		}
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


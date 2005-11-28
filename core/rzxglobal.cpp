/***************************************************************************
                          rzxglobal  -  description
                             -------------------
    begin                : Sun Jul 24 2005
    copyright            : (C) 2005 by Florent Bruneau
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
#include <QString>
#include <QRegExp>

#include <RzxGlobal>

static QtMsgHandler oldMsgHandler = NULL;
static FILE *logfile = NULL;
int level = 0;

///Message handler spécifique pour avoir un formatage intéressant
void msgHandler(QtMsgType type, const char *msg)
{
	char *finalMsg;
	int len = strlen(msg);
	finalMsg = new char[len + 1 + level*2];
	for(int i = 0 ; i < level*2 ; i++)
		finalMsg[i] = ' ';
	memcpy(finalMsg + level*2, msg, len+1);
	if(logfile != NULL)
		fprintf(logfile, "[%s] %s\n", type==QtDebugMsg ? "debug" : type==QtWarningMsg ? "warning" : "error",finalMsg);
	
	if(oldMsgHandler != NULL)
		(*oldMsgHandler)(type,finalMsg);

	if(logfile == NULL && oldMsgHandler == NULL)
		printf("%s\n", finalMsg);

	delete[] finalMsg;
}

///Pour définir un fichier de log particulier
void Rzx::useOutputFile(FILE *file)
{
	logfile = file;
}

///Remplace le render de Qt par le msgHandler de qRezix
void Rzx::installMsgHandler()
{
	oldMsgHandler = qInstallMsgHandler(msgHandler);
}

///Ferme de fichier de log
void Rzx::closeMsgHandler()
{
	if(logfile)
		fclose(logfile);
}

///Permet juste de loguer le début du chargement d'un module
void Rzx::beginModuleLoading(const QString& module)
{
	QString msg = module;
	switch(level)
	{
		case 0: msg = "=== Loading " + msg + " ==="; break;
		case 1: msg = "--- Loading " + msg + " ---"; break;
		case 2: msg = "--> Loading " + msg + "..."; break;
		default: msg = "* Loading " + msg + "..."; break;
	}
	qDebug("%s", msg.toAscii().constData());
	level++;
}

///Permet juste de logguer la fin du chargement d'un module
void Rzx::endModuleLoading(const QString& module, bool success)
{
	QString msg = module;
	if(!success)
	{
		msg = "Loading failed : " + msg;
		qDebug("%s", msg.toAscii().constData());
	}
	switch(--level)
	{
		case 0: msg = "=== " + module + " loading finished ===\n"; break;
		default: return;
	}
	qDebug("%s", msg.toAscii().constData());
}

///Permet juste de loguer le début de fermeture d'un module
void Rzx::beginModuleClosing(const QString& module)
{
	QString msg = module;
	switch(level)
	{
		case 0: msg = "\n=== Closing " + msg + " ==="; break;
		case 1: msg = "--- Closing " + msg + " ---"; break;
		case 2: msg = "--> Closing " + msg + "..."; break;
		default: msg = "* Closing " + msg + "..."; break;
	}
	qDebug("%s", msg.toAscii().constData());
	level++;
}

///Permet juste de logguer la fin de la fermeture d'un module
void Rzx::endModuleClosing(const QString& module)
{
	QString msg = module;
	switch(--level)
	{
		case 0: msg = "=== " + msg + " closed  ===\n"; break;
		default: return;
	}
	qDebug("%s", msg.toAscii().constData());
}

///Pour trier...
bool Rzx::caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}


///Converti la version en numéro de version lisible par l'utilisateur
/** Le numéro de version est de la forme :
 * major.minor.build[-tag],
 * le tag étant ignoré si et seulement si la chaîne est nulle
 */
QString Rzx::versionToString(const Rzx::Version& version, Rzx::VersionParts parts)
{
	QString value;
	if(parts & MajorVersion)
		value += QString::number(version.major);
	if(parts & MinorVersion)
	{
		if(!value.isEmpty()) value += ".";
		value += QString::number(version.minor);
	}
	if(parts & BuildVersion)
	{
		if(!value.isEmpty()) value += ".";
		value += QString::number(version.build);
	}
	if(parts & TagVersion && !version.tag.isNull())
		value += version.tag;
	return value;
}

///Converti une chaîne de caractère en version
Rzx::Version Rzx::versionFromString(const QString& name)
{
	Version version;
	QRegExp mask("^(\\d+)\\.(\\d+)\\.(\\d+)(\\S*)$");
	if(mask.indexIn(name) == -1)
	{
		version.major = version.minor = version.build = 0;
		version.tag = QString();
	}
	else
	{
		version.major = mask.cap(1).toUInt();
		version.minor = mask.cap(2).toUInt();
		version.build = mask.cap(3).toUInt();
		version.tag = mask.cap(4);
	}
	return version;
}

///Compare les numéros de version
bool Rzx::operator==(const Rzx::Version& a, const Rzx::Version& b)
{
	return a.major == b.major && a.minor == b.minor && a.build == b.build;
}

///Compare les numéros de version
bool Rzx::operator<(const Rzx::Version& a, const Rzx::Version& b)
{
	if(a.major < b.major) return true;
	if(a.minor < b.minor) return true;
	if(a.build < b.build) return true;
	return false;
}

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
#include <QTime>

#include <RzxGlobal>

static QtMsgHandler oldMsgHandler = NULL;
static FILE *logfile = NULL;
int level = 0;
bool withTS = false;

///Message handler spécifique pour avoir un formatage intéressant
void msgHandler(QtMsgType type, const char *msg)
{
	if( msg == NULL )
		return;
	if( level < 0 )	level = 0;
	char *finalMsg;
	int len = strlen(msg);
	finalMsg = new char[len + 1 + level*2];
	if( finalMsg == NULL )	return;

	memset( finalMsg, ' ', level*2 );
	memcpy(finalMsg + level*2, msg, len+1);

	FILE *file = logfile;
	if(!file && !oldMsgHandler)
	{
		if(type >= QtCriticalMsg)
			file = stderr;
		else
			file = stdout;
	}

	if(withTS && file)
		fprintf(file, "[%s]", QTime::currentTime().toString("hh:mm:ss").toAscii().constData());

	if(logfile)
		fprintf(logfile, "[%s]", type==QtDebugMsg ? "debug" : type==QtWarningMsg ? "warning" : "error");

	if(file)
		fprintf(file, "%s\n", finalMsg);

//	if(oldMsgHandler != NULL)
//		(*oldMsgHandler)(type,finalMsg);

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
	if(a.major == b.major && a.minor < b.minor) return true;
	if(a.major == b.major && a.minor == b.minor && a.build < b.build) return true;
	return false;
}

///Compare les numéros de version
bool Rzx::operator>=(const Rzx::Version& a, const Rzx::Version& b)
{
	return !(a < b);
}

///Converti le système d'exploitation en identifiant d'icône
Rzx::Icon Rzx::toIcon(Rzx::SysEx os, bool large)
{
	switch(os)
	{
		case SYSEX_WIN9X:  return large ? ICON_OS1_LARGE : ICON_OS1;
		case SYSEX_WINNT: return large ? ICON_OS2_LARGE : ICON_OS2;
		case SYSEX_LINUX: return large ? ICON_OS3_LARGE : ICON_OS3;
		case SYSEX_MACOS: return large ? ICON_OS4_LARGE : ICON_OS4;
		case SYSEX_MACOSX: return large ? ICON_OS5_LARGE : ICON_OS5;
		case SYSEX_BSD: return large ? ICON_OS6_LARGE : ICON_OS6;
		default: return large ? ICON_OS0_LARGE : ICON_OS0;
	}
}

///Converti la promo en identifiant d'icône
Rzx::Icon Rzx::toIcon(Rzx::Promal promo)
{
	switch(promo)
	{
		case PROMAL_ROUJE: return ICON_ROUJE;
		case PROMAL_JONE: return ICON_JONE;
		default: return ICON_ORANJE;
	}
}

///Converti l'état du répondeur en icône
Rzx::Icon Rzx::toIcon(Rzx::ConnectionState state)
{
	if(state == STATE_HERE) return ICON_HERE;
	return ICON_AWAY;
}

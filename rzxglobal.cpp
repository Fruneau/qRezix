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

#include "rzxglobal.h"

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
		case 0: msg = "=== Loading module " + msg + " ==="; break;
		case 1: msg = "--- Loading sub-module " + msg + " ---"; break;
		case 2: msg = "--> Loading sub-sub-module " + msg + "..."; break;
		default: msg = "* Loading " + msg + "..."; break;
	}
	qDebug("%s", msg.toAscii().constData());
	level++;
}

///Permet juste de logguer la fin du chargement d'un module
void Rzx::endModuleLoading(const QString& module, bool success)
{
	QString msg = module;
	if(success)
		switch(--level)
		{
			case 0: msg = "=== " + msg + " loaded with success  ===\n"; break;
//			case 1: msg = "--- " + msg + " loaded with success ---"; break;
//			case 2: msg = "--> " + msg + " loaded with success..."; break;
//			default: msg = "* " + msg + "loaded with success..."; break;
			default: return;
		}
	else
		switch(--level)
		{
			case 0: msg = "=== " + msg + " failed  ===\n"; break;
			case 1: msg = "--- " + msg + " failed ---"; break;
			case 2: msg = "--> " + msg + " failed..."; break;
			default: msg = "* " + msg + "failed..."; break;
		}
	qDebug("%s", msg.toAscii().constData());
}

///Pour trier...
bool Rzx::caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}


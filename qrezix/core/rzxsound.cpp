/***************************************************************************
                          rzxsound  -  description
                             -------------------
    begin                : Sat Nov 5 2005
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
#include <QSound>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QApplication>

#include <RzxConfig>

#include <RzxSound>

///Lecture d'un son
void RzxSound::play(const QString& file)
{
	if(file.isEmpty() || !QFile(file).exists())
		QApplication::beep();
	else
	{
#if defined(WIN32) || defined(Q_OS_MAC)
		QSound::play(file);
#else
		QString cmd = RzxConfig::beepCmd();
		if(cmd.isEmpty())
			QSound::play(file);
		else
		{
			QProcess process;
			process.start(cmd, QStringList(file));
		}
#endif
	}
}

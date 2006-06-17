/***************************************************************************
                          rzxsound  -  description
                             -------------------
    begin                : Sat Jun 17 2006
    copyright            : (C) 2006 by Florent Bruneau
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

#ifndef RZX_QUICKRUN_H
#define RZX_QUICKRUN_H

#include <QDialog>
#include <RzxGlobal>
#include <RzxComputer>

namespace Ui { class RzxQuickRun; }

class RzxQuickRun : public QDialog
{
	Q_OBJECT
	
	Ui::RzxQuickRun *ui;
	
	enum Actions
	{
		Chat = 0,
		Ftp = 1,
		Http = 2,
		News = 3,
		Samba = 4,
		Mail = 5,
		Properties = 6,
		History = 7,
		AddFavorite = 8,
		RemoveFavorite = 9,
		AddBan = 10,
		RemoveBan = 11,
		Away = 12,
		Preferences = 13,
		NbActions = 14
	};
	
	struct Action
	{
		char *name;
		Rzx::Icon icon;
		bool needComputer;
		RzxComputer::testComputer *filter;
	};
	
	static Action actions[NbActions];

	public:
		RzxQuickRun(QWidget *parent = 0);
		
	public slots:
		virtual void accept();
	
	protected slots:
		void actionChanged(int);
};

#endif

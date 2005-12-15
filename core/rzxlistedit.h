/***************************************************************************
            rzxlistedit  -  fenêtre d'édition de computer list
                             -------------------
    begin                : Thu Dec 15 2005
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
#ifndef RZXLISTEDIT_H
#define RZXLISTEDIT_H

#include <QWidget>

class RzxComputerList;
namespace Ui { class RzxListEditUI; }

class RzxListEdit: public QWidget
{
	Q_OBJECT

	Ui::RzxListEditUI *ui;
	RzxComputerList *list;

	public:
		RzxListEdit(QWidget *parent = NULL);
		~RzxListEdit();

	public slots:
		void setList(RzxComputerList *);
		void changeTheme();

	protected slots:
		void refresh();
		void add();
		void remove();
		void edited(const QString&);
		void selectionChanged(int);
};

#endif

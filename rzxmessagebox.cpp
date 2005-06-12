/***************************************************************************
                          rzxmessagebox.h  -  description
                             -------------------
    begin                : Fri Jan 24 2003
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qmessagebox.h>
#include "rzxmessagebox.h"


int RzxMessageBox::information( QWidget *parent, const QString& caption,
	const QString& text, bool modal ){
	QMessageBox *mb = new QMessageBox( caption, text, QMessageBox::Information,
		0, 0, 0, parent, "qt_msgbox_information",
		modal, Qt::WDestructiveClose );
	mb->show();
	return 0;
}


int RzxMessageBox::warning( QWidget *parent, const QString& caption,
	const QString& text, bool modal ){
	QMessageBox *mb = new QMessageBox( caption, text, QMessageBox::Warning,
		0, 0, 0, parent, "qt_msgbox_warning",
		modal, Qt::WDestructiveClose );
	mb->show();
	return 0;
}


int RzxMessageBox::critical( QWidget *parent, const QString& caption,
	const QString& text, bool modal ){
	QMessageBox *mb = new QMessageBox( caption, text, QMessageBox::Critical,
		0, 0, 0, parent, "qt_msgbox_critical",
		modal, Qt::WDestructiveClose );
	mb->show();
	return 0;
}


/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "qrezixquit.h"

const QRezixQuit::selectAbort		= 0x0;
const QRezixQuit::selectMinimize	= 0x1;
const QRezixQuit::selectQuit		= 0x2;


//connexion des boutons à leurs action respectives
QRezixQuit::QRezixQuit(QWidget* parent, const char *name)
	: QRezixQuitUI(parent, name)
{
	connect((QObject *)radioQuit, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioMinimize, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioDoNothing, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)btnApply, SIGNAL(clicked()), this, SLOT(done()));
	quitOptionChange();
}

QRezixQuit::~QRezixQuit()
{
}

//Recherche du bouton coché, et extraction d'un numéro
void QRezixQuit::quitOptionChange(void)
{
	if(radioQuit->isChecked())
	{
		selection = selectQuit;
		btnApply->setText(tr("Quit now !"));
	}
	else if(radioMinimize->isChecked())
	{
		selection = selectMinimize;
		btnApply->setText(tr("Minimize me..."));
	}
	else if(radioDoNothing->isChecked())
	{
		selection = selectAbort;
		btnApply->setText(tr("Abort quitting please"));
	}
}

//Sortie de la fenêtre entre retournant le code d'action
void QRezixQuit::done()
{
	QDialog::done(selection);
}
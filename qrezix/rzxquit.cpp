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

#include "rzxquit.h"

const int RzxQuit::selectAbort		= 0x0;
const int RzxQuit::selectMinimize	= 0x1;
const int RzxQuit::selectQuit		= 0x2;


//connexion des boutons à leurs action respectives
RzxQuit::RzxQuit(QWidget* parent, const char *name)
	: RzxQuitUI(parent, name)
{
	connect((QObject *)radioQuit, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioMinimize, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioDoNothing, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)btnApply, SIGNAL(clicked()), this, SLOT(done()));
	quitOptionChange();
}

RzxQuit::~RzxQuit()
{
}

//Recherche du bouton coché, et extraction d'un numéro
void RzxQuit::quitOptionChange(void)
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
void RzxQuit::done()
{
	QDialog::done(selection);
}
/***************************************************************************
                         rzxquit.cpp  -  description
                             -------------------
    begin                : Thu Jun 24 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "rzxquit.h"
#include "rzxconfig.h"

//connexion des boutons � leurs action respectives
RzxQuit::RzxQuit(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	connect((QObject *)radioQuit, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioMinimize, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioDoNothing, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)btnApply, SIGNAL(clicked()), this, SLOT(done()));
	quitOptionChange();
}

RzxQuit::~RzxQuit()
{
}

//Recherche du bouton coch�, et extraction d'un num�ro
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

//Sortie de la fen�tre entre retournant le code d'action
void RzxQuit::done()
{
	RzxConfig::writeQuitMode(selection);
	RzxConfig::writeShowQuit(!cbSave->isChecked());
	QDialog::done(selection);
}

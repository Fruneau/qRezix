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

#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

#include <RzxConfig>

#include "rzxquit.h"
#include "rzxmainuiconfig.h"

///connexion des boutons à leurs action respectives
RzxQuit::RzxQuit(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	connect((QObject *)radioQuit, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioMinimize, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect((QObject *)radioDoNothing, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	quitOptionChange();
}

RzxQuit::~RzxQuit()
{
}

///Retourne l'état de la sélection
RzxQuit::QuitMode RzxQuit::quitMode() const
{
	return selection;
}

///Recherche du bouton coché, et extraction d'un numéro
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

///Sortie de la fenêtre entre retournant le code d'action
void RzxQuit::on_btnApply_clicked()
{
	RzxMainUIConfig::setQuitMode(selection);
	RzxMainUIConfig::setShowQuit(!cbSave->isChecked());
	QDialog::done(selection);
}

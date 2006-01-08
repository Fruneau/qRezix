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
#include "ui_rzxquit.h"

#include "rzxmainuiconfig.h"

///connexion des boutons à leurs action respectives
RzxQuit::RzxQuit(QWidget* parent)
	: QDialog(parent)
{
	selection = None;
	ui->setupUi(this);
	connect(ui->radioQuit, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect(ui->radioMinimize, SIGNAL(clicked()), this, SLOT(quitOptionChange()));
	connect(ui->radioDoNothing, SIGNAL(clicked()), this, SLOT(quitOptionChange()));

#ifdef Q_OS_MAC
	ui->radioMinimize->setText(tr("no, I want to hide qRezix"));
#endif

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
	if(ui->radioQuit->isChecked())
	{
		selection = Quit;
		ui->btnApply->setText(tr("Quit now !"));
	}
	else if(ui->radioMinimize->isChecked())
	{
		selection = Minimize;
#ifndef Q_OS_MAC
		ui->btnApply->setText(tr("Minimize me..."));
#else
		ui->btnApply->setText(tr("Hide me..."));
#endif
	}
	else if(ui->radioDoNothing->isChecked())
	{
		selection = Abort;
		ui->btnApply->setText(tr("Abort quitting please"));
	}
}

///Sortie de la fenêtre entre retournant le code d'action
void RzxQuit::on_btnApply_clicked()
{
	RzxMainUIConfig::setQuitMode(selection);
	RzxMainUIConfig::setShowQuit(!ui->cbSave->isChecked());
	QDialog::done(selection);
}

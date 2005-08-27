/***************************************************************************
                          rzxchangepass  -  description
                             -------------------
    begin                : Sat Aug 27 2005
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
#include <QRegExpValidator>

#include <RzxThemedIcon>
#include <RzxNetwork>

#include "rzxchangepass.h"

RzxChangePass::RzxChangePass(RzxNetwork *net, const QString &oldpass)
	:network(net)
{
	init(oldpass);
	if(net)
		setAttribute(Qt::WA_DeleteOnClose);
}

RzxChangePass::RzxChangePass(const QString &oldpass)
	:network(NULL)
{
	init(oldpass);
}


RzxChangePass::~RzxChangePass()
{
}


void RzxChangePass::init(const QString& oldPass)
{
	if(!oldPass.isNull())
		setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	setupUi(this);
	
	//Application du masque pour être sur du formatage du password
	leNewPass->setValidator(new QRegExpValidator(QRegExp(".{6,63}"), this));
	connect(leNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	connect(leReenterNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	btnOK->setEnabled(false);
	if(oldPass.isNull())
		btnCancel->hide();
	
	//Rajout des icônes aux boutons
	btnOK->setIcon(RzxThemedIcon(Rzx::ICON_OK));
	btnCancel->setIcon(RzxThemedIcon(Rzx::ICON_CANCEL));

	//Connexion des boutons comme il va bien
	connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	
	//Affichage de la fenêtre
	raise();
	show();
}

QString RzxChangePass::newPass() const
{
	return leNewPass->text();
}


void RzxChangePass::accept()
{
	if(network)
		network->changePass(newPass());
	QDialog::accept();
}

void RzxChangePass::analyseNewPass()
{
	btnOK->setEnabled(leNewPass->hasAcceptableInput() && leNewPass->text() == leReenterNewPass->text());
}

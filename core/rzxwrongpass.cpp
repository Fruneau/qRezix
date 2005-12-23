/***************************************************************************
                          rzxwrongpass  -  description
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
#include <RzxThemedIcon>
#include <RzxNetwork>

#include <RzxWrongPass>

#include "ui_rzxwrongpass.h"

///Construit un RzxWrongPass en le liant à un module réseau
/** Cette liaison permet une personnalisation du texte etc...
 */
RzxWrongPass::RzxWrongPass(RzxNetwork *net)
	:network(net)
{
	ui = new Ui::RzxWrongPass();
	ui->setupUi(this);
	if(network)
		setAttribute(Qt::WA_DeleteOnClose);
	RzxThemedIcon icon(Rzx::ICON_OK);
	ui->btnOK->setIcon(icon);
	raise();
	show();
}

///Destruction
RzxWrongPass::~RzxWrongPass()
{
	delete ui;
}

///Retourne le nouveau mot de passe
QString RzxWrongPass::newPass() const
{
	return ui->ledPassword->text();
}

///Si un nouveau mot de passe est défini on l'envoie au module réseau
void RzxWrongPass::accept()
{
	if(network)
		network->usePass(newPass());
	QDialog::accept();
}

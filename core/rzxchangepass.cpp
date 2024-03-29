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
#include "ui_rzxchangepass.h"

///Fen�tre de changement de mot de passe
/** Si un module r�seau est indiqu�, la fen�tre mourra en envoyant les
 * donn�es au module lorsque celle-ci.
 *
 * \sa init
 */
RzxChangePass::RzxChangePass(RzxNetwork *net, const QString &oldpass)
	:network(net)
{
	setAttribute(Qt::WA_QuitOnClose,false);
	init(oldpass);
	if(net)
		setAttribute(Qt::WA_DeleteOnClose);
}

///Cr�e une fen�tre de changement de mot de passe
/** \sa init */
RzxChangePass::RzxChangePass(const QString &oldpass)
	:network(NULL)
{
	setAttribute(Qt::WA_QuitOnClose,false);
	init(oldpass);
}

///Destruction de la fen�tre
RzxChangePass::~RzxChangePass()
{
	delete ui;
}

///Initialise la fen�tre
/** Si l'ancien mot de passe est nul, la fen�tre force la demande d'un nouveau
 * mot de passe, et n'acceptera pas de quitter avant que ce nouveau mot de passe
 * soit d�fini.
 */
void RzxChangePass::init(const QString& oldPass)
{
	if(!oldPass.isNull())
		setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	ui = new Ui::RzxChangePass();
	ui->setupUi(this);
	
	//Application du masque pour �tre sur du formatage du password
	ui->leNewPass->setValidator(new QRegExpValidator(QRegExp(".{6,63}"), this));
	connect(ui->leNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	connect(ui->leReenterNewPass, SIGNAL(textChanged(const QString&)), this, SLOT(analyseNewPass()));
	ui->btnOK->setEnabled(false);
	if(oldPass.isNull())
		ui->btnCancel->hide();
	
	//Rajout des ic�nes aux boutons
	ui->btnOK->setIcon(RzxThemedIcon(Rzx::ICON_OK));
	ui->btnCancel->setIcon(RzxThemedIcon(Rzx::ICON_CANCEL));
	
	//Affichage de la fen�tre
	raise();
	show();
}

///Retourne le nouveau mot de passe
QString RzxChangePass::newPass() const
{
	return ui->leNewPass->text();
}

///Redirection de l'acceptation
void RzxChangePass::accept()
{
	if(network)
		network->changePass(newPass());
	QDialog::accept();
}

///V�rifie que le nouveau mot de passe est valide
/** Un mot de passe est valide si sa taille respecte les conditions (6 caract�res)
 * et si les deux entr�es sont identiques.
 */
void RzxChangePass::analyseNewPass()
{
	ui->btnOK->setEnabled(ui->leNewPass->hasAcceptableInput() && ui->leNewPass->text() == ui->leReenterNewPass->text());
}

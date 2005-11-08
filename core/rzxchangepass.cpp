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

///Fenêtre de changement de mot de passe
/** Si un module réseau est indiqué, la fenêtre mourra en envoyant les
 * données au module lorsque celle-ci.
 *
 * \sa init
 */
RzxChangePass::RzxChangePass(RzxNetwork *net, const QString &oldpass)
	:network(net)
{
	init(oldpass);
	if(net)
		setAttribute(Qt::WA_DeleteOnClose);
}

///Crée une fenêtre de changement de mot de passe
/** \sa init */
RzxChangePass::RzxChangePass(const QString &oldpass)
	:network(NULL)
{
	init(oldpass);
}

///Destruction de la fenêtre
RzxChangePass::~RzxChangePass()
{
}

///Initialise la fenêtre
/** Si l'ancien mot de passe est nul, la fenêtre force la demande d'un nouveau
 * mot de passe, et n'acceptera pas de quitter avant que ce nouveau mot de passe
 * soit défini.
 */
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
	
	//Affichage de la fenêtre
	raise();
	show();
}

///Retourne le nouveau mot de passe
QString RzxChangePass::newPass() const
{
	return leNewPass->text();
}

///Redirection de l'acceptation
void RzxChangePass::accept()
{
	if(network)
		network->changePass(newPass());
	QDialog::accept();
}

///Vérifie que le nouveau mot de passe est valide
/** Un mot de passe est valide si sa taille respecte les conditions (6 caractères)
 * et si les deux entrées sont identiques.
 */
void RzxChangePass::analyseNewPass()
{
	btnOK->setEnabled(leNewPass->hasAcceptableInput() && leNewPass->text() == leReenterNewPass->text());
}

/***************************************************************************
                          rzxrezal.cpp  -  description
                             -------------------
    begin                : Mon Aug 15 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include "rzxrezal.h"

#include "rzxmainuiconfig.h"

///Construction du rezal à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
 */
RzxRezal::RzxRezal(const QString& name, const char* description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version), dock(NULL)
{
}

///Destruction du module
RzxRezal::~RzxRezal()
{
}

///On dit que le module est initialisé correctement par défaut...
/** Ce n'est peut-être pas le plus intelligent, mais au moins
 * c'est pas trop fatiguant...
 */
bool RzxRezal::isInitialised() const
{
	return true;
}

///Ajout d'un flag aux type du module
void RzxRezal::setType(const Type& flag)
{
	m_type |= flag;
}

///Récupération du type du module
const RzxRezal::Type& RzxRezal::type() const
{
	return m_type;
}

///Défini la DockWidget associée au module
/** Contrairement aux autres données du module, le DockWidget
 * est utilisé pour stockée une information qui n'est pas gérée
 * directement par le module mais par QRezix.
 */
void RzxRezal::setDockWidget(QDockWidget *dck)
{
	dock = dck;
	if(dck)
		dck->installEventFilter(widget());
}

///Retourne la DockWidget associée au module
/** \sa setDockWidget */
QDockWidget *RzxRezal::dockWidget() const
{
	return dock;
}

///Met à jour l'affichage
/** L'implémentation par défaut ne fait rien
 */
void RzxRezal::updateLayout()
{
}

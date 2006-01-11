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

///Construction du rezal � partir des diff�rentes informations
/** Les informations � fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit �tre le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin � l'utilisateur
 * 	- num�ro de version major.minor.build-tag
 */
RzxRezal::RzxRezal(const QString& name, const char* description, const Rzx::Version& version)
	:RzxBaseModule(name, description, version), dock(NULL)
{
}

///Destruction du module
RzxRezal::~RzxRezal()
{
}

///On dit que le module est initialis� correctement par d�faut...
/** Ce n'est peut-�tre pas le plus intelligent, mais au moins
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

///R�cup�ration du type du module
const RzxRezal::Type& RzxRezal::type() const
{
	return m_type;
}

///D�fini la DockWidget associ�e au module
/** Contrairement aux autres donn�es du module, le DockWidget
 * est utilis� pour stock�e une information qui n'est pas g�r�e
 * directement par le module mais par QRezix.
 */
void RzxRezal::setDockWidget(QDockWidget *dck)
{
	dock = dck;
	if(dck)
		dck->installEventFilter(widget());
}

///Retourne la DockWidget associ�e au module
/** \sa setDockWidget */
QDockWidget *RzxRezal::dockWidget() const
{
	return dock;
}

///Met � jour l'affichage
/** L'impl�mentation par d�faut ne fait rien
 */
void RzxRezal::updateLayout()
{
}

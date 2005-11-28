/***************************************************************************
                          rzxbasemodule  -  description
                             -------------------
    begin                : Sat Aug 20 2005
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
#include <QList>
#include <QWidget>

#include <RzxBaseModule>

///Construction du module � partir des diff�rentes informations
/** Les informations � fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit �tre le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin � l'utilisateur
 * 	- num�ro de version major.minor.build-tag
 */
RzxBaseModule::RzxBaseModule(const QString& name, const QString& description, const Rzx::Version& version)
	:m_name(name), m_description(description), m_version(version)
{
}

///Destruction du module
RzxBaseModule::~RzxBaseModule()
{
	propClose();
}

///Indique le d�but du chargement du module
void RzxBaseModule::beginLoading() const
{
	Rzx::beginModuleLoading(name());
}

///Indique la fin du chargement du module
void RzxBaseModule::endLoading() const
{
	Rzx::endModuleLoading(name(), isInitialised());
}

///Indique le d�but de la fermeture du module
void RzxBaseModule::beginClosing() const
{
	Rzx::beginModuleClosing(name());
}

///Indique la fin de la fermeture du module
void RzxBaseModule::endClosing() const
{
	Rzx::endModuleClosing(name());
}


///D�fini l'ic�ne du module
/** \sa icon */
void RzxBaseModule::setIcon(const RzxThemedIcon& icon)
{
	m_icon = icon;
}

///D�fini le nom de l'auteur
/** \sa author */
void RzxBaseModule::setAuthor(const QString& author)
{
	m_author = author;
}

///D�fini le copyright
/** \sa copyright */
void RzxBaseModule::setCopyright(const QString &copyright)
{
	m_copyright = copyright;
}

///R�cup�ration du nom du module
const QString& RzxBaseModule::name() const
{
	return m_name;
}

///R�cup�ration de la description du module
const QString& RzxBaseModule::description() const
{
	return m_description;
}

///R�cup�ration de la version du module
const Rzx::Version& RzxBaseModule::version() const
{
	return m_version;
}

///R�cup�ration de la version du module sous forme humainement lisible
/** \ref versionToString
 */
QString RzxBaseModule::versionString() const
{
	return Rzx::versionToString(m_version);
}

///R�cup�ration de l'ic�ne repr�sentant le module
/** L'impl�mentation par d�faut retourne une ic�ne vide.
 *
 * On utilise un RzxThemedIcon dans les modules
 * dans le but de permettre l'utilisation d'une ic�ne qui change
 * en fonction du th�me
 *
 * \sa setIcon
 */
const RzxThemedIcon &RzxBaseModule::icon() const
{
	return m_icon;
}

///Retourne le nom de l'auteur
/** Le nom de l'auteur est une donn�e facultative mais
 * qui peut int�ress� certains concepteurs, en particulier pour
 * fournir une moyen simple de le contacter.
 *
 * Un format commun est "nom <email>"
 *
 * \sa setAuthor
 */
const QString &RzxBaseModule::author() const
{
	return m_author;
}

///Retourne le copyright du module
/** Un copyright peut-�tre d�fini pour indiquer des informations sur
 * le module.
 *
 * Contrairement � ce qu'on peut penser, un copyright n'est pas
 * forc�ment pour les licenses ferm�es. Le GPL peut tr�s bien inclure
 * une license.
 *
 * Le copyright indiqu� n'est donn� qu'� titre indicatif, tous les
 * d�tails doivent �tre donn�s dans la documentation fournie avec
 * le module.
 *
 * \sa setCopyright
 */
const QString &RzxBaseModule::copyright() const
{
	return m_copyright;
}

///Retourne la liste des modules fils
/** Chaque module b�n�ficie de la possibilit� de fournir au programme
 * une liste de sous-modules. La seul chose importante est que ces sous-
 * modules d�rivent de RzxBaseModule.
 *
 * L'impl�mentation par d�faut retourne une liste vide
 */
QList<RzxBaseModule*> RzxBaseModule::childModules() const
{
	return QList<RzxBaseModule*>();
}

///Retourne la liste des fen�tres de configuration du plugin
/** Chaque module b�n�ficie de la possibilit� de fournir plusieurs
 * fen�tre de configuration. Ces fen�tres poss�dent alors un nom chacune
 * qu'il est possible d'obtenir via la fonction \ref propWidgetsName.
 *
 * Toute la gestion de la fen�tre est d�l�gu�e totalement au module...
 * qui sera inform� lors de diff�rentes demandes comme \ref propUpdate ou
 * \ref propDefault.
 *
 * L'impl�mentation par d�faut retourne une liste vide
 */
QList<QWidget*> RzxBaseModule::propWidgets()
{
	return QList<QWidget*>();
}

///Retourne les noms des diff�rentes fen�tres de configuration
/** \sa propWidgets
 *
 * L'impl�mentation par d�faut retourne une liste vide
 */
QStringList RzxBaseModule::propWidgetsName()
{
	return QStringList();
}

///Initialise les donn�es de configuration du module
/** Lorsqu'il faut r�initialiser les valeurs des donn�es de
 * configurations affich�es dans les fen�tres du module.
 *
 * Si le bool�en est true, on initialise aux valeurs par d�faut
 * et non pas aux derni�res valeurs enregistr�e (cf \ref propDefault )
 *
 * \sa propUpdate propWidgets
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxBaseModule::propInit(bool)
{
}

///Mets � jour le module avec le contenu actuel des fen�tres
/** Lorsque l'utilisateur demande la mise � jour des donn�es de la
 * fen�tre de propri�t�s (par OK ou Appliquer), cette fonction est appell�e
 * dans le but de mettre � jour toutes les propri�t�s.
 *
 * \sa propWidgets propWidgetsName propDefault
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxBaseModule::propUpdate()
{
}

///Restauration de la configuration par d�faut
/** Lorsque l'utilisateur demande la restauration des propri�t�s
 * par d�faut, cette fonction fait le travail pour les donn�es
 * li�es au module.
 *
 * \sa propWidgets propWidgetsName propUpdate
 *
 * L'impl�mentation par d�faut appel propInit avec def = true
 */
void RzxBaseModule::propDefault()
{
	propInit(true);
}

///Fermeture de la fen�tre de configuration
/** Lorsqu'on proc�de � la fermeture des 
 * ce slot est appel� dans le but de proc�der � la fermeture des fen�tres
 * du module.
 *
 * L'impl�mentation par d�faut d�truit tous les objets obtenus par
 * \ref propWidgets
 */
void RzxBaseModule::propClose()
{
	QList<QWidget*> widgets;
	qDeleteAll(widgets);
}

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

///Construction du module à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
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

///Indique le début du chargement du module
void RzxBaseModule::beginLoading() const
{
	Rzx::beginModuleLoading(name());
}

///Indique la fin du chargement du module
void RzxBaseModule::endLoading() const
{
	Rzx::endModuleLoading(name(), isInitialised());
}

///Indique le début de la fermeture du module
void RzxBaseModule::beginClosing() const
{
	Rzx::beginModuleClosing(name());
}

///Indique la fin de la fermeture du module
void RzxBaseModule::endClosing() const
{
	Rzx::endModuleClosing(name());
}


///Défini l'icône du module
/** \sa icon */
void RzxBaseModule::setIcon(const RzxThemedIcon& icon)
{
	m_icon = icon;
}

///Défini le nom de l'auteur
/** \sa author */
void RzxBaseModule::setAuthor(const QString& author)
{
	m_author = author;
}

///Défini le copyright
/** \sa copyright */
void RzxBaseModule::setCopyright(const QString &copyright)
{
	m_copyright = copyright;
}

///Récupération du nom du module
const QString& RzxBaseModule::name() const
{
	return m_name;
}

///Récupération de la description du module
const QString& RzxBaseModule::description() const
{
	return m_description;
}

///Récupération de la version du module
const Rzx::Version& RzxBaseModule::version() const
{
	return m_version;
}

///Récupération de la version du module sous forme humainement lisible
/** \ref versionToString
 */
QString RzxBaseModule::versionString() const
{
	return Rzx::versionToString(m_version);
}

///Récupération de l'icône représentant le module
/** L'implémentation par défaut retourne une icône vide.
 *
 * On utilise un RzxThemedIcon dans les modules
 * dans le but de permettre l'utilisation d'une icône qui change
 * en fonction du thème
 *
 * \sa setIcon
 */
const RzxThemedIcon &RzxBaseModule::icon() const
{
	return m_icon;
}

///Retourne le nom de l'auteur
/** Le nom de l'auteur est une donnée facultative mais
 * qui peut intéressé certains concepteurs, en particulier pour
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
/** Un copyright peut-être défini pour indiquer des informations sur
 * le module.
 *
 * Contrairement à ce qu'on peut penser, un copyright n'est pas
 * forcément pour les licenses fermées. Le GPL peut très bien inclure
 * une license.
 *
 * Le copyright indiqué n'est donné qu'à titre indicatif, tous les
 * détails doivent être donnés dans la documentation fournie avec
 * le module.
 *
 * \sa setCopyright
 */
const QString &RzxBaseModule::copyright() const
{
	return m_copyright;
}

///Retourne la liste des modules fils
/** Chaque module bénéficie de la possibilité de fournir au programme
 * une liste de sous-modules. La seul chose importante est que ces sous-
 * modules dérivent de RzxBaseModule.
 *
 * L'implémentation par défaut retourne une liste vide
 */
QList<RzxBaseModule*> RzxBaseModule::childModules() const
{
	return QList<RzxBaseModule*>();
}

///Retourne la liste des fenêtres de configuration du plugin
/** Chaque module bénéficie de la possibilité de fournir plusieurs
 * fenêtre de configuration. Ces fenêtres possèdent alors un nom chacune
 * qu'il est possible d'obtenir via la fonction \ref propWidgetsName.
 *
 * Toute la gestion de la fenêtre est déléguée totalement au module...
 * qui sera informé lors de différentes demandes comme \ref propUpdate ou
 * \ref propDefault.
 *
 * L'implémentation par défaut retourne une liste vide
 */
QList<QWidget*> RzxBaseModule::propWidgets()
{
	return QList<QWidget*>();
}

///Retourne les noms des différentes fenêtres de configuration
/** \sa propWidgets
 *
 * L'implémentation par défaut retourne une liste vide
 */
QStringList RzxBaseModule::propWidgetsName()
{
	return QStringList();
}

///Initialise les données de configuration du module
/** Lorsqu'il faut réinitialiser les valeurs des données de
 * configurations affichées dans les fenêtres du module.
 *
 * Si le booléen est true, on initialise aux valeurs par défaut
 * et non pas aux dernières valeurs enregistrée (cf \ref propDefault )
 *
 * \sa propUpdate propWidgets
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxBaseModule::propInit(bool)
{
}

///Mets à jour le module avec le contenu actuel des fenêtres
/** Lorsque l'utilisateur demande la mise à jour des données de la
 * fenêtre de propriétés (par OK ou Appliquer), cette fonction est appellée
 * dans le but de mettre à jour toutes les propriétés.
 *
 * \sa propWidgets propWidgetsName propDefault
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxBaseModule::propUpdate()
{
}

///Restauration de la configuration par défaut
/** Lorsque l'utilisateur demande la restauration des propriétés
 * par défaut, cette fonction fait le travail pour les données
 * liées au module.
 *
 * \sa propWidgets propWidgetsName propUpdate
 *
 * L'implémentation par défaut appel propInit avec def = true
 */
void RzxBaseModule::propDefault()
{
	propInit(true);
}

///Fermeture de la fenêtre de configuration
/** Lorsqu'on procède à la fermeture des 
 * ce slot est appelé dans le but de procéder à la fermeture des fenêtres
 * du module.
 *
 * L'implémentation par défaut détruit tous les objets obtenus par
 * \ref propWidgets
 */
void RzxBaseModule::propClose()
{
	QList<QWidget*> widgets;
	qDeleteAll(widgets);
}

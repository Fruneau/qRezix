/***************************************************************************
                               rzxmodule.cpp
        Interface à implémenter pour développer un module pour qRezix
                             -------------------
    begin                : Sat Aug 6 2005
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
#include <QRegExp>
#include <QWidget>

#include <RzxGlobal>
#include <RzxModule>


///Construction du module à partir des différentes informations
/** Les informations à fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit être le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin à l'utilisateur
 * 	- numéro de version major.minor.build-tag
 */
RzxModule::RzxModule(const QString& name, const QString& description, int major, int minor, int build, const QString& tag)
	:m_name(name), m_description(description)
{
	m_version.major = major;
	m_version.minor = minor;
	m_version.build = build;
	m_version.tag = tag;
}

///Construction d'un module
/** Fonction surchargée
 */
RzxModule::RzxModule(const QString& name, const QString& description, const Version& version)
	:m_name(name), m_description(description), m_version(version)
{
}

///Construction d'un module à partir d'une chaîne décrivant son nom version
/** Cette fonction surchargée présente l'intérêt de permettre la construction d'un module simplemente à partir
 * de 2 chaînes de caractères. La première décrivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce même module.
 *
 * Le nom est donné de la forme :
 * "nom major.minor.build-tag"
 */
RzxModule::RzxModule(const QString& name, const QString& description)
	:m_description(description)
{
	QRegExp mask("(^.+) (\\d+)\\.(\\d+)\\.(\\d+)-(.*)$");
	if(mask.indexIn(name) == -1)
	{
		m_name = name;
		m_version.major = m_version.minor = m_version.build = 0;
		m_version.tag = QString();
	}
	else
	{
		m_name = mask.cap(1);
		m_version.major = mask.cap(2).toUInt();
		m_version.minor = mask.cap(3).toUInt();
		m_version.build = mask.cap(4).toUInt();
		m_version.tag = mask.cap(5);
	}
}

///Destruction du module
RzxModule::~RzxModule()
{
}

///Indique le début du chargement du module
void RzxModule::beginLoading() const
{
	Rzx::beginModuleLoading(name());
}

///Indique la fin du chargement du module
void RzxModule::endLoading() const
{
	Rzx::endModuleLoading(name(), isInitialised());
}

///Indique le début de la fermeture du module
void RzxModule::beginClosing() const
{
	Rzx::beginModuleClosing(name());
}

///Indique la fin de la fermeture du module
void RzxModule::endClosing() const
{
	Rzx::endModuleClosing(name());
}

///Ajout d'un flag aux type du module
void RzxModule::setType(TypeFlags flag)
{
	m_type |= flag;
}

///Récupération du nom du module
const QString& RzxModule::name() const
{
	return m_name;
}

///Récupération de la description du module
const QString& RzxModule::description() const
{
	return m_description;
}

///Récupération de la version du module
const RzxModule::Version& RzxModule::version() const
{
	return m_version;
}

///Récupération de la version du module sous forme humainement lisible
/** \ref versionToString
 */
QString RzxModule::versionString() const
{
	return versionToString(m_version);
}

///Récupération du type du module
const QFlags<RzxModule::TypeFlags>& RzxModule::type() const
{
	return m_type;
}

///Récupération de l'icône représentant le module
/** L'implémentation par défaut retourne une icône vide.
 *
 * On utilise une réimplémentation de cette fonction dans les modules
 * dans le but de permettre l'utilisation d'une icône qui change
 * en fonction du thème
 */
QIcon RzxModule::icon() const
{
	return QIcon();
}

///Converti la version en numéro de version lisible par l'utilisateur
/** Le numéro de version est de la forme :
 * major.minor.build[-tag],
 * le tag étant ignoré si et seulement si la chaîne est nulle
 */
QString RzxModule::versionToString(const RzxModule::Version& version)
{
	QString value = QString("%1.%2.%3").arg(version.major).arg(version.minor).arg(version.build);
	if(!version.tag.isNull())
		value += "-" + version.tag;
	return value;
}


///Inverse l'état d'affichage de l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantToggleVisible.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::toggleVisible()
{
}

///Affichage de l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantShow.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::show()
{
}

///Cache l'interface graphique
/** Ce slot est appelé pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE émet le signal \ref wantHide.
 *
 * L'implémentation par défaut ne fait rien.
 */
void RzxModule::hide()
{
}

///Retourne la fenêtre principale
/** Cette fonction permet de récupérer la fenêtre principale du module
 * son intérêt de de fournir une fenêtre principale pour le programme
 * pour tous les autres modules. Il est donc fortement conseillé de
 * réimplémenter cette fonction pour tous les modules de type \ref MOD_MAINUI.
 *
 * L'implémentation par défault renvoie NULL
 */
QWidget *RzxModule::mainWindow() const
{
	return NULL;
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
QList<QWidget*> RzxModule::propWidgets()
{
	return QList<QWidget*>();
}

///Retourne les noms des différentes fenêtres de configuration
/** \sa propWidgets
 *
 * L'implémentation par défaut retourne une liste vide
 */
QStringList RzxModule::propWidgetsName()
{
	return QStringList();
}

///Initialise les données de configuration du module
/** Lorsqu'il faut réinitialiser les valeurs des données de
 * configurations affichées dans les fenêtres du module.
 *
 * \sa propDefault propUpdate propWidgets
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::propInit()
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
void RzxModule::propUpdate()
{
}

///Restauration de la configuration par défaut
/** Lorsque l'utilisateur demande la restauration des propriétés
 * par défaut, cette fonction fait le travail pour les données
 * liées au module.
 *
 * \sa propWidgets propWidgetsName propUpdate
 *
 * L'implémentation par défaut ne fait rien
 */
void RzxModule::propDefault()
{
}

///Fermeture de la fenêtre de configuration
/** Lorsqu'on procède à la fermeture des 
 * ce slot est appelé dans le but de procéder à la fermeture des fenêtres
 * du module.
 *
 * L'implémentation par défaut détruit tous les objets obtenus par
 * \ref propWidgets
 */
void RzxModule::propClose()
{
	QList<QWidget*> widgets;
	qDeleteAll(widgets);
}

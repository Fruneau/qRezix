/***************************************************************************
                               rzxmodule.cpp
        Interface � impl�menter pour d�velopper un module pour qRezix
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


///Construction du module � partir des diff�rentes informations
/** Les informations � fournir sont le nom du module et sa version :
 * 	- nom sous la forme d'une QString doit �tre le nom identifiant le module
 * 	- description du plugin qui est juste un texte expliquant le plugin � l'utilisateur
 * 	- num�ro de version major.minor.build-tag
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
/** Fonction surcharg�e
 */
RzxModule::RzxModule(const QString& name, const QString& description, const Version& version)
	:m_name(name), m_description(description), m_version(version)
{
}

///Construction d'un module � partir d'une cha�ne d�crivant son nom version
/** Cette fonction surcharg�e pr�sente l'int�r�t de permettre la construction d'un module simplemente � partir
 * de 2 cha�nes de caract�res. La premi�re d�crivant le module avec son nom et sa version, l'autre donnant une
 * description humainre de ce m�me module.
 *
 * Le nom est donn� de la forme :
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

///Indique le d�but du chargement du module
void RzxModule::beginLoading() const
{
	Rzx::beginModuleLoading(name());
}

///Indique la fin du chargement du module
void RzxModule::endLoading() const
{
	Rzx::endModuleLoading(name(), isInitialised());
}

///Indique le d�but de la fermeture du module
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

///R�cup�ration du nom du module
const QString& RzxModule::name() const
{
	return m_name;
}

///R�cup�ration de la description du module
const QString& RzxModule::description() const
{
	return m_description;
}

///R�cup�ration de la version du module
const RzxModule::Version& RzxModule::version() const
{
	return m_version;
}

///R�cup�ration de la version du module sous forme humainement lisible
/** \ref versionToString
 */
QString RzxModule::versionString() const
{
	return versionToString(m_version);
}

///R�cup�ration du type du module
const QFlags<RzxModule::TypeFlags>& RzxModule::type() const
{
	return m_type;
}

///R�cup�ration de l'ic�ne repr�sentant le module
/** L'impl�mentation par d�faut retourne une ic�ne vide.
 *
 * On utilise une r�impl�mentation de cette fonction dans les modules
 * dans le but de permettre l'utilisation d'une ic�ne qui change
 * en fonction du th�me
 */
QIcon RzxModule::icon() const
{
	return QIcon();
}

///Converti la version en num�ro de version lisible par l'utilisateur
/** Le num�ro de version est de la forme :
 * major.minor.build[-tag],
 * le tag �tant ignor� si et seulement si la cha�ne est nulle
 */
QString RzxModule::versionToString(const RzxModule::Version& version)
{
	QString value = QString("%1.%2.%3").arg(version.major).arg(version.minor).arg(version.build);
	if(!version.tag.isNull())
		value += "-" + version.tag;
	return value;
}


///Inverse l'�tat d'affichage de l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantToggleVisible.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::toggleVisible()
{
}

///Affichage de l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantShow.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::show()
{
}

///Cache l'interface graphique
/** Ce slot est appel� pour tous les modules ayant le flag \ref MOD_MAINUI
 * lorsqu'un module du type \ref MOD_HIDE �met le signal \ref wantHide.
 *
 * L'impl�mentation par d�faut ne fait rien.
 */
void RzxModule::hide()
{
}

///Retourne la fen�tre principale
/** Cette fonction permet de r�cup�rer la fen�tre principale du module
 * son int�r�t de de fournir une fen�tre principale pour le programme
 * pour tous les autres modules. Il est donc fortement conseill� de
 * r�impl�menter cette fonction pour tous les modules de type \ref MOD_MAINUI.
 *
 * L'impl�mentation par d�fault renvoie NULL
 */
QWidget *RzxModule::mainWindow() const
{
	return NULL;
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
QList<QWidget*> RzxModule::propWidgets()
{
	return QList<QWidget*>();
}

///Retourne les noms des diff�rentes fen�tres de configuration
/** \sa propWidgets
 *
 * L'impl�mentation par d�faut retourne une liste vide
 */
QStringList RzxModule::propWidgetsName()
{
	return QStringList();
}

///Initialise les donn�es de configuration du module
/** Lorsqu'il faut r�initialiser les valeurs des donn�es de
 * configurations affich�es dans les fen�tres du module.
 *
 * \sa propDefault propUpdate propWidgets
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::propInit()
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
void RzxModule::propUpdate()
{
}

///Restauration de la configuration par d�faut
/** Lorsque l'utilisateur demande la restauration des propri�t�s
 * par d�faut, cette fonction fait le travail pour les donn�es
 * li�es au module.
 *
 * \sa propWidgets propWidgetsName propUpdate
 *
 * L'impl�mentation par d�faut ne fait rien
 */
void RzxModule::propDefault()
{
}

///Fermeture de la fen�tre de configuration
/** Lorsqu'on proc�de � la fermeture des 
 * ce slot est appel� dans le but de proc�der � la fermeture des fen�tres
 * du module.
 *
 * L'impl�mentation par d�faut d�truit tous les objets obtenus par
 * \ref propWidgets
 */
void RzxModule::propClose()
{
	QList<QWidget*> widgets;
	qDeleteAll(widgets);
}

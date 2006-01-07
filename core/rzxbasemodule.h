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
#ifndef RZXBASECONFIG_H
#define RZXBASECONFIG_H

#include <RzxGlobal>
#include <RzxThemedIcon>

/**
 @author Florent Bruneau
 */

class QLibrary;
class QTreeWidgetItem;

///Structure de base � impl�menter pour r�aliser un type de module pour qRezix
/** Le but de cette classe est de d�finir un type de base qui pourra servir � tous les types
 * de module de qRezix. En effet, on peut envisager non seulement des modules r�seau, graphique...
 * mais aussi des modules de modules. Cette classe fournit donc une impl�mentation simple qui permet
 * de r�aliser rapidement un type de module avec tout ce qui est n�cessaire pour une bonne int�gration
 * � qRezix :
 * 	- nom
 * 	- descriptions
 * 	- nom de l'auteur
 * 	- copyright
 * 	- version
 * 	- ic�ne
 * 	- v�rification d'initalisation
 * 	- fen�tre de propri�t�
 *
 * L'impl�mentation d'un nouveau type de module doit se faire de la forme suivante :
 * \code
 * *** dans mymodule.h ***
 * #include <RzxBaseModule>
 *
 * class MyModuleType: public RzxBaseModule
 * {
		MyModuleType(const QString&, const QString&, const Rzx::Version&);
 * 	...
 * };
 *
 * #define RZX_MYMODULE_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getMyModule, getMyModuleName, getMyModuleVersion, MyModuleType, MODULE)
 * \endcode
 *
 * Cette classe d�fini quasiment une interface m�me si seule isInitialised est abstraite. Dans
 * tous les cas, cette classe n'h�rite pas de QObject ce qui permet une certaine souplesse chez
 * les classes qui en h�rite, en particulier on peut envisager de d�finir sans conflits sur les
 * h�ritages de QObject un type :
 * \code
 * class MyModuleType : public QWidget, public RzxBaseModule
 * \endcode
 *
 * Par conception, cette classe n'est pas con�ue pour �tre h�rit� directement par des modules
 * mais plut�t par des types g�n�raux de modules (RzxModule, RzxRezal...). Les modules � proprement
 * dire, qui h�ritent de RzxModule ou RzxRezal par exemple, doivent faire un appel � RZX_MYMODULE_EXPORT
 * pour fournir le moyen d'acc�s au module. <br>
 * RzxBaseModule fournit �galement des fonctions qui simplifie l'�criture de logs (\ref beginLoading,
 * \ref endLoading, \ref beginClosing et \ref endClosing). Ces fonctions sont con�ues pour n'�tre appel�es que par
 * un module � part enti�re. L'utilisation de ces fonctions dans des classes qui ne sont pas des
 * module mais des sous-entit�s donnera l'affichage de logs inint�ressants. Leur utilisations se
 * fait donc de la forme :
 *
 * Il faut de plus d�finir deux variables pour avant d'exporter un module. Le plus simple est de placer les
 * d�finition en d�but de fichier avant toute inclusion !
 * 	- RZX_MODULE_NAME = le nom du module
 * 	- RZX_MODULE_VERSIONSTR = la version du module... si manquant, la version de qRezix sera utilis�e, mais
 * cela ne refl�tera de fait en rien l'�tat de d�veloppement du module. Donc il est fortement conseill� de ne pas
 * omettre se param�tre sauf pour les modules de base de qRezix
 *
 * Une variable RZX_MODULE_VERSION de type Rzx::version est alors cr��e pour refl�ter le choix de l'utilisateur
 *
 * \code
 * class MyModule : public MyModuleType
 * {
 * 	public:
 * 		MyModule();
 * 		~MyModule();
 * 	...
 * };
 *
 * //Un constructeur bien pratique qui initialise correctement le module
 * //et qui �crit des logs informant l'utilisateur de l'�volution de l'initialisation
 * //Attention � bien utiliser RZX_MODULE_VERSION et RZX_MODULE_NAME !!!
 * MyModule::MyModule():MyModuleType(RZX_MODULE_NAME, QT_TR_NOOPT("Mon super module de test"), RZX_MODULE_VERSION)
 * {
 * 	beginLoading();
 * 	/// initialisation du module... 
 * 	endLoading(); //endLoading utilise isInitialised pour v�rifier que l'initialisation s'est bien pass�e
 * }
 *
 * //Un destructeur qui �crit des logs informant l'utilisateur de la fermeture
 * MyModule::~MyModule()
 * {
 * 	beginClosing();
 * 	/// on vide tout
 * 	endClosing();
 * }
 * \endcode
 */
class RZX_CORE_EXPORT RzxBaseModule
{
	//Propri�t�s du modules
	QString m_name;
	const char* m_description;
	QString m_author;
	QString m_copyright;
	Rzx::Version m_version;
	RzxThemedIcon m_icon;
	QLibrary *m_lib;

	//Chargement du module
	protected:
		RzxBaseModule(const QString&, const char*, const Rzx::Version&);

		void beginLoading() const;
		void endLoading() const;
		void beginClosing() const;
		void endClosing() const;

		void setIcon(const RzxThemedIcon&);
		void setAuthor(const QString&);
		void setCopyright(const QString&);

	public:
		virtual ~RzxBaseModule();
		virtual bool isInitialised() const = 0;
		const QString &name() const;
		QString description() const;
		const Rzx::Version &version() const;
		QString versionString() const;
		const RzxThemedIcon &icon() const;
		const QString& author() const;
		const QString& copyright() const;

		//Gestion des sous-modules
		virtual QList<RzxBaseModule*> childModules() const;
		RzxBaseModule *childModule(const QString& name) const;

		//Gestion de propri�t�s du module
		virtual QList<QWidget*> propWidgets();
		virtual QStringList propWidgetsName();
		virtual void propInit(bool = false);
		virtual void propUpdate();
		virtual void propDefault();
		virtual void propClose();
		virtual void setTreeItem(QTreeWidgetItem *item);

		void setLibrary(QLibrary*);
		QLibrary *library() const;
};

#endif

#undef RZX_BASEMODULE_EXPORT

#ifndef RZX_MODULE_NAME
#	define RZX_MODULE_NAME "Unamed module"
#endif
#ifndef RZX_MODULE_VERSIONSTR
#	define RZX_MODULE_VERSION RzxApplication::version()
#else
#	define RZX_MODULE_VERSION Rzx::versionFromString(RZX_MODULE_VERSIONSTR)
#endif
#ifndef RZX_MODULE_DESCRIPTION
#	define RZX_MODULE_DESCRIPTION "No description"
#endif
#ifndef RZX_MODULE_ICON
#	define RZX_MODULE_ICON RzxThemedIcon()
#endif

///Exportation du module
/** D�fini une fonction qui exporte le module. Cette macro doit �tre appel�e pour tout
 * module. Elle sert � la cr�ation d'une entit� du module en pour les plug-ins.
 *
 * Il est important de d�finir RZX_PLUGIN si le module est un plugin et RZX_BUILTIN si
 * le module est un builtin avant l'inclusion de RzxModule. Cet appel doit
 * imp�rativement �tre r�alis� un dans un fichier sources et non un fichier d'en-t�tes.
 *
 * Pas besoin de s'inqui�ter de la propagation de RZX_PLUGIN et RZX_BUILTIN. Les deux
 * variables sont en effet 'undefined' � la fin de rzxmodule.h sauf si RZX_PRESERVE_MODULETYPE
 * est d�fini.
 */
#ifndef RZX_PLUGIN
#	define RZX_BASEMODULE_EXPORT(modExp, nameExp, versExp, descExp, iconExp, type, MODULE)
#else
#	ifdef Q_OS_WIN
#		define RZX_BASEMODULE_EXPORT(modExp, nameExp, versExp, descExp, iconExp, type, MODULE) \
			extern "C" __declspec(dllexport) type *modExp(void) { return (type*)(new MODULE); } \
			extern "C" __declspec(dllexport) QString nameExp(void) { return RZX_MODULE_NAME; } \
			extern "C" __declspec(dllexport) Rzx::Version versExp(void) { return RZX_MODULE_VERSION; } \
			extern "C" __declspec(dllexport) const char *descExp(void) { return RZX_MODULE_DESCRIPTION; } \
			extern "C" __declspec(dllexport) RzxThemedIcon iconExp(void) { return RZX_MODULE_ICON; }
#	else
#		define RZX_BASEMODULE_EXPORT(modExp, nameExp, versExp, descExp, iconExp, type, MODULE) \
			extern "C" type *modExp(void) { return (type*)(new MODULE); } \
			extern "C" QString nameExp(void) { return RZX_MODULE_NAME; } \
			extern "C" Rzx::Version versExp(void) { return RZX_MODULE_VERSION; } \
			extern "C" const char *descExp(void) { return RZX_MODULE_DESCRIPTION; } \
			extern "C" RzxThemedIcon iconExp(void) { return RZX_MODULE_ICON; }
#	endif
#endif

#ifndef RZX_PRESERVE_MODULETYPE
#	undef RZX_PLUGIN
#	undef RZX_BUILTIN
#endif

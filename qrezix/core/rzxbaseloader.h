/***************************************************************************
                          rzxbaseloader  -  description
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
#ifndef RZXBASELOADER_H
#define RZXBASELOADER_H

#include <QHash>
#include <QList>
#include <QString>
#include <QDir>
#include <QLibrary>

#include <RzxGlobal>
#include <RzxConfig>

/**
 @author Florent Bruneau
 */

///D�fini une interface de base pour charger des modules
/** L'interface qui et partiellement impl�ment�e ici est pr�vue pour permettre un
 * chargement simple. Cette classe est capable de charger simplement un syst�me
 * de modules simple. Il faudra tout de m�me la d�riv�e d�s qu'il y a des types
 * de modules particuliers.
 *
 * Tout comme RzxBaseModule, cette base ne d�rive pas de QObject ce qui permet
 * une grande souplesse d'utilisation.
 *
 * Les fonctions importantes sont :
 * 	- loadPlugins qui charge les plugins de mani�re automatis� en scannant les
 * r�pertoires et en analysant � partir d'un filtre.
 * 	- loadBuiltins dont l'impl�mentation par d�faut ne fait rien, mais qui permet
 * de charger des modules forc�s par l'utilisateur.
 * 	- installModule qui ajoute le module � la liste des modules charg�s apr�s avoir
 * v�rifi� si le module �tait valide
 * 	- linkModules qui permet de cr�er des liens entre les modules. L'impl�mentatio
 * par d�faut est bien s�r vide.
 */
template <class T>
class RzxBaseLoader
{
	typedef T *(*loadTModuleProc)(void);

	QHash<QString, T*> modules;

	public:
		RzxBaseLoader(const QString&, const QString&, const char*);
		RzxBaseLoader();
		virtual ~RzxBaseLoader();

		QList<T*> moduleList() const;

	protected:
		void loadModules(const QString&, const QString&, const char*);
		void closeModules();

		virtual void loadBuiltins();
		virtual void loadPlugins(const QString&, const QString&, const char*);
		virtual bool installModule(T*);
		virtual void linkModules();
};

///Construit un loader en lan�ant automatiquement le chargement
template <class T>
RzxBaseLoader<T>::RzxBaseLoader(const QString& rep, const QString& pattern, const char* symbol)
{
	loadModules(rep, pattern, symbol);
}

///Construit un loader vide
template <class T>
RzxBaseLoader<T>::RzxBaseLoader()
{
}

///D�truit le loader en fermant les modules...
template <class T>
RzxBaseLoader<T>::~RzxBaseLoader()
{
}

///Retourne la liste des modules
template <class T>
QList<T*> RzxBaseLoader<T>::moduleList() const
{
	return modules.values();
}

///Vide la liste des modules
template <class T>
void RzxBaseLoader<T>::closeModules()
{
	qDeleteAll(modules);
}

///Charge les modules
/** Le chargement se fait dans l'ordre :
 * 	- les builtins par un appel � loadBuiltins
 * 	- les plugins par un appel � loadPlugins
 *
 * Une fois les modules charg�, l'appel � linkModules cr�e les liens entre
 * les diff�rents modules.
 */
template <class T>
void RzxBaseLoader<T>::loadModules(const QString& rep, const QString& pattern, const char* symbol)
{
//	Rzx::beginModuleLoading("Built-ins");
	loadBuiltins();
//	Rzx::endModuleLoading("Built-ins");
//	Rzx::beginModuleLoading("Plug-ins");
	loadPlugins(rep, pattern, symbol);
//	Rzx::endModuleLoading("Built-ins");
//	Rzx::beginModuleLoading("Modules interactions");
	linkModules();
//	Rzx::endModuleLoading("Modules interactions");
}

///Charge les builtins
/** Les builtins doivent �tre d�clar�s par l'utilisateur. La fonction doit avoir une forme du genre :
 * \code
 * void MonLoader::loadBuiltins()
 * {
 * 	installModule(new MonPremierBuiltin);
 * 	installModule(new MonDeuxiemeBuiltin);
 * 	...
 * }
 * \endcode
 */
template <class T>
void RzxBaseLoader<T>::loadBuiltins()
{
}

///Charge les plugins
/** Les plug-ins sont recherch� dans le sous r�pertoire des diff�rents directories
 * d�fini pour qRezix : usrDir/rep, libDir/rep, sysDir/rep et currentDir/rep, en
 * cherchant les plug-ins qui r�ponde au \param pattern. Ce pattern est ensuite adapt�
 * en fonction de l'OS. Le pattern doit �tre de la forme "rzx*" qui permet de matcher
 * librzx*.so, librzx*.dylib, rzx*.dll...
 *
 * Une fois le fichier trouv�, on cherche le symbole dans la biblioth�que.
 */
template <class T>
void RzxBaseLoader<T>::loadPlugins(const QString& rep, const QString& pattern, const char* symbol)
{
	QList<QDir> path = RzxConfig::dirList(RzxConfig::DefSearchDirs, rep
#ifndef Q_OS_MAC
		, true
#endif
			);
	foreach(QDir dir, path)
	{
		QStringList filter;
#ifdef WIN32
		filter << pattern;
#else
#ifdef Q_OS_MAC
		filter << "lib" + pattern + ".dylib" << "lib" + pattern + ".bundle";
#endif
		filter << "lib" + pattern + ".so";
#endif

		QStringList plugins = dir.entryList(filter, QDir::Files|QDir::Readable);
		foreach(QString it, plugins)
		{
			loadTModuleProc getModule = (loadTModuleProc)QLibrary::resolve(dir.absoluteFilePath(it), symbol);
			if(getModule)
				installModule(getModule());
		}
	}
}

///Installe le module
/** L'installation du module dans son impl�mentation par d�faut consiste
 * juste � v�rifier la validit� du module et � l'ajouter � la liste des
 * modules charg�s.
 */
template <class T>
bool RzxBaseLoader<T>::installModule(T* module)
{
	if(!module) return false;
	if(!module->isInitialised())
	{
		delete module;
		return false;
	}

	//Recherche d'un module existant d�j� du m�me nom
	T* old = modules[module->name()];
	if(!old || old->version() < module->version())
	{
		delete old;
		modules.insert(module->name(), module);
		return true;
	}
	else
		delete module;
	return false;
}

///Cr�e les liens entre les modules
template <class T>
void RzxBaseLoader<T>::linkModules()
{
}

#endif

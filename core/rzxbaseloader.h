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
#include <RzxInfoMessage>

template <class T>
class RzxLoaderProp;

/**
 @author Florent Bruneau
 */

///Défini une interface de base pour charger des modules
/** L'interface qui et partiellement implémentée ici est prévue pour permettre un
 * chargement simple. Cette classe est capable de charger simplement un système
 * de modules simple. Il faudra tout de même la dérivée dès qu'il y a des types
 * de modules particuliers.
 *
 * Tout comme RzxBaseModule, cette base ne dérive pas de QObject ce qui permet
 * une grande souplesse d'utilisation.
 *
 * Les fonctions importantes sont :
 * 	- loadPlugins qui charge les plugins de manière automatisé en scannant les
 * répertoires et en analysant à partir d'un filtre.
 * 	- loadBuiltins dont l'implémentation par défaut ne fait rien, mais qui permet
 * de charger des modules forcés par l'utilisateur.
 * 	- installModule qui ajoute le module à la liste des modules chargés après avoir
 * vérifié si le module était valide
 * 	- linkModules qui permet de créer des liens entre les modules. L'implémentatio
 * par défaut est bien sûr vide.
 */
template <class T>
class RzxBaseLoader
{
	friend class RzxLoaderProp<T>;

	typedef T *(*loadTModuleProc)(void);
	typedef QString (*loadNameProc)(void);
	typedef Rzx::Version (*loadVersionProc)(void);

	QString rep;
	QString pattern;
	const char *symbol;
	const char *name;
	const char *version;
	QSettings *settings;

	QHash<QString, QString> files;
	QHash<QString, Rzx::Version> versions;
	QHash<QString, T*> modules;

	public:
		RzxBaseLoader(const QString&, const QString&, const char*, const char*, const char*, bool = false);
		virtual ~RzxBaseLoader();

		QList<T*> moduleList() const;
		T* module(const QString& name) const;

	protected:
		void loadModules();
		void closeModules();

		T *resolve(const QString& file);

		virtual void loadBuiltins();
		virtual void loadPlugins();
		bool installModule(const QString&);
		virtual bool installModule(T*);
		virtual void linkModules();
		virtual void relinkModules(T* newT = NULL, T* oldT = NULL);

		void unloadModule(const QString&);
		virtual void unloadModule(T*);

		virtual bool loadModule(const QString&);
		virtual bool reloadModule(const QString&);

		void setSettings(QSettings *);
		void saveState();
		bool shouldLoad(const QString&) const;
};

///Construit un loader en lançant automatiquement le chargement
template <class T>
RzxBaseLoader<T>::RzxBaseLoader(const QString& m_rep, const QString& m_pattern,
		const char* m_symbol, const char* m_name, const char* m_version, bool load)
	:rep(m_rep), pattern(m_pattern), symbol(m_symbol), name(m_name), version(m_version)
{
	settings = NULL;
	if(load)
		loadModules();
}

///Détruit le loader en fermant les modules...
template <class T>
RzxBaseLoader<T>::~RzxBaseLoader()
{
}

///Enregistre l'état des modules...
template <class T>
void RzxBaseLoader<T>::saveState()
{
	if(settings)
	{
		settings->beginGroup("modules");
		QStringList list = modules.keys();
		foreach(QString name, list)
			settings->setValue(name, modules[name]);
		settings->endGroup();
	}
}

///Défini le QSettings qui doit servir à l'enregistrement de l'état du chargement
/** Permet d'enregistrer et de raffraîchir l'état de chargement des
 * modules.
 */
template <class T>
void RzxBaseLoader<T>::setSettings(QSettings *m_settings)
{
	settings = m_settings;
}

template <class T>
bool RzxBaseLoader<T>::shouldLoad(const QString& name) const
{
	bool load = true;
	if(settings)
	{
		settings->beginGroup("modules");
		load = settings->value(name, true).toBool();
		settings->endGroup();
	}
	return load;
}

///Retourne la liste des modules
template <class T>
QList<T*> RzxBaseLoader<T>::moduleList() const
{
	QList<T*> list = modules.values();
	list.removeAll(NULL);
	return list;
}

///Retourne le module correspondant au nom indiqué
template <class T>
T* RzxBaseLoader<T>::module(const QString& name) const
{
	return modules[name];
}

///Vide la liste des modules
template <class T>
void RzxBaseLoader<T>::closeModules()
{
	saveState();
	foreach(T *module, modules)
		unloadModule(module);
}

///Charge les modules
/** Le chargement se fait dans l'ordre :
 * 	- les builtins par un appel à loadBuiltins
 * 	- les plugins par un appel à loadPlugins
 *
 * Une fois les modules chargé, l'appel à linkModules crée les liens entre
 * les différents modules.
 */
template <class T>
void RzxBaseLoader<T>::loadModules()
{
//	Rzx::beginModuleLoading("Built-ins");
	loadBuiltins();
//	Rzx::endModuleLoading("Built-ins");
//	Rzx::beginModuleLoading("Plug-ins");
	loadPlugins();
//	Rzx::endModuleLoading("Built-ins");
//	Rzx::beginModuleLoading("Modules interactions");
	linkModules();
//	Rzx::endModuleLoading("Modules interactions");
}

///Charge les builtins
/** Les builtins doivent être déclarés par l'utilisateur. La fonction doit avoir une forme du genre :
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
/** Les plug-ins sont recherché dans le sous répertoire des différents directories
 * défini pour qRezix : usrDir/rep, libDir/rep, sysDir/rep et currentDir/rep, en
 * cherchant les plug-ins qui réponde au \param pattern. Ce pattern est ensuite adapté
 * en fonction de l'OS. Le pattern doit être de la forme "rzx*" qui permet de matcher
 * librzx*.so, librzx*.dylib, rzx*.dll...
 *
 * Une fois le fichier trouvé, on cherche le symbole dans la bibliothèque.
 */
template <class T>
void RzxBaseLoader<T>::loadPlugins()
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
			installModule(dir.absoluteFilePath(it));
	}
}

///Résoud le symbole
/** Permet de charger une lib. Ne charge la lib que si les données indique qu'il n'y
 * a pas de module qui entre en conflit avec le module donné
 *
 * Retourne NULL en cas d'échec de la résolution du symbole
 */
template <class T>
T *RzxBaseLoader<T>::resolve(const QString& file)
{
	QLibrary *lib = new QLibrary(file);
	loadNameProc getName = (loadNameProc)lib->resolve(name);
	if(!getName)
	{
		lib->unload();
		delete lib;
		return NULL;
	}

	loadVersionProc getVersion = (loadVersionProc)lib->resolve(file, version);
	if(!getVersion)
	{
		lib->unload();
		delete lib;
		return NULL;
	}

	QString moduleName = getName();
	Rzx::Version moduleVersion = getVersion();

	T* module = modules[moduleName];
	bool load = shouldLoad(moduleName);
	if(!load)
	{
		files.insert(moduleName, file);
		modules.insert(moduleName, NULL);
	}
	if(!versions.keys().contains(moduleName) || versions[moduleName] < moduleVersion)
		versions[moduleName] = moduleVersion;
	if(!load || module && module->version() >= moduleVersion)
	{
		lib->unload();
		delete lib;
		return NULL;
	}

	loadTModuleProc getModule = (loadTModuleProc)lib->resolve(file, symbol);
	if(getModule)
	{
		T *module = getModule();
		if(module)
			module->setLibrary(lib);
		return module;
	}
	lib->unload();
	delete lib;
	return NULL;
}

///Installe le module
/** Installe le module contenu dans le fichier spécifié
 *
 * Simple surcharge de la fonction qui suit
 */
template <class T>
bool RzxBaseLoader<T>::installModule(const QString& file)
{
	T *module = resolve(file);
	if(module)
	{
		if(!installModule(module))
			return false;
		files.insert(module->name(), file);
	}
	return false;
}

///Installe le module
/** L'installation du module dans son implémentation par défaut consiste
 * juste à vérifier la validité du module et à l'ajouter à la liste des
 * modules chargés.
 */
template <class T>
bool RzxBaseLoader<T>::installModule(T* module)
{
	if(!module) return false;
	if(!module->isInitialised() || !shouldLoad(module->name()))
	{
		delete module;
		return false;
	}

	//Recherche d'un module existant déjà du même nom
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

///Crée les liens entre les modules
template <class T>
void RzxBaseLoader<T>::linkModules()
{
}

///Crée les liens pour un nouveau module ou détruit ceux d'un ancien
/** Lorsqu'un module est déchargé ou qu'un autre est chargé, il faut
 * reconstruire les liens avec les autres modules. Cette fonction est censée
 * s'en charger.
 *
 * Pour ceci elle peut prendre 2 arguments, vraissemblablement jamais plus d'un à la
 * fois :
 * 	- soit un nouveau module à lié au reste
 * 	- soit un ancien module à libérer de ses liens
 */
template <class T>
void RzxBaseLoader<T>::relinkModules(T*, T*)
{
}

///Décharge un module de la mémoire
/** Ferme le module, et décharge la biblithèque associée si nécessaire
 */
template <class T>
void RzxBaseLoader<T>::unloadModule(const QString& name)
{
	T *module = modules[name];
	if(module)
		unloadModule(module);
}

///Décharge un module de la mémoire
/** Surcharge fournie pour simplifier le travail
 */
template <class T>
void RzxBaseLoader<T>::unloadModule(T *module)
{
	if(!module)
		return;

	relinkModules(NULL, module);
	QLibrary *lib = module->library();
	modules.insert(module->name(), NULL);
	delete module;
	if(lib)
	{
		lib->unload();
		delete lib;
	}
}

///Charge un module à partir de son nom
/** Charge un module à partir du nom.
 *
 * Cette fonction utilise la correspondance nom-fichier pour retrouver
 * la lib à charger
 */
template <class T>
bool RzxBaseLoader<T>::loadModule(const QString& moduleName)
{
	if(modules[moduleName]) return false;

	//On enregistre le changement d'état
	//sinon on se fait jeter plus loin
	//et ça permet aussi de recharger les builtins au reboot du programme
	if(settings)
	{
		settings->beginGroup("modules");
		settings->setValue(moduleName, true);
		settings->endGroup();
	}

	if(files[moduleName].isNull())
	{
		new RzxInfoMessage(RzxConfig::global(),
			"loadBuiltin",
			RzxThemedIcon(Rzx::ICON_PLUGIN),
			RzxInfoMessage::tr("You want to load module %1, but this module is compiled into qRezix as a built-in.<br>"
				"To load this module you have to restart qRezix.").arg(moduleName),
			NULL);
		return false;
	}
	bool ret = installModule(files[moduleName]);
	if(ret)
		relinkModules(modules[moduleName], NULL);
	return ret;
}

///Recharge un module
/** Décharge complètement un module et le recharge dans la foulée
 * La bibliothèque associée est normalement également rechargée, ce qui
 * a pour conséquence de permettre de tester des modifications des modules
 * en cours de route
 */
template <class T>
bool RzxBaseLoader<T>::reloadModule(const QString& moduleName)
{
	unloadModule(moduleName);
	return loadModule(moduleName);
}

#endif

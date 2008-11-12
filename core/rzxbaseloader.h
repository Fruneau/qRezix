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
	friend class RzxLoaderProp<T>;

	typedef T *(*loadTModuleProc)(void);
	typedef QString (*loadNameProc)(void);
	typedef Rzx::Version (*loadVersionProc)(void);
	typedef const char* (*loadDescriptionProc)(void);
	typedef RzxThemedIcon (*loadIconProc)(void);

	QString rep;
	QString pattern;
	const char *symbol;
	const char *name;
	const char *version;
	const char *description;
	const char *icon;
	QSettings *settings;

	QHash<QString, QString> files;
	QHash<QString, Rzx::Version> versions;
	QHash<QString, const char*> descriptions;
	QHash<QString, RzxThemedIcon> icons;
	QHash<QString, T*> modules;
	QStringList toBeReloaded;

	public:
		RzxBaseLoader(const QString&, const QString&, const char*, const char*, const char*, const char*, const char*, bool = false);
		virtual ~RzxBaseLoader();

		QList<T*> moduleList() const;
		T* module(const QString& name) const;

	protected:
		bool addBuiltin(const RzxThemedIcon&, const QString&, const Rzx::Version&, const char*);
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

///Construit un loader en lan�ant automatiquement le chargement
template <class T>
RzxBaseLoader<T>::RzxBaseLoader(const QString& m_rep, const QString& m_pattern,
		const char* m_symbol, const char* m_name, const char* m_version, 
		const char* m_desc, const char* m_icon, bool load)
	:rep(m_rep), pattern(m_pattern), symbol(m_symbol), name(m_name), version(m_version), description(m_desc), icon(m_icon)
{
	settings = NULL;
	if(load)
		loadModules();
}

///D�truit le loader en fermant les modules...
template <class T>
RzxBaseLoader<T>::~RzxBaseLoader()
{
}

///Enregistre l'�tat des modules...
template <class T>
void RzxBaseLoader<T>::saveState()
{
	if(settings)
	{
		settings->beginGroup("modules");
		QStringList list = modules.keys();
		foreach(QString name, list)
			settings->setValue(name, modules[name] || toBeReloaded.contains(name));
		settings->endGroup();
	}
}

///D�fini le QSettings qui doit servir � l'enregistrement de l'�tat du chargement
/** Permet d'enregistrer et de raffra�chir l'�tat de chargement des
 * modules.
 */
template <class T>
void RzxBaseLoader<T>::setSettings(QSettings *m_settings)
{
	settings = m_settings;
}

///Indique si on doit charger ou non le module indiqu�
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

///Insert un built-in dans la liste des modules disponibles
/** Et indique si on doit le charger ou non...
 */
template <class T>
bool RzxBaseLoader<T>::addBuiltin(const RzxThemedIcon& icon, const QString& name, const Rzx::Version& version, const char *desc)
{
	modules.insert(name, NULL);
	versions.insert(name, version);
	descriptions.insert(name, desc);
	icons.insert(name, icon);
	return shouldLoad(name);
}

///Retourne la liste des modules
template <class T>
QList<T*> RzxBaseLoader<T>::moduleList() const
{
	QList<T*> list = modules.values();
	list.removeAll(NULL);
	return list;
}

///Retourne le module correspondant au nom indiqu�
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
 * 	- les builtins par un appel � loadBuiltins
 * 	- les plugins par un appel � loadPlugins
 *
 * Une fois les modules charg�, l'appel � linkModules cr�e les liens entre
 * les diff�rents modules.
 */
template <class T>
void RzxBaseLoader<T>::loadModules()
{
	loadBuiltins();
	loadPlugins();
	linkModules();
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

///R�soud le symbole
/** Permet de charger une lib. Ne charge la lib que si les donn�es indique qu'il n'y
 * a pas de module qui entre en conflit avec le module donn�
 *
 * Retourne NULL en cas d'�chec de la r�solution du symbole
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

	loadDescriptionProc getDescription = (loadDescriptionProc)lib->resolve(file, description);
	if(!getVersion)
	{
		lib->unload();
		delete lib;
		return NULL;
	}

	loadIconProc getIcon = (loadIconProc)lib->resolve(file, icon);
	if(!getVersion)
	{
		lib->unload();
		delete lib;
		return NULL;
	}

	const QString moduleName = getName();
	const Rzx::Version moduleVersion = getVersion();
	const RzxThemedIcon moduleIcon = getIcon();
	const char* moduleDescription = getDescription();

	T* module = modules[moduleName];
	bool load = shouldLoad(moduleName);
	if(!load)
	{
		files.insert(moduleName, file);
		modules.insert(moduleName, NULL);
	}
	if(!versions.keys().contains(moduleName) || versions[moduleName] < moduleVersion)
	{
		versions[moduleName] = moduleVersion;
		icons[moduleName] = moduleIcon;
		descriptions[moduleName] = moduleDescription;
	}
	if(!load || (module && module->version() >= moduleVersion))
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
/** Installe le module contenu dans le fichier sp�cifi�
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
		return true;
	}
	return false;
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
	if(!module->isInitialised() || !shouldLoad(module->name()))
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

///Cr�e les liens pour un nouveau module ou d�truit ceux d'un ancien
/** Lorsqu'un module est d�charg� ou qu'un autre est charg�, il faut
 * reconstruire les liens avec les autres modules. Cette fonction est cens�e
 * s'en charger.
 *
 * Pour ceci elle peut prendre 2 arguments, vraissemblablement jamais plus d'un � la
 * fois :
 * 	- soit un nouveau module � li� au reste
 * 	- soit un ancien module � lib�rer de ses liens
 */
template <class T>
void RzxBaseLoader<T>::relinkModules(T*, T*)
{
}

///D�charge un module de la m�moire
/** Ferme le module, et d�charge la biblith�que associ�e si n�cessaire
 */
template <class T>
void RzxBaseLoader<T>::unloadModule(const QString& name)
{
	T *module = modules[name];
	if(module)
		unloadModule(module);
}

///D�charge un module de la m�moire
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
	toBeReloaded.removeAll(module->name());
	delete module;
	if(lib)
	{
		lib->unload();
		delete lib;
	}
}

///Charge un module � partir de son nom
/** Charge un module � partir du nom.
 *
 * Cette fonction utilise la correspondance nom-fichier pour retrouver
 * la lib � charger
 */
template <class T>
bool RzxBaseLoader<T>::loadModule(const QString& moduleName)
{
	if(modules[moduleName]) return false;

	//On enregistre le changement d'�tat
	//sinon on se fait jeter plus loin
	//et �a permet aussi de recharger les builtins au reboot du programme
	if(settings)
	{
		settings->beginGroup("modules");
		settings->setValue(moduleName, true);
		settings->endGroup();
	}

	if(files[moduleName].isNull())
	{
		toBeReloaded << moduleName;
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
/** D�charge compl�tement un module et le recharge dans la foul�e
 * La biblioth�que associ�e est normalement �galement recharg�e, ce qui
 * a pour cons�quence de permettre de tester des modifications des modules
 * en cours de route
 */
template <class T>
bool RzxBaseLoader<T>::reloadModule(const QString& moduleName)
{
	unloadModule(moduleName);
	return loadModule(moduleName);
}

#endif

/***************************************************************************
                     rzxabstractconfig.h  -  description
                             -------------------
    begin                : Fri Aug 12 2005
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
#ifndef RZXABSTRACTCONFIG_H
#define RZXABSTRACTCONFIG_H

#include <QSettings>
#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QList>

#include <RzxGlobal>

class RzxBaseModule;

/**
 @author Florent Bruneau
 */

///Classe de base pour stocker des informations propres à un module
/** Cette classe (dont l'abstraction est toute ambigüe), n'a pour seul
 * but que de permettre de fournir un système simple aux modules pour
 * stocker proprement leurs données de configuration.
 */
class RZX_CORE_EXPORT RzxAbstractConfig: public QSettings
{
	Q_OBJECT

	protected:
		RzxBaseModule *module;

	public:
		RzxAbstractConfig(RzxBaseModule * = NULL);
		~RzxAbstractConfig();

		void flush();
		void saveWidget(const QString&, QWidget*);
		void restoreWidget(const QString&, QWidget*, const QPoint&, const QSize&, bool def = false);
};

///Surcharge pour être plus naturel
inline void RzxAbstractConfig::flush()
{
	sync();
}

///Macro générale pour définir et renvoyer un été d'un certain type
#define RZX_PROP(type, name, read, write, default) \
	static type read(bool def = false) { return def?default:global()->value(name, default).value<type>(); } \
	static void write(const type& value = default) { global()->setValue(name, value); }
#define RZX_ENUMPROP(type, name, read, write, default) \
	static type read(bool def = false) { return def?default:(type)global()->value(name, default).toInt(); } \
	static void write(const type& value = default) { global()->setValue(name, value); }
#define RZX_LISTPROP(type, name, read, write) \
	static QList<type> read(bool def = false) { \
		if(def) return QList<type>(); \
		QList<QVariant> list; \
		list = global()->value(name, list).toList(); \
		QList<type> ret; \
		foreach(const QVariant &v, list) ret << v.value<type>(); \
		return ret; \
	} \
	static void write(const QList<type>& value = QList<type>()) { \
		QList<QVariant> list; \
		foreach(const type &t, value) list << t; \
		global()->setValue(name, list); \
	}

///Macro qui déclare une fonction de lecture et une fonction d'écriture pour le une propriété.
/** les fonctions ont les prototype suivants :
 * \code
 * static type read(bool def = false, const type& defValue = default);
 * static void write(const type& value = default);
 * \endcode
 */
#define RZX_PROP_DECLARE(type, read, write, default) \
	static type read(bool def = false, const type& defValue = default); \
	static void write(const type& value = default);

///Macro permettant de définir et de renvoyer un objet
#define RZX_STRINGPROP(name, read, write, default) \
	RZX_PROP(QString, name, read, write, default)
#define RZX_INTPROP(name, read, write, default) \
	RZX_PROP(int, name, read, write, default)
#define RZX_UINTPROP(name, read, write, default) \
	RZX_PROP(uint, name, read, write, default)
#define RZX_BOOLPROP(name, read, write, default) \
	RZX_PROP(bool, name, read, write, default)
#define RZX_STRINGLISTPROP(name, read, write, default) \
	RZX_PROP(QStringList, name, read, write, default)
#define RZX_RGBPROP(name, read, write, default) \
	static QColor read(bool def = false) { \
		QColor ret; \
		ret.setRgb(def?default:global()->value(name, default).toUInt()); \
		return ret; \
	} \
	static void write(unsigned int value = default) { global()->setValue(name, value); }

///Macro définissant la fonction statique d'enregistrement d'une fenêtre particulière
#define RZX_WIDGETPROP(name, read, write, pos, size) \
	static void read(QWidget *widget, bool def = false) { global()->restoreWidget(name, widget, pos, size, def); } \
	static void write(QWidget *widget) { global()->saveWidget(name, widget); }

///Macro qui permet d'initialiser un objet de configuration
/** \ref RZX_CONFIG
 */
#define RZX_CONFIG_INIT(myclass) \
	RZX_GLOBAL_INIT(myclass)

///Macro définissant les base d'une classe de configuration
/** Cette macro doit être appelée dans la partie privée d'une classe
 * de la forme :
 *
 * \code
 * *** Dans rzxmyconfig.h ***
 * class RzxMyConfig: public RzxAbstractConfig
 * {
 * 	RZX_CONFIG(RzxMyConfig)
 *
 * 	public:
 * 		RZX_STRINGPROP("bidule", bidule, setBidule, QString())
 * 		...
 * };
 *
 * *** Dans n'importe quelle fichier .cpp ***
 * RZX_CONFIG_INIT(RzxMyConfig)
 * \endcode
 * 
 * Cette macro défini les constructeurs, destructeur et une fonction global() statique
 * renvoyant un object du type RzxMyConfig.
 *
 * Si RZX_CONFIG_EXPANDED est utilisé, alors la construction de l'objet nécessite
 * la définition d'une fonction init() qui permet de définir une construction
 * personnalisée et une fonction destroy() qui permet une destruction personnalisée.
 *
 * Ces macros inclues RZX_GLOBAL pour définir un objet global
 */
#define RZX_CONFIG(myclass) \
	RZX_GLOBAL(myclass) \
	public: \
		myclass(RzxBaseModule *module = NULL):RzxAbstractConfig(module) { if(!object) object = this; } \
		~myclass() { RZX_GLOBAL_CLOSE } \

/** \ref RZX_CONFIG */
#define RZX_CONFIG_EXPANDED(myclass) \
	RZX_GLOBAL(myclass) \
	public: \
		myclass(RzxBaseModule *module = NULL):RzxAbstractConfig(module) { if(!object) object = this; init(); } \
		~myclass() { destroy(); RZX_GLOBAL_CLOSE } \
	protected: \
		void init(); \
		void destroy(); \
	private:

#endif

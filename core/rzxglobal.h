/***************************************************************************
                          rzxglobal  -  d�finition globales
                             -------------------
    begin                : Fri Jul 22 2005
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
#ifndef RZXGLOBAL_H
#define RZXGLOBAL_H

#include <QtGlobal>
#include <QtDebug>
#include <QString>

#include <stdio.h>

#include "defaults.h"

///Pour exporter
#ifndef RZX_CORE_EXPORT
#	ifdef RZX_BUILD_CORE
#		define RZX_CORE_EXPORT Q_DECL_EXPORT
#	else
#		define RZX_CORE_EXPORT
#	endif
#endif

///Namespace pour encadrer les enums communs � la plupart des interfaces
namespace Rzx
{
	enum SysEx {
		SYSEX_UNKNOWN = 0, /**< Syst�me d'exploitation inconnu */
		SYSEX_WIN9X = 1, /**< Windows 95, 98 ou Millenium */
		SYSEX_WINNT = 2, /**< Windows NT, 2000, XP ou 2003 server */
		SYSEX_LINUX = 3, /**< Linux, ou tout autre Unix non BSD */
		SYSEX_MACOS = 4, /**< MacOS jusqu'� 9 */
		SYSEX_MACOSX = 5, /**< MacOS X */
		SYSEX_BSD = 6 /**< BSD diff�rentre de MacOS */
	};

	enum Promal {
		PROMAL_UNK = 0, /**< Promotion inconnue */
		PROMAL_ORANGE = 1, /**< Promotion oranje, ou bipromo */
		PROMAL_ROUJE = 2, /**< Promo rouje */
		PROMAL_JONE = 3 /**< Chic � la J�ne */
	};
	
	enum ConnectionState {
		STATE_DISCONNECTED = 0, /**< Non connect� */
		STATE_HERE = 1, /**< Connect� et pr�sent */
		STATE_AWAY = 2, /**< Connect� et sur r�pondeur */
		STATE_REFUSE = 3 /**< Connect� et sur r�pondeur qui refuse les messages */
	};	
	
	///D�finition des capabilities suppl�mentaires connues
	enum Capabilities {
		CAP_NONE = 0, /**< Aucune extension connue */
		CAP_ON = 1, /**< Supporte le syst�me d'extension de fonctionnalit� */
		CAP_CHAT = 2, /**< Supporte le chat sur le protocole DCC du xNet */
		CAP_XPLO = 4 /**< Supporte le protocole Xplo */
	};

	///Identifiant des ic�nes
	enum Icon {
		ICON_ACTION = 0,
		ICON_APPLY = 1,
		ICON_OK = 2,
		ICON_CANCEL = 3,
		ICON_AWAY = 4,
		ICON_HERE = 5,
		ICON_CHAT = 6,
		ICON_FTP = 7,
		ICON_HTTP = 8,
		ICON_SAMBA = 9,
		ICON_NEWS = 10,
		ICON_HOTLINE = 11,
		ICON_NOFTP = 12,
		ICON_NOHTTP = 13,
		ICON_NOSAMBA = 14,
		ICON_NONEWS = 15,
		ICON_NOHOTLINE = 16,
		ICON_BAN = 17,
		ICON_UNBAN = 18,
		ICON_FAVORITE = 19,
		ICON_NOTFAVORITE = 20,
		ICON_SAMEGATEWAY = 21,
		ICON_OTHERGATEWAY = 22,
		ICON_COLUMN = 23,
		ICON_SEARCH = 24,
		ICON_PREFERENCES = 25,
		ICON_PLUGIN = 26,
		ICON_SOUNDON = 27,
		ICON_SOUNDOFF = 28,
		ICON_PROPRIETES = 29,
		ICON_HISTORIQUE = 30,
		ICON_JONE = 31,
		ICON_ROUJE = 32,
		ICON_ORANJE = 33,
		ICON_QUIT = 34,
		ICON_ON = 35,
		ICON_OFF = 36,
		ICON_SEND = 37,
		ICON_SYSTRAYHERE = 38,
		ICON_SYSTRAYAWAY = 39,
		ICON_LAYOUT = 40,
		ICON_NETWORK = 41,
		ICON_OS0 = 42,
		ICON_OS0_LARGE = 43,
		ICON_OS1 = 44,
		ICON_OS1_LARGE = 45,
		ICON_OS2 = 46,
		ICON_OS2_LARGE = 47,
		ICON_OS3 = 48,
		ICON_OS3_LARGE = 49,
		ICON_OS4 = 50,
		ICON_OS4_LARGE = 41,
		ICON_OS5 = 52,
		ICON_OS5_LARGE = 53,
		ICON_OS6 = 54,
		ICON_OS6_LARGE = 55,
		ICON_PRINTER = 56,
		ICON_NOPRINTER = 57,
		ICON_NUMBER = 58
	};

	///Types de messages auxquel on peut s'attendre dans un protocole de chat
	enum ChatMessageType {
		Chat = 0,
		Responder = 1,
		Ping = 2,
		Pong = 3,
		Typing = 4,
		StopTyping = 5,
		InfoMessage = 6,
		Closed = 7
	};

	///D�fini une version
	/** La version est simplement d�finie de la forme
	 * major.minor.build-tag
	 *
	 * comme par exemple 1.7.0-svn
	 */
	struct Version
	{
		uint major;
		uint minor;
		uint build;
		QString tag;
	};
	
	///Flags pour retrouver les parties du num�ro de version
	enum VersionPartFlags
	{
		NoVersion = 0,
		MajorVersion = 1,
		MinorVersion = 2,
		BuildVersion = 4,
		TagVersion = 8,
		ShortVersion = MajorVersion | MinorVersion,
		LongVersion = ShortVersion | BuildVersion,
		CompleteVersion = LongVersion | TagVersion
	};
	Q_DECLARE_FLAGS(VersionParts, VersionPartFlags)
	RZX_CORE_EXPORT QString versionToString(const Rzx::Version&, VersionParts = CompleteVersion);

	///D�finition de la fonction d'output surcharg�e
	RZX_CORE_EXPORT void installMsgHandler();
	RZX_CORE_EXPORT void closeMsgHandler();
	RZX_CORE_EXPORT void useOutputFile(FILE *file);

	//Chargement d'un module
	RZX_CORE_EXPORT void beginModuleLoading(const QString&);
	RZX_CORE_EXPORT void endModuleLoading(const QString&, bool success = true);

	//Fermeture d'un module
	RZX_CORE_EXPORT void beginModuleClosing(const QString&);
	RZX_CORE_EXPORT void endModuleClosing(const QString&);

	///Pour trier case unsensitive
	RZX_CORE_EXPORT bool caseInsensitiveLessThan(const QString&, const QString&);

	///Comparaison de versions
	RZX_CORE_EXPORT bool operator==(const Version&, const Version&);
	RZX_CORE_EXPORT bool operator<(const Version&, const Version&);
};

///D�fini les fonctions n�cessaire pour g�n�rer un objet global
/** Cette macro n'a pour but que de fournir un syst�me homog�ne pour
 * d�finir des objets globaux pour une classe
 *
 * Il faut placer cet macro dans la partie priv�e de la d�claration
 * de la classe :
 * \code
 * *** dans myclass.h ***
 * class MyClass
 * {
 * 	RZX_GLOBAL(MyClass)
 * 	...
 * };
 *
 * *** dans myclass.cpp ***
 * RZX_GLOBAL_INIT(MyClass)
 *
 * MyClass::~MyClass()
 * {
 * 	RZX_GLOBAL_CLOSE
 * }
 * \endcode
 * \sa RZX_GLOBAL_INIT
 */
#ifndef Q_OS_WIN
#define RZX_GLOBAL(type) \
	public: \
		static type *global() { if(!object) object = new type; return object; } \
	private: \
		static type *object;
#else
#define RZX_GLOBAL(type) \
	public: \
		static type *global(); \
	private: \
		static type *object;
#endif

///Initialise la d�finition globale
/** \sa RZX_GLOBAL */
#ifndef Q_OS_WIN
#define RZX_GLOBAL_INIT(type) type *type::object = NULL;
#else
#define RZX_GLOBAL_INIT(type) type *type::object = NULL; \
	type *type::global() { if(!object) object = new type; return object; }
#endif

 ///Vire le pointeur vers l'objet global
/** \sa RZX_GLOBAL */
#define RZX_GLOBAL_CLOSE if(object == this) object = NULL;

///Little Endian - Big Endian
#ifndef __BYTE_ORDER
#ifdef BYTE_ORDER
#define __BYTE_ORDER BYTE_ORDER
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BIG_ENDIAN BIG_ENDIAN
#else
#define __BIG_ENDIAN 0
#define __LITTLE_ENDIAN 1
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif
#endif

#endif
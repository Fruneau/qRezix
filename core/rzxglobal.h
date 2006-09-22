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
		ICON_ACTION = 0,				/**< Ic�ne Action */
		ICON_APPLY = 1,				/**< Ic�ne Appliquer */
		ICON_OK = 2,					/**< Ic�ne OK */
		ICON_CANCEL = 3,				/**< Ic�ne Annuler */
		ICON_AWAY = 4,					/**< Ic�ne Absent */
		ICON_HERE = 5,					/**< Ic�ne Pr�sent */
		ICON_CHAT = 6,					/**< Ic�ne Discussion */
		ICON_FTP = 7,					/**< Ic�ne Ftp */
		ICON_HTTP = 8,					/**< Ic�ne Http */
		ICON_SAMBA = 9,				/**< Ic�ne Samba */
		ICON_NEWS = 10,				/**< Ic�ne News */
		ICON_HOTLINE = 11,			/**< Ic�ne Hotline */
		ICON_NOFTP = 12,				/**< Ic�ne Pas de Ftp */
		ICON_NOHTTP = 13,				/**< Ic�ne Pas de Http */
		ICON_NOSAMBA = 14,			/**< Ic�ne Pas de Samba */
		ICON_NONEWS = 15,				/**< Ic�ne Pas de News */
		ICON_NOHOTLINE = 16,			/**< Ic�ne Pas de Hotline */
		ICON_BAN = 17,					/**< Ic�ne Banni */
		ICON_UNBAN = 18,				/**< Ic�ne D�banni */
		ICON_FAVORITE = 19,			/**< Ic�ne Favoris */
		ICON_NOTFAVORITE = 20,		/**< Ic�ne Retirer des Favoris */
		ICON_SAMEGATEWAY = 21,		/**< Ic�ne M�me passerelle */
		ICON_OTHERGATEWAY = 22,		/**< Ic�ne Autre passerelle */
		ICON_COLUMN = 23,				/**< Ic�ne Ajuster les colonnes */
		ICON_SEARCH = 24,				/**< Ic�ne Recherche */
		ICON_PREFERENCES = 25,		/**< Ic�ne Pr�f�rences */
		ICON_PLUGIN = 26,				/**< Ic�ne Plugins - Modules */
		ICON_SOUNDON = 27,			/**< Ic�ne Son activ� */
		ICON_SOUNDOFF = 28,			/**< Ic�ne Son d�sactiv� */
		ICON_PROPRIETES = 29,		/**< Ic�ne Propri�t�s de la personne */
		ICON_HISTORIQUE = 30,		/**< Ic�ne Historique */
		ICON_JONE = 31,				/**< Ic�ne Promotion J�ne (Chic � la j�ne) */
		ICON_ROUJE = 32,				/**< Ic�ne Promotion Rouje */
		ICON_ORANJE = 33,				/**< Ic�ne Promotion Oranje */
		ICON_QUIT = 34,				/**< Ic�ne Quitter */
		ICON_ON = 35,					/**< Ic�ne Activ�, Allum�, Connect� */
		ICON_OFF = 36,					/**< Ic�ne D�sactiv�, Eteind, D�connect� */
		ICON_SEND = 37,				/**< Ic�ne Envoyer */
		ICON_SYSTRAYHERE = 38,		/**< Ic�ne Trayicon en cas de pr�sence */
		ICON_SYSTRAYAWAY = 39,		/**< Ic�ne Trayicon en cas d'absence */
		ICON_LAYOUT = 40,				/**< Ic�ne Affichage */
		ICON_NETWORK = 41,			/**< Ic�ne R�seau */
		ICON_OS0 = 42,					/**< Ic�ne OS Iconnu 32x32 */
		ICON_OS0_LARGE = 43,			/**< Ic�ne OS Iconnu 64x64 */
		ICON_OS1 = 44,					/**< Ic�ne OS Win9x 32x32 */
		ICON_OS1_LARGE = 45,			/**< Ic�ne OS Win9x 64x64 */
		ICON_OS2 = 46,					/**< Ic�ne OS WinNT 32x32 */
		ICON_OS2_LARGE = 47,			/**< Ic�ne OS WinNT 64x64 */
		ICON_OS3 = 48,					/**< Ic�ne OS Linux 32x32 */
		ICON_OS3_LARGE = 49,			/**< Ic�ne OS Linux 64x64 */
		ICON_OS4 = 50,					/**< Ic�ne OS MacOS9 32x32 */
		ICON_OS4_LARGE = 41,			/**< Ic�ne OS MacOS9 64x64 */
		ICON_OS5 = 52,					/**< Ic�ne OS MacOSX 32x32 */
		ICON_OS5_LARGE = 53,			/**< Ic�ne OS MacOSX 64x64 */
		ICON_OS6 = 54,					/**< Ic�ne OS BSD 32x32 */
		ICON_OS6_LARGE = 55,			/**< Ic�ne OS BSD 64x64 */
		ICON_PRINTER = 56,			/**< Ic�ne Imprimante */
		ICON_NOPRINTER = 57,			/**< Ic�ne Pas d'imprimante */
		ICON_LOAD = 58,				/**< Ic�ne Charger */
		ICON_UNLOAD = 59,				/**< Ic�ne D�charger */
		ICON_RELOAD = 60,				/**< Ic�ne Recharger */
		ICON_MAIL = 61,				/**< Ic�ne pour les mails */
		ICON_PHONE = 62,				/**< Ic�ne pour le t�l�phone */
		ICON_FILE = 63,				/**< Ic�ne pour les fichiers */
		ICON_SYSTRAYDISCON = 64,	/**< Ic�ne Trayicon en cas de d�connexion des serveurs */
		ICON_FILETRANSFER = 65,		/**< Icone du trandfert de fichier */
		ICON_REFUSE = 66,			/**< Icone de refus du transfert */
		ICON_NUMBER = 67				/**< Nombre d'ic�ne... */
	};

	///Types de messages auxquel on peut s'attendre dans un protocole de chat
	enum ChatMessageType {
		Chat = 0,			/**< Message de discussion */
		Responder = 1,		/**< Message de r�pondeur */
		Ping = 2,			/**< Demande de ping */
		Pong = 3,			/**< R�ponse � un ping (avec le temps en ms)*/
		Typing = 4,			/**< Informe qu'un message est en cours d'�dition */
		StopTyping = 5,	/**< Informe que l'�dition du message a �t� annul�e */
		InfoMessage = 6,	/**< Envoie d'un message d'information */
		Closed = 7,			/**< Notification de la fin de la discussion */
		File = 8			/**< Notification lors du transfert de fichier */
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
		NoVersion = 0,			/**< Pas de num�ro de version */
		MajorVersion = 1,		/**< Demande d'affichage du num�ro de version majeure */
		MinorVersion = 2,		/**< Demande d'affichage du num�ro de version mineure */
		BuildVersion = 4,		/**< Demande d'affichage du num�ro de build */
		TagVersion = 8,		/**< Demande d'affichage du tag */
		ShortVersion = MajorVersion | MinorVersion,	/**< Demande d'affichage d'un num�ro de version court major.minor */
		LongVersion = ShortVersion | BuildVersion,	/**< Demande d'affichage d'un num�ro de version long major.minor.build */
		CompleteVersion = LongVersion | TagVersion	/**< Demande d'affichage d'un num�ro de version compl�te major.minor.build-tag */
	};
	Q_DECLARE_FLAGS(VersionParts, VersionPartFlags)
	RZX_CORE_EXPORT QString versionToString(const Rzx::Version&, VersionParts = CompleteVersion);
	RZX_CORE_EXPORT Rzx::Version versionFromString(const QString&);

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
	RZX_CORE_EXPORT bool operator>=(const Version&, const Version&);

	///Conversion des diff�rents types en ic�ne
	RZX_CORE_EXPORT Icon toIcon(SysEx, bool = true);
	RZX_CORE_EXPORT Icon toIcon(Promal);
	RZX_CORE_EXPORT Icon toIcon(ConnectionState);
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

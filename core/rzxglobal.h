/***************************************************************************
                          rzxglobal  -  définition globales
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

///Namespace pour encadrer les enums communs à la plupart des interfaces
namespace Rzx
{
	enum SysEx {
		SYSEX_UNKNOWN = 0, /**< Système d'exploitation inconnu */
		SYSEX_WIN9X = 1, /**< Windows 95, 98 ou Millenium */
		SYSEX_WINNT = 2, /**< Windows NT, 2000, XP ou 2003 server */
		SYSEX_LINUX = 3, /**< Linux, ou tout autre Unix non BSD */
		SYSEX_MACOS = 4, /**< MacOS jusqu'à 9 */
		SYSEX_MACOSX = 5, /**< MacOS X */
		SYSEX_BSD = 6 /**< BSD différentre de MacOS */
	};

	enum Promal {
		PROMAL_UNK = 0, /**< Promotion inconnue */
		PROMAL_ORANGE = 1, /**< Promotion oranje, ou bipromo */
		PROMAL_ROUJE = 2, /**< Promo rouje */
		PROMAL_JONE = 3 /**< Chic à la Jône */
	};
	
	enum ConnectionState {
		STATE_DISCONNECTED = 0, /**< Non connecté */
		STATE_HERE = 1, /**< Connecté et présent */
		STATE_AWAY = 2, /**< Connecté et sur répondeur */
		STATE_REFUSE = 3 /**< Connecté et sur répondeur qui refuse les messages */
	};	
	
	///Définition des capabilities supplémentaires connues
	enum Capabilities {
		CAP_NONE = 0, /**< Aucune extension connue */
		CAP_ON = 1, /**< Supporte le système d'extension de fonctionnalité */
		CAP_CHAT = 2, /**< Supporte le chat sur le protocole DCC du xNet */
		CAP_XPLO = 4 /**< Supporte le protocole Xplo */
	};

	///Identifiant des icônes
	enum Icon {
		ICON_ACTION = 0,				/**< Icône Action */
		ICON_APPLY = 1,				/**< Icône Appliquer */
		ICON_OK = 2,					/**< Icône OK */
		ICON_CANCEL = 3,				/**< Icône Annuler */
		ICON_AWAY = 4,					/**< Icône Absent */
		ICON_HERE = 5,					/**< Icône Présent */
		ICON_CHAT = 6,					/**< Icône Discussion */
		ICON_FTP = 7,					/**< Icône Ftp */
		ICON_HTTP = 8,					/**< Icône Http */
		ICON_SAMBA = 9,				/**< Icône Samba */
		ICON_NEWS = 10,				/**< Icône News */
		ICON_HOTLINE = 11,			/**< Icône Hotline */
		ICON_NOFTP = 12,				/**< Icône Pas de Ftp */
		ICON_NOHTTP = 13,				/**< Icône Pas de Http */
		ICON_NOSAMBA = 14,			/**< Icône Pas de Samba */
		ICON_NONEWS = 15,				/**< Icône Pas de News */
		ICON_NOHOTLINE = 16,			/**< Icône Pas de Hotline */
		ICON_BAN = 17,					/**< Icône Banni */
		ICON_UNBAN = 18,				/**< Icône Débanni */
		ICON_FAVORITE = 19,			/**< Icône Favoris */
		ICON_NOTFAVORITE = 20,		/**< Icône Retirer des Favoris */
		ICON_SAMEGATEWAY = 21,		/**< Icône Même passerelle */
		ICON_OTHERGATEWAY = 22,		/**< Icône Autre passerelle */
		ICON_COLUMN = 23,				/**< Icône Ajuster les colonnes */
		ICON_SEARCH = 24,				/**< Icône Recherche */
		ICON_PREFERENCES = 25,		/**< Icône Préférences */
		ICON_PLUGIN = 26,				/**< Icône Plugins - Modules */
		ICON_SOUNDON = 27,			/**< Icône Son activé */
		ICON_SOUNDOFF = 28,			/**< Icône Son désactivé */
		ICON_PROPRIETES = 29,		/**< Icône Propriétés de la personne */
		ICON_HISTORIQUE = 30,		/**< Icône Historique */
		ICON_JONE = 31,				/**< Icône Promotion Jône (Chic à la jône) */
		ICON_ROUJE = 32,				/**< Icône Promotion Rouje */
		ICON_ORANJE = 33,				/**< Icône Promotion Oranje */
		ICON_QUIT = 34,				/**< Icône Quitter */
		ICON_ON = 35,					/**< Icône Activé, Allumé, Connecté */
		ICON_OFF = 36,					/**< Icône Désactivé, Eteind, Déconnecté */
		ICON_SEND = 37,				/**< Icône Envoyer */
		ICON_SYSTRAYHERE = 38,		/**< Icône Trayicon en cas de présence */
		ICON_SYSTRAYAWAY = 39,		/**< Icône Trayicon en cas d'absence */
		ICON_LAYOUT = 40,				/**< Icône Affichage */
		ICON_NETWORK = 41,			/**< Icône Réseau */
		ICON_OS0 = 42,					/**< Icône OS Iconnu 32x32 */
		ICON_OS0_LARGE = 43,			/**< Icône OS Iconnu 64x64 */
		ICON_OS1 = 44,					/**< Icône OS Win9x 32x32 */
		ICON_OS1_LARGE = 45,			/**< Icône OS Win9x 64x64 */
		ICON_OS2 = 46,					/**< Icône OS WinNT 32x32 */
		ICON_OS2_LARGE = 47,			/**< Icône OS WinNT 64x64 */
		ICON_OS3 = 48,					/**< Icône OS Linux 32x32 */
		ICON_OS3_LARGE = 49,			/**< Icône OS Linux 64x64 */
		ICON_OS4 = 50,					/**< Icône OS MacOS9 32x32 */
		ICON_OS4_LARGE = 41,			/**< Icône OS MacOS9 64x64 */
		ICON_OS5 = 52,					/**< Icône OS MacOSX 32x32 */
		ICON_OS5_LARGE = 53,			/**< Icône OS MacOSX 64x64 */
		ICON_OS6 = 54,					/**< Icône OS BSD 32x32 */
		ICON_OS6_LARGE = 55,			/**< Icône OS BSD 64x64 */
		ICON_PRINTER = 56,			/**< Icône Imprimante */
		ICON_NOPRINTER = 57,			/**< Icône Pas d'imprimante */
		ICON_LOAD = 58,				/**< Icône Charger */
		ICON_UNLOAD = 59,				/**< Icône Décharger */
		ICON_RELOAD = 60,				/**< Icône Recharger */
		ICON_MAIL = 61,				/**< Icône pour les mails */
		ICON_PHONE = 62,				/**< Icône pour le téléphone */
		ICON_FILE = 63,				/**< Icône pour les fichiers */
		ICON_SYSTRAYDISCON = 64,	/**< Icône Trayicon en cas de déconnexion des serveurs */
		ICON_FILETRANSFER = 65,		/**< Icone du trandfert de fichier */
		ICON_REFUSE = 66,			/**< Icone de refus du transfert */
		ICON_NUMBER = 67				/**< Nombre d'icône... */
	};

	///Types de messages auxquel on peut s'attendre dans un protocole de chat
	enum ChatMessageType {
		Chat = 0,			/**< Message de discussion */
		Responder = 1,		/**< Message de répondeur */
		Ping = 2,			/**< Demande de ping */
		Pong = 3,			/**< Réponse à un ping (avec le temps en ms)*/
		Typing = 4,			/**< Informe qu'un message est en cours d'édition */
		StopTyping = 5,	/**< Informe que l'édition du message a été annulée */
		InfoMessage = 6,	/**< Envoie d'un message d'information */
		Closed = 7,			/**< Notification de la fin de la discussion */
		File = 8			/**< Notification lors du transfert de fichier */
	};

	///Défini une version
	/** La version est simplement définie de la forme
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
	
	///Flags pour retrouver les parties du numéro de version
	enum VersionPartFlags
	{
		NoVersion = 0,			/**< Pas de numéro de version */
		MajorVersion = 1,		/**< Demande d'affichage du numéro de version majeure */
		MinorVersion = 2,		/**< Demande d'affichage du numéro de version mineure */
		BuildVersion = 4,		/**< Demande d'affichage du numéro de build */
		TagVersion = 8,		/**< Demande d'affichage du tag */
		ShortVersion = MajorVersion | MinorVersion,	/**< Demande d'affichage d'un numéro de version court major.minor */
		LongVersion = ShortVersion | BuildVersion,	/**< Demande d'affichage d'un numéro de version long major.minor.build */
		CompleteVersion = LongVersion | TagVersion	/**< Demande d'affichage d'un numéro de version complète major.minor.build-tag */
	};
	Q_DECLARE_FLAGS(VersionParts, VersionPartFlags)
	RZX_CORE_EXPORT QString versionToString(const Rzx::Version&, VersionParts = CompleteVersion);
	RZX_CORE_EXPORT Rzx::Version versionFromString(const QString&);

	///Définition de la fonction d'output surchargée
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

	///Conversion des différents types en icône
	RZX_CORE_EXPORT Icon toIcon(SysEx, bool = true);
	RZX_CORE_EXPORT Icon toIcon(Promal);
	RZX_CORE_EXPORT Icon toIcon(ConnectionState);
};

///Défini les fonctions nécessaire pour générer un objet global
/** Cette macro n'a pour but que de fournir un système homogène pour
 * définir des objets globaux pour une classe
 *
 * Il faut placer cet macro dans la partie privée de la déclaration
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

///Initialise la définition globale
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

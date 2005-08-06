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

#include "defaults.h"

///Namespace pour encadrer les enums communs à la plupart des interfaces
namespace Rzx
{
	enum SysEx {
		SYSEX_UNKNOWN = 0,
		SYSEX_WIN9X = 1,
		SYSEX_WINNT = 2,
		SYSEX_LINUX = 3,
		SYSEX_MACOS = 4,
		SYSEX_MACOSX = 5,
		SYSEX_BSD = 6
	};

	enum Promal {
		PROMAL_UNK = 0,
		PROMAL_ORANGE = 1,
		PROMAL_ROUJE = 2,
		PROMAL_JONE = 3
	};
	
	enum ConnectionState {
		STATE_DISCONNECTED = 0,
		STATE_HERE = 1,
		STATE_AWAY = 2,
		STATE_REFUSE = 3
	};	
	
	///Définition des capabilities supplémentaires connues
	enum Capabilities {
		CAP_NONE = 0,
		CAP_ON = 1,
		CAP_CHAT = 2,
		CAP_XPLO = 4
	};

	///Définition des identifiants des sous réseaux
	enum RezalId {
		RZL_WORLD = 0,
		RZL_BINETS = 1,
		RZL_BR = 2,
		RZL_BEMA = 3,
		RZL_BEMD = 4,
		RZL_FOCH0 = 5,
		RZL_FOCH1 = 6,
		RZL_FOCH2 = 7,
		RZL_FOCH3 = 8,
		RZL_FAYOLLE0 = 9,
		RZL_FAYOLLE1 = 10,
		RZL_FAYOLLE2 = 11,
		RZL_FAYOLLE3 = 12,
		RZL_PEM = 13,
		RZL_JOFFRE0 = 14,
		RZL_JOFFRE1 = 15,
		RZL_JOFFRE2 = 16,
		RZL_JOFFRE3 = 17,
		RZL_MAUNOURY0 = 18,
		RZL_MAUNOURY1 = 19,
		RZL_MAUNOURY2 = 20,
		RZL_MAUNOURY3 = 21,
		RZL_BAT411 = 22,
		RZL_BAT70 = 23,
		RZL_BAT71 = 24,
		RZL_BAT72 = 25,
		RZL_BAT73 = 26,
		RZL_BAT74 = 27,
		RZL_BAT75 = 28,
		RZL_BAT76 = 29,
		RZL_BAT77 = 30,
		RZL_BAT78 = 31,
		RZL_BAT79 = 32,
		RZL_BAT80 = 33,
		RZL_WIFI = 34,
		RZL_X = 35,
		RZL_NUMBER = 36
	};

	///Identifiant des icônes
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
		ICON_NUMBER = 56
	};

	///Définition de la fonction d'output surchargée
	void installMsgHandler();
	void closeMsgHandler();
	void useOutputFile(FILE *file = NULL);

	//Chargement d'un module
	void beginModuleLoading(const QString&);
	void endModuleLoading(const QString&, bool success = true);

	//Fermeture d'un module
	void beginModuleClosing(const QString&);
	void endModuleClosing(const QString&);

	///Pour trier case unsensitive
	bool caseInsensitiveLessThan(const QString&, const QString&);
};

#endif

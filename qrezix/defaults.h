/***************************************************************************
                          defaults.h  -  description
                             -------------------
    begin                : Fri Jan 24 2003
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32			// sous linux, la version est definie dans le makefile
#define VERSION "3.x"
#endif

#define DEFAULT_COMMENT "qRezix v1.3 : la nouvelle version de Rezix !! (mais si tu lis ca, je suis un boulet)"
#define DEFAULT_SERVER "xnetserver.eleves.polytechnique.fr"
#define DEFAULT_PORT 5053
#define DEFAULT_CHATPORT 5050
#define DEFAULT_WINDOWSIZE "100000000"
#define DEFAULT_RECONNECTION 60000
#define DEFAULT_TIMEOUT 120000
#define DEFAULT_MAIL "Inscris toi sur polytechnique.org"
#define DEFAULT_THEME "classic"

// Version envoyee au serveur
#define RZX_CLIENT_ID (0x60)
#define RZX_MAJOR_VERSION (1)
#define RZX_MINOR_VERSION (3)
#define RZX_FUNNY_VERSION (666)


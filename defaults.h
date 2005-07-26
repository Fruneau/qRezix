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

#define DEFAULT_SERVER "xnetserver.eleves.polytechnique.fr"
#define DEFAULT_PORT 5053
#define DEFAULT_CHATPORT 5050
#define DEFAULT_WINDOWSIZE "100000000"
#define DEFAULT_RECONNECTION 60000
#define DEFAULT_TIMEOUT 120000
#define DEFAULT_MAIL "Inscris toi sur polytechnique.org"
#define DEFAULT_THEME "krystal"
#define DEFAULT_THEME_HELP "classic"

// Version envoyee au serveur
#define RZX_CLIENT_ID (0x06)
#define RZX_MAJOR_VERSION (1)
#define RZX_MINOR_VERSION (7)
#define RZX_FUNNY_VERSION (0)
#define RZX_TAG_VERSION   "-svn"

#define RZX_VERSION QString("%1.%2").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION)

#ifndef HAVE_CONFIG_H			// sous linux, la version est definie dans config.h
	#define VERSION QString("%1.%2.%3").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION).arg(RZX_FUNNY_VERSION)
#endif

#ifdef Q_OS_MAC
	#define QREZIX_ICON "q_mac.xpm"
#else
	#define QREZIX_ICON "q.xpm"
#endif
#define QREZIX_AWAY_ICON "t.xpm"

#ifdef HAVE_CONFIG_H
	#include "../config.h"
#else
	#ifdef Q_OS_MAC
		#define QREZIX_DATA_DIR "./qrezix.app/Contents/Resources/"
	#else
		#define QREZIX_DATA_DIR "/usr/share/qrezix"
		#define QREZIX_LIB_DIR "/usr/lib/qrezix"
	#endif
#endif

//Obsolete, seulement pour la compatibilité avec qRezix < 1.5
#define CONF_FAVORITES "favorites.conf"
#define CONF_IGNORELIST "ignorelist.conf"

#define DEFAULT_COMMENT "La nouvelle version de qRezix, plus que le WEI, c'est le feu... merci les BRmen\n" \
	"L'ascenseur : \"Quand tu montes dans un ascenseur... tu penses. À des tas de choses ; à des créations, à des gens, à des souvenirs... Donc on est jamais seul spirituellement! Mais physiquement, \"dans l'enveloppe\", si je suis seul... eh bien... je suis là. Et je reste là. Jusqu'à ce que les portes s'ouvrent... Et puis je commence à marcher. Je bouge mon enveloppe. Vers ma mission de tous les jours... -+- Jean-Claude VanDamme -+-\n" \
	"meme ke moloss il a un alias il tape /mortderire et ca ecrit <moloss> mdr -+- Big-Toof in GPJ: Les alias les plus courts sont les meilleurs -+-\n" \
	"Mieux vaut penser le changement que changer de pansement. -+- Francis Blanche -+-\n" \
	"Un bousier dans chaque narine... et plus besoin... de se mettre les doigts dans le nez...\n" \
	"Pour toi, je serai femme, maîtresse, mère. -+- Louis Calaferte, La mécanique des femmes -+-\n" \
	"> Sérieusement tu crois que la Debian est plus stable que les autres ? Pour moi c'est juste une simple distrib pour une bande de pauvres cons fanatiques\n" \
	"Ta mère c'est Bernard Minet. -+- SH in GFA : Bien configurer sa mère -+-\n" \
	"Si ça marche pas c'est que problème est entre la chaine et le clavier\n" \
	"Le fait que le monde soit peuplé de crétins permet à chacun de nous de ne pas se faire remarquer. -+- Sim -+-\n" \
	"INFINITÉSIMAL : On ne sais pas ce que ce c'est, mais a rapport à l'homéopathie. -+- Gustave Flaubert, Dictionnaire des idées reçues -+-\n" \
	"Chic à la jône X2003 !!!\n" \
	"o< o< o< o< o< o< o< \\_o< \\_0< COIN ! COIN !\n" \
	"Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! PAN ! PAAANNN !\n" \
	"Chaque jour est veille pour les uns, lendemain pour d'autres, mais aujourd'hui pour bien peu. -+- Gilbert Cesbron (1913-1979), de petites choses. -+-\n" \
	"> cwd     /home/celine euh, c'est pas plutôt cwd /femme/celine ? -+- MS in GFA : homme, sweet homme -+-\n" \
	"qRezix v" + RZX_VERSION + " is running... oupa\n" \
	"qRezix v" + RZX_VERSION + " la nouvelle version de qRezix...\n" \
	"Je suis un boulet sans imagination, et donc j'ai laissé la phrase par défaut de qRezix (which is running...)\n"

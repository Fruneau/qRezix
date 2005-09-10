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

#define DEFAULT_MAIL "Inscris toi sur polytechnique.org"
#define DEFAULT_THEME "krystal"

#ifdef Q_OS_WIN
#	define DEFAULT_SAMBACMD "standard"
#	define DEFAULT_FTPCMD "standard"
#	define DEFAULT_HTTPCMD "standard"
#	define DEFAULT_NEWSCMD "standard"
#else
#ifdef Q_OS_MAC
#	define DEFAULT_SAMBACMD "open"
#	define DEFAULT_FTPCMD "Default"
#	define DEFAULT_HTTPCMD "Default"
#	define DEFAULT_NEWSCMD "Default"
#else
#	define DEFAULT_SAMBACMD "standard"
#	define DEFAULT_FTPCMD "gftp"
#	define DEFAULT_HTTPCMD "firefox"
#	define DEFAULT_NEWSCMD "knode"
#endif
#endif


// Version envoyee au serveur
#define RZX_CLIENT_ID (0x06)
#define RZX_MAJOR_VERSION (1)
#define RZX_MINOR_VERSION (7)
#define RZX_FUNNY_VERSION (0)
#define RZX_TAG_VERSION   "-svn"

#define RZX_VERSION QString("%1.%2").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION)
#ifndef HAVE_CONFIG_H
#	define VERSION QString("%1.%2.%3").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION).arg(RZX_FUNNY_VERSION)
#endif

#ifdef Q_OS_MAC
#	define QREZIX_ICON "../resources/q_mac.xpm"
#else
#	define QREZIX_ICON "../resources/q.xpm"
#endif
#define QREZIX_AWAY_ICON "../resources/t.xpm"

#ifdef HAVE_CONFIG_H
#	include "../../config.h"
#else
#	ifdef Q_OS_MAC
#		define QREZIX_DATA_DIR "./qRezix.app/Contents/Resources/"
#		define QREZIX_LIB_DIR "./qRezix.app/Contents/Frameworks/"
#		define QREZIX_SYSTEM_DIR "./qRezix.app/Contents/MacOS/"
#	else
#		define QREZIX_DATA_DIR "/usr/share/qrezix"
#		define QREZIX_LIB_DIR "/usr/lib/qrezix"
#	endif
#endif

//Obsolete, seulement pour la compatibilit� avec qRezix < 1.5
#define CONF_FAVORITES "favorites.conf"
#define CONF_IGNORELIST "ignorelist.conf"

#define DEFAULT_COMMENT "La nouvelle version de qRezix, plus que le WEI, c'est le feu... merci les BRmen\n" \
	"L'ascenseur : \"Quand tu montes dans un ascenseur... tu penses. � des tas de choses ; � des cr�ations, � des gens, � des souvenirs... Donc on est jamais seul spirituellement! Mais physiquement, \"dans l'enveloppe\", si je suis seul... eh bien... je suis l�. Et je reste l�. Jusqu'� ce que les portes s'ouvrent... Et puis je commence � marcher. Je bouge mon enveloppe. Vers ma mission de tous les jours... -+- Jean-Claude VanDamme -+-\n" \
	"meme ke moloss il a un alias il tape /mortderire et ca ecrit <moloss> mdr -+- Big-Toof in GPJ: Les alias les plus courts sont les meilleurs -+-\n" \
	"Mieux vaut penser le changement que changer de pansement. -+- Francis Blanche -+-\n" \
	"Un bousier dans chaque narine... et plus besoin... de se mettre les doigts dans le nez...\n" \
	"Pour toi, je serai femme, ma�tresse, m�re. -+- Louis Calaferte, La m�canique des femmes -+-\n" \
	"> S�rieusement tu crois que la Debian est plus stable que les autres ? Pour moi c'est juste une simple distrib pour une bande de pauvres cons fanatiques\n" \
	"Ta m�re c'est Bernard Minet. -+- SH in GFA : Bien configurer sa m�re -+-\n" \
	"Si �a marche pas c'est que probl�me est entre la chaine et le clavier\n" \
	"Le fait que le monde soit peupl� de cr�tins permet � chacun de nous de ne pas se faire remarquer. -+- Sim -+-\n" \
	"INFINIT�SIMAL : On ne sais pas ce que ce c'est, mais a rapport � l'hom�opathie. -+- Gustave Flaubert, Dictionnaire des id�es re�ues -+-\n" \
	"Chic � la j�ne X2003 !!!\n" \
	"o< o< o< o< o< o< o< \\_o< \\_0< COIN ! COIN !\n" \
	"Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! PAN ! PAAANNN !\n" \
	"Chaque jour est veille pour les uns, lendemain pour d'autres, mais aujourd'hui pour bien peu. -+- Gilbert Cesbron (1913-1979), de petites choses. -+-\n" \
	"> cwd     /home/celine euh, c'est pas plut�t cwd /femme/celine ? -+- MS in GFA : homme, sweet homme -+-\n" \
	"qRezix v" + RZX_VERSION + " is running... oupa\n" \
	"qRezix v" + RZX_VERSION + " la nouvelle version de qRezix...\n" \
	"Je suis un boulet sans imagination, et donc j'ai laiss� la phrase par d�faut de qRezix (which is running...)\n"

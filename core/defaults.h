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
#define DEFAULT_THEME "Nuvola"

#ifdef Q_OS_WIN
#	define DEFAULT_SAMBACMD "standard"
#	define DEFAULT_FTPCMD "standard"
#	define DEFAULT_HTTPCMD "standard"
#	define DEFAULT_NEWSCMD "standard"
#	define DEFAULT_MAILCMD "standard"
#else
#ifdef Q_OS_MAC
#	define DEFAULT_SAMBACMD "open"
#	define DEFAULT_FTPCMD "Default"
#	define DEFAULT_HTTPCMD "Default"
#	define DEFAULT_NEWSCMD "Default"
#	define DEFAULT_MAILCMD "Default"
#else
#	define DEFAULT_SAMBACMD "standard"
#	define DEFAULT_FTPCMD "gftp"
#	define DEFAULT_HTTPCMD "firefox"
#	define DEFAULT_NEWSCMD "knode"
#	define DEFAULT_MAILCMD "kmail"
#endif
#endif


// Version envoyee au serveur
#define RZX_CLIENT_ID (0x06)

#ifdef Q_OS_MAC
#	undef QREZIX_LIB_DIR
#	undef QREZIX_SYSTEM_DIR
#	define QREZIX_LIB_DIR "./qRezix.app/Contents/Frameworks/"
#	define QREZIX_SYSTEM_DIR "./qRezix.app/Contents/Resources/"
#else
#	ifndef PREFIX
#		define PREFIX "/usr"
#	endif
#	ifndef QREZIX_DATA_DIR
#		define QREZIX_DATA_DIR PREFIX "/share/qrezix"
#	endif
#	ifndef QREZIX_LIB_DIR
#		define QREZIX_LIB_DIR PREFIX "/lib/qrezix"
#	endif
#endif

#define DEFAULT_COMMENT "La nouvelle version de qRezix, plus que le WEI, c'est le feu... merci les BRmen\r\n" \
	"L'ascenseur : Quand tu montes dans un ascenseur... tu penses. � des tas de choses ; � des cr�ations, � des gens, � des souvenirs... Donc on est jamais seul spirituellement! Mais physiquement, \"dans l'enveloppe\", si je suis seul... eh bien... je suis l�. Et je reste l�. Jusqu'� ce que les portes s'ouvrent... Et puis je commence � marcher. Je bouge mon enveloppe. Vers ma mission de tous les jours... \n -+- Jean-Claude VanDamme -+-\r\n" \
	"meme ke moloss il a un alias il tape /mortderire et ca ecrit <moloss> mdr \n -+- Big-Toof in GPJ: Les alias les plus courts sont les meilleurs -+-\r\n" \
	"Mieux vaut penser le changement que changer de pansement. \n -+- Francis Blanche -+-\r\n" \
	"Un bousier dans chaque narine... et plus besoin... de se mettre les doigts dans le nez...\r\n" \
	"Pour toi, je serai femme, ma�tresse, m�re. \n -+- Louis Calaferte, La m�canique des femmes -+-\r\n" \
	"> S�rieusement tu crois que la Debian est plus stable que les autres ? Pour moi c'est juste une simple distrib pour une bande de pauvres cons fanatiques\r\n" \
	"Ta m�re c'est Bernard Minet. \n -+- SH in GFA : Bien configurer sa m�re -+-\r\n" \
	"Si �a marche pas c'est que probl�me est entre la chaine et le clavier\r\n" \
	"Le fait que le monde soit peupl� de cr�tins permet � chacun de nous de ne pas se faire remarquer. \n -+- Sim -+-\r\n" \
	"INFINIT�SIMAL : On ne sais pas ce que ce c'est, mais a rapport � l'hom�opathie. \n -+- Gustave Flaubert, Dictionnaire des id�es re�ues -+-\r\n" \
	"Chic � la rouge X2006 !!!\r\n" \
	"o< o< o< o< o< o< o< \\_o< \\_0< COIN ! COIN !\r\n" \
	"Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! Pan ! PAN ! PAAANNN !\r\n" \
	"Chaque jour est veille pour les uns, lendemain pour d'autres, mais aujourd'hui pour bien peu. \n -+- Gilbert Cesbron (1913-1979), de petites choses. -+-\r\n" \
	"> cwd /home/celine euh, c'est pas plut�t `cwd /femme/celine` ? \n -+- MS in GFA : homme, sweet homme -+-\r\n" \
	"qRezix v" + Rzx::versionToString(RzxApplication::version()) + " is running... oupa\r\n" \
	"qRezix v" + Rzx::versionToString(RzxApplication::version()) + " la nouvelle version de qRezix...\r\n" \
	"Je suis un boulet sans imagination, et donc j'ai laiss� la phrase par d�faut de qRezix (which is running...)\r\n" \
	"Inscris toi � Polytechnique.org\r\n"

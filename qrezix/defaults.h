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

#define DEFAULT_COMMENT "La nouvelle version de qRezix, plus que le WEI, c'est le feu... merci les BRmen\nJ'ai diminu� un peu la vitesse de d�marrage de MultiDeskOS pour acc�l�rer son ex�cution.-- Jayce - Allez comprendre ! --\nL'ascenseur : \"Quand tu montes dans un ascenseur... tu penses. � des tas de choses ; � des cr�ations, � des gens, � des souvenirs... Donc on est jamais seul spirituellement! Mais physiquement, \"dans l'enveloppe\", si je suis seul... eh bien... je suis l�. Et je reste l�. Jusqu'� ce que les portes s'ouvrent... Et puis je commence � marcher. Je bouge mon enveloppe. Vers ma mission de tous les jours... -+- Jean-Claude VanDamme -+-\n meme ke moloss il a un alias il tape /mortderire et ca ecrit <moloss> mdr -+- Big-Toof in GPJ: Les alias les plus courts sont les meilleurs -+-\nMieux vaut penser le changement que changer de pansement. -+- Francis Blanche -+-\nUn bousier dans chaque narine... et plus besoin... de se mettre les doigts dans le nez...\nPour toi, je serai femme, ma�tresse, m�re. -+- Louis Calaferte, La m�canique des femmes -+-\n> S�rieusement tu crois que la Debian est plus stable que les autres ? Pour moi c'est juste une simple distrib pour une bande de pauvres cons fanatiques     Ta m�re c'est Bernard Minet. -+- SH in GFA : Bien configurer sa m�re -+-\nSi �a marche pas c'est que probl�me est entre la chaine et le clavier\nLe fait que le monde soit peupl� de cr�tins permet � chacun de nous de ne pas se faire remarquer. -+- Sim -+-\nINFINIT�SIMAL : On ne sais pas ce que ce c'est, mais a rapport � l'hom�opathie. -+- Gustave Flaubert, Dictionnaire des id�es re�ues -+-"


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
#define RZX_MINOR_VERSION (5)
#define RZX_FUNNY_VERSION (0)

#define RZX_VERSION QString("%1.%2").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION)

#ifdef WIN32			// sous linux, la version est definie dans le makefile
#define VERSION QString("%1.%2.%3").arg(RZX_MAJOR_VERSION).arg(RZX_MINOR_VERSION).arg(RZX_FUNNYVERSION)
#endif


/***************************************************************************
                         rzxplugin.h  -  description
                             -------------------
    begin                : Thu Jul 19 2004
    copyright            : (C) 2004 by Florent Bruneau
    email                : fruneau@melix.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef RZXPLUGIN_H
#define RZXPLUGIN_H

#include <qstring.h>
#include <qstringlist.h>
#include <qpopupmenu.h>
#include <qobject.h>
#include <qsettings.h>
#include <qvariant.h>

/**
@file rzxplugin.h
*/

///Version de la structure de plug-in
/** Cette version est un entier de 32bits en 3 parties PLUGIN_VERSION = 0xaaaccbbb
 *		- aaa = numéro de version principale, sert à marquer les incompatibilité majeurs. <br><i>Lorsque que aaa change, on a perdu la compatibilité avec la version précédente (il faut réécrirer certaines parties du code des plug-ins pour les rendres compatibles).</i>
 *		- cc = numéro de version secondaire, sert à marquer les évolutions de la version. <br><i>Lorsque que cc change, on a perdu la compatibilité avec la version précédente (il faut recompiler tous les plug-ins existants pour les rendres compatibles sans apporter de modification au code).</i>
 *		- bbb = numéro de version ternaire, sert simplement de marqueur d'évolution minimes. <br><i>Même si bbb change, la compatibilité avec la version précédente n'est pas perdu. Les nouveaux plug-ins peuvent alors avoir accès à de nouvelles possibilités sans que cela n'empêche le bon fonctionnement des anciens plug-ins.</i>
 *
 * Il est important de comprendre que ce numéro de version ne dépend pas du comportement de qRezix envers le plug-in (on peut très bien envisager de faire évoluer qRezix pour garder la compatibilité avec les anciennes version de plug-ins) mais de l'évolution de la structure du plug-in. En fait, Cette structure dépend de QObject et de RzxPlugIn, mais seule les évolutions de RzxPlugIn entre en compte pour le calcul de cette version (le principe étant de compiler les plugins de qRezix avec la même version de Qt que pour qRezix lui-même).
 */
#define PLUGIN_VERSION 0x00102004

class QWidget;
class QPixmap;

///Exportation du plug-in
/** Cette macro permet l'exportation du plug-in de manière sûre vers qRezix. De manière indépendante de la plate-forme et de la version du compilateur.
 * \param PLUGIN est le nom de la classe du plug-in exporté. Cette classe doit comporté un constructeur sans paramètre car la macros construit un plug-in avec
 * \code
 new PLUGIN();
 \endcode
 */
#ifdef Q_WS_WIN
#	define RZX_EXPORT_PLUGIN(PLUGIN) \
		extern "C" __declspec(dllexport) RzxPlugIn *getPlugIn(void) { return (RzxPlugIn*)(new PLUGIN()); }
#else
#	define RZX_EXPORT_PLUGIN(PLUGIN) \
		extern "C" RzxPlugIn *getPlugIn(void) { return (RzxPlugIn*)(new PLUGIN()); }
#endif

///Exportation du plug-in avec paramètre
/** Cette macro permet l'exportation du plug-in de manière sûre vers qRezix. De manière indépendante de la plate-forme et de la version du compilateur.
 * \param PLUGIN est l'appel au constructeur désiré pour le plug-in. La macro va en effet effectuer
 * \code
 new PLUGIN;
 \endcode
 */
#ifdef Q_WS_WIN
#	define RZX_EXPORT_PLUGIN_WITH_PARAM(PLUGIN) \
		extern "C" __declspec(dllexport) RzxPlugIn *getPlugIn(void) { return (RzxPlugIn*)(new PLUGIN); }
#else
#	define RZX_EXPORT_PLUGIN_WITH_PARAM(PLUGIN) \
		extern "C" RzxPlugIn *getPlugIn(void) { return (RzxPlugIn*)(new PLUGIN); }
#endif


/**
@author Florent Bruneau
*/

///La classe plug-in fournit la structure de base d'un plug-in
/** Tout plug-in de qRezix doit être construit autour d'une classe héritant de de RzxPlugIn.
 * Cette classe contient toutes les fonctions utiles pour la réalisation d'un plug-in avec en particulier
 *		- les outils pour communiquer avec le programme principale
 *		- les outils pour envoyer des données au serveur
 *		- les outils pour stocker les données de configuration
 *
 * Cette classe doit tout de même être utilisée comme une interface entre une classe principale du plug-in et qRezix.
 * <br><br><b>IMPORTANT, pour les dével de qRezix : </b> Le structure de plug-in doit être suffisamment constante pour pouvoir tester les données de base (version et nom) de toutes les versions de RzxPlugIn avec les même fonctions. Il faut donc absolument laissé dans l'ordre en début de plug-in les données et les méthodes correspondantes.
 */
//DEBUT DE PARTIE A NE PAS MODIFIER DANS LA CLASSE
class RzxPlugIn : public QObject		//NE PAS MODIFIER
{												//NE PAS MODIFIER
	Q_OBJECT									//NE PAS MODIFIER
												//NE PAS MODIFIER
	QString name;							//NE PAS MODIFIER
	int version;							//NE PAS MODIFIER
												//NE PAS MODIFIER
	public:									//NE PAS MODIFIER
		QString getName();				//NE PAS MODIFIER
		int getVersion();					//NE PAS MODIFIER
//FIN DE PARTIE A NE PAS MODIFIER DANS LA CLASSE
	private:
		QString description;

	public:
		///Données pour les requêtes de données
		enum Data
		{
			DATA_NONE = 0,				/**< normalement inutilisé */
			DATA_SERVERFTP = 1,		/**< demande s'il y a un serveur ftp sur l'ordinateur, on attend un booléen en retour */
			DATA_SERVERHTTP = 2,		/**< demande s'il y a un serveur http sur l'ordinateur, on attend un booléen en retour */
			DATA_SERVERNEWS = 3,		/**< demande s'il y a un serveur de news sur l'ordinateur, on attend un booléen en retour */
			DATA_SERVERSMB = 4,		/**< demande s'il y a un serveur samba sur l'ordinateur, on attend un booléen en retour */
			DATA_DNSNAME = 5,			/**< demande le nom dns de l'ordinateur, on attend une QString */
			DATA_NAME = 6,				/**< demande le nom de famille de l'utilisateur, on attend une QString */
			DATA_FIRSTNAME = 7,		/**< demande le prénom de l'utilisateur, on attend une QString */
			DATA_SURNAME = 8,			/**< demande le surnom de l'utilisateur, on attend une QString */
			DATA_IP = 9,				/**< demande l'ip sous la forme xxx.xxx.xxx.xxx, on attend une QString */
			DATA_CHAT = 10,			/**< indique que le chat actif a changé, qRezix envoie pour information un QTextEdit* qui n'est autre que les données du champs d'édition de la fenêtre de chat actuelle, <i>ce QTextEdit* est envoyé à la place du QVariant*</i> */
			DATA_CHATEMIT = 11,		/**< indique que le contenu de la fenêtre de chat va être envoyé, aucune donnée n'est envoyée */
			DATA_CHATRECEIVE = 12,	/**< indique qu'un chat a été reçu dans le fenêtre de chat actuelle, qRezix envoie un QString* qui contient la chaîne de caractère reçue, <i>ce QString* est envoyé à la place du QVariant*</i> */
			DATA_WORKSPACE = 13,		/**< demande le répertoire de travail pour les clients ftp, http..., on attend une QString */
			DATA_ITEMSELECTED = 14,	/**< indique que l'item du rézal actuellement sélectionné à changé, qRezix envoie un QListViewItem* qui contient l'item actuellement sélectionné, <i>ce QListViewItem* est envoyé à la place du QVariant*</i> */
			DATA_FAVORITESELECTED = 15,	/**< indique que l'item du rézal de la fenêtre favoris actuellement sélectionné à changé, qRezix envoie un QListViewItem* qui contient l'item actuellement sélectionné, <i>ce QListViewItem* est envoyé à la place du QVariant*</i> */
			DATA_DISPFTP = 16,		/**< demande si le serveur ftp de l'ordinateur est affiché ou pas, on attend un booléen en retour */
			DATA_DISPHTTP = 17,		/**< demande si le serveur http de l'ordinateur est affiché ou pas, on attend un booléen en retour */
			DATA_DISPNEWS = 18,		/**< demande si le serveur news de l'ordinateur est affiché ou pas, on attend un booléen en retour */
			DATA_DISPSMB = 19,		/**< demande si le serveur samba de l'ordinateur est affiché ou pas, on attend un booléen en retour */
			DATA_LANGUAGE = 20,		/**< demande le langage utilisé par le programme, on attend une QString */
			DATA_THEME = 21,			/**< demande le thème utilisé par qRezix, on attend une QString en retour */
			DATA_AWAY = 22,			/**< demande si l'utilisateur est marqué comme loin, on attend un booléen en retour */
			DATA_ICONFTP = 23,		/**< demande l'icône ftp du thème actuel, on attend un QPixmap en retour */
			DATA_ICONHTTP = 24,		/**< demande l'icône http du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNEWS = 25,		/**< demande l'icône new du thème actuel, on attend un QPixmap en retour */
			DATA_ICONSAMBA = 26,		/**< demande l'icône samba du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNOFTP = 27,		/**< demande l'icône no_ftp du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNOHTTP = 28,	/**< demande l'icône no_http du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNONEWS = 29,	/**< demande l'icône no_news du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNOSAMBA = 30,	/**< demande l'icône no_samba du thème actuel, on attend un QPixmap en retour */
			DATA_ICONSAMEGATEWAY = 31,	/**< demande l'icône same_gateway du thème actuel, on attend un QPixmap en retour */
			DATA_ICONDIFFGATEWAY = 32, /**< demande l'icône diff_gateway du thème actuel, on attend un QPixmap en retour */
			DATA_ICONJONE = 33,		/**< demande l'icône jone (chic à la jône) du thème actuel, on attend un QPixmap en retour */
			DATA_ICONORANGE = 34,	/**< demande l'icône orange du thème actuel, on attend un QPixmap en retour */
			DATA_ICONROUJE = 35,		/**< demande l'icône rouje du thème actuel, on attend un QPixmap en retour */
			DATA_ICONUNKOS = 36,		/**< demande l'icône os_0 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONUNKOSLARGE = 37,	/** < demande l'icône os_0_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONWIN = 38,		/**< demande l'icône os_1 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONWINLARGE = 39,	/**< demande l'icône os_1_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONWINNT = 40,		/**< demande l'icône os_2 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONWINNTLARGE = 41,	/**< demande l'icône os_2_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONLINUX = 42,		/**< demande l'icône os_3 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONLINUXLARGE = 43,	/**< demande l'icône os_3_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONMAC = 44,		/**< demande l'icône os_4 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONMACLARGE = 45, /**< demande l'icône os_4_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONMACX = 46,		/**< demande l'icône os_5 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONMACXLARGE = 47,	/**< demande l'icône os_5_large du thème actuel, on attend un QPixmap en retour */
			DATA_ICONSPEAKER = 48,	/**< demande l'icône haut_parleur1 du thème actuel, on attend un QPixmap en retour */
			DATA_ICONNOSPEAKER = 49,	/**< demande l'icône haut_parleur2 du thème actuel, on attend un QPixmap en retour */
			DATA_PLUGINPATH = 50,	/**< envoi du répertoire où est installé le plug-in, émit dès le démarrage du plug-in et NE PEUT PAS ETRE DEMANDE, on attend un QString */
			DATA_ICONPLUGIN = 51,	/**< demande l'icône du menu 'Plug-ins' du thème actuel, on attend un QPixmap en retour */
			DATA_ICONHERE = 52,		/**< demande l'icône 'Away' cliquée du thème actuel, on attend un QPixmap en retour */
			DATA_ICONAWAY = 53,		/**< demande l'icône 'Away non-cliquée du thème actuel, on attend un QPixmap en retour */
			DATA_ICONCOLUMNS = 54,	/**< demande l'icône 'Adjust columns' du thème actuel, on attend un QPixmap en retour */
			DATA_ICONPREF = 55,		/**< demande l'icône 'Preferences' du thème actuel, on attend un QPixmap en retour */
			DATA_ICONSEND = 56,		/**< demande l'icône send du thème actuel, on attend un QPixmap en retour*/
			DATA_ICONHIST = 57,		/**< demande l'icône historique du thème actuel, on attend un QPixmap en retour*/
			DATA_ICONPROP = 58, 		/**< demande l'icône prop du thème actuel, on attend un QPixmap en retour*/
			DATA_ICONCHAT = 59,		/**< demande l'icône chat du thème actuel, on attend un QPixmap en retour*/
			DATA_ICONSIZE = 60,		/**< demande la taille des icônes du menu, on attend un int en retour avec 0 pas d'icône, 1 petite icône, 2 grande icône*/
			DATA_ICONTEXT = 61,		/**< demande la position du texte des icônes du menu, on attend un int en retour avec 0 pas de texte, 1 à côté, 2 en dessous*/
			DATA_ICONFAVORITE = 62,	/**< demande l'icône favorite du thème actuel, on attend un QPixmap en retour*/
			DATA_ICONNOTFAVORITE = 63	/**< demande l'icône not_favorite du thème actuel, on attend un QPixmap en retour*/
		};

	protected:
		///Indique si le plug-in contient une fenêtre de propriétés
		bool prop;

		///Contient l'icône du plug-in
		QPixmap *icon;

		///Contribution du plug-in au menu de la tray-icon
		QPopupMenu *tray;
		///Contribution du plug-in au menu contextuel des éléments du rezal
		QPopupMenu *item;
		///Contribution du plug-in au menu plug-in de la fenêtre principale
		QPopupMenu *action;
		///Contribution du plug-in au menu plug-in de la fenêtre de chat
		QPopupMenu *chat;

		/*Les fonctions qui suivents fournissent au plug-in
		le matériel pour enregistrer et lire des données de
		configuration de manière organisée*/
		///Outils de stockage des propriétés cohérent avec celui de qRezix
		QSettings *settings;

	private:
		RzxPlugIn();
		
	public:
		RzxPlugIn(const QString& nm, const QString& desc);
		virtual ~RzxPlugIn();

		QString readEntry(const QString& name, const QString& def);
		int readNumEntry(const QString& name, int def);
		bool readBoolEntry(const QString& name, bool def);
		QStringList readListEntry(const QString& name);

		//Il est du devoir du concepteur de plug-in
		//de mettre à jour les champs name, description et prop
		//pour que ces valeurs fonction renvoient des données sensées
		QString getDescription();
		bool hasProp();
		QPixmap *getIcon();

		/* Partie abstraite devant être réimplémentée dans chaque
		classe fille qui définira un plug-in */
		///Envoi de la contribution au menu de la trayicon
		virtual QPopupMenu *getTrayMenu() = 0;
		///Envoi de la contribution au menu contextuel des éléments du rezal
		virtual QPopupMenu *getItemMenu() = 0;
		///Envoi de la contribution au menu de la fenêtre principale
		virtual QPopupMenu *getActionMenu() = 0;
		///Envoi de la contribution au menu de la fenêtre de chat
		virtual QPopupMenu *getChatMenu() = 0;

		///Initialisation du programme principale du plug-in
		/** Cette méthode abstraite doit implémentée le lancement de l'exécution du plug-in.
		 * <br>Cette exécution ne doit pas bloquer l'exécution du qRezix, il faut donc utiliser le programmation asynchrône pour réaliser le plug-in.
		 * \param set le QSetting utilisé par qRezix pour stocker les données de configuration
		 * \param ip ip à laquelle on peut trouver l'ordinateur local
		 */
		virtual void init(QSettings *set, const QString& ip) = 0;
		///Arrêt de l'exécution du programme principale du plug-in
		/** Cette méthode abstraite doit implémenter l'arrêt de l'exéuction du plug-in
		 */
		virtual void stop() = 0;

		///Affichage de la boite de dialogue des propriétés
		/** Dans le but d'implémenter la boîte de dialogue de manière asynchrône, il faut utiliser les slots propertiesOK() et propertiesCancel() pour la gestion des modifications des propriétés */
		virtual void properties(QWidget *prop = 0) = 0;

		//slots et signaux générique assurant les relations entre le plug-in lui même
		//et l'interface de qrezix
		//ce sont les seuls qui puissent être géré par l'interface de rezix
	public slots:
		///Réception de données depuis qRezix
		/** Cette méthode abstraite doit permettre de répartir les données entre les différents composant du plug-in.
		 * \param data le type de donnée que l'on reçoit
		 * \param value la valeur envoyée par qRezix au plug-in. Le QVariant n'est valide que pendant l'exécution de la méthode, il est donc de la responsabilité de l'utilisateur que de stocker le résultat.
		 */
		virtual void getData(Data data, const QVariant *value) = 0;
		void writeEntry(const QString& name, const QString& value);
		void writeEntry(const QString& name, int value);
		void writeEntry(const QString& name, bool value);
		void writeEntry(const QString& name, const QStringList& value);

	protected slots:
		void sender(const QString& msg);
		void querySender(Data data);
		
		///Fermeture de la fenêtre des propriétés par OK
		/** Cette méthode abstraite va de paire avec la méthode properties, elle permet d'utiliser la programmation asynchrône pour la boîte de dialogue des propriétés. Ce connecteur doit être connecté au bouton OK de la boîte de dialogue */
		virtual void propertiesOK() = 0;
		
		///Fermeture de la fenêtre des propriétés par Cancel
		/** Cette méthode abstraite va de paire avec la méthode properties, elle permet d'utiliser la programmation asynchrône pour la boîte de dialogue des propriétés. Ce connecteur doit être connecté au bouton OK de la boîte de dialogue */
		virtual void propertiesCancel() = 0;

	signals:
		///Demande d'envoi d'un message au serveur
		/** Ce signal demande l'envoi d'un message au serveur. Ce message doit donc être reconnu par le serveur.
		 * <br><b>L'UTILISATION DE CETTE FONCTION NE DOIT PAS SURCHARGER LE SERVEUR</b>
		 * \param msg contient le message complet envoyé au serveur
		 */
		void send(const QString& msg);
		///Demande de données à qRezix
		/** Ce signal constitue la seule manière qu'à le programme à obtenir des données depuis qRezix, il est à la charge du développeur de plug-in de faire attention car il s'agit d'un élément asynchrone.
		 * <br>On pourrait imaginer l'utilisation d'une fonction synchrône, mais forcer à la programmation asynchrône permet d'être sur que qRezix garde la main sur le plug-in
		 * \param data indique ce qu'on recherche comme donnée
		 * \param plugin indique quel plug-in requiert cette donnée. On a donc nécessairement <i>plugin = this</i>
		 */
		void queryData(RzxPlugIn::Data data, RzxPlugIn *plugin);
};

#endif

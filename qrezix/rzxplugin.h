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
 *		- aaa = num�ro de version principale, sert � marquer les incompatibilit� majeurs. <br><i>Lorsque que aaa change, on a perdu la compatibilit� avec la version pr�c�dente (il faut r��crirer certaines parties du code des plug-ins pour les rendres compatibles).</i>
 *		- cc = num�ro de version secondaire, sert � marquer les �volutions de la version. <br><i>Lorsque que cc change, on a perdu la compatibilit� avec la version pr�c�dente (il faut recompiler tous les plug-ins existants pour les rendres compatibles sans apporter de modification au code).</i>
 *		- bbb = num�ro de version ternaire, sert simplement de marqueur d'�volution minimes. <br><i>M�me si bbb change, la compatibilit� avec la version pr�c�dente n'est pas perdu. Les nouveaux plug-ins peuvent alors avoir acc�s � de nouvelles possibilit�s sans que cela n'emp�che le bon fonctionnement des anciens plug-ins.</i>
 *
 * Il est important de comprendre que ce num�ro de version ne d�pend pas du comportement de qRezix envers le plug-in (on peut tr�s bien envisager de faire �voluer qRezix pour garder la compatibilit� avec les anciennes version de plug-ins) mais de l'�volution de la structure du plug-in. En fait, Cette structure d�pend de QObject et de RzxPlugIn, mais seule les �volutions de RzxPlugIn entre en compte pour le calcul de cette version (le principe �tant de compiler les plugins de qRezix avec la m�me version de Qt que pour qRezix lui-m�me).
 */
#define PLUGIN_VERSION 0x00102004

class QWidget;
class QPixmap;

///Exportation du plug-in
/** Cette macro permet l'exportation du plug-in de mani�re s�re vers qRezix. De mani�re ind�pendante de la plate-forme et de la version du compilateur.
 * \param PLUGIN est le nom de la classe du plug-in export�. Cette classe doit comport� un constructeur sans param�tre car la macros construit un plug-in avec
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

///Exportation du plug-in avec param�tre
/** Cette macro permet l'exportation du plug-in de mani�re s�re vers qRezix. De mani�re ind�pendante de la plate-forme et de la version du compilateur.
 * \param PLUGIN est l'appel au constructeur d�sir� pour le plug-in. La macro va en effet effectuer
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
/** Tout plug-in de qRezix doit �tre construit autour d'une classe h�ritant de de RzxPlugIn.
 * Cette classe contient toutes les fonctions utiles pour la r�alisation d'un plug-in avec en particulier
 *		- les outils pour communiquer avec le programme principale
 *		- les outils pour envoyer des donn�es au serveur
 *		- les outils pour stocker les donn�es de configuration
 *
 * Cette classe doit tout de m�me �tre utilis�e comme une interface entre une classe principale du plug-in et qRezix.
 * <br><br><b>IMPORTANT, pour les d�vel de qRezix : </b> Le structure de plug-in doit �tre suffisamment constante pour pouvoir tester les donn�es de base (version et nom) de toutes les versions de RzxPlugIn avec les m�me fonctions. Il faut donc absolument laiss� dans l'ordre en d�but de plug-in les donn�es et les m�thodes correspondantes.
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
		///Donn�es pour les requ�tes de donn�es
		enum Data
		{
			DATA_NONE = 0,				/**< normalement inutilis� */
			DATA_SERVERFTP = 1,		/**< demande s'il y a un serveur ftp sur l'ordinateur, on attend un bool�en en retour */
			DATA_SERVERHTTP = 2,		/**< demande s'il y a un serveur http sur l'ordinateur, on attend un bool�en en retour */
			DATA_SERVERNEWS = 3,		/**< demande s'il y a un serveur de news sur l'ordinateur, on attend un bool�en en retour */
			DATA_SERVERSMB = 4,		/**< demande s'il y a un serveur samba sur l'ordinateur, on attend un bool�en en retour */
			DATA_DNSNAME = 5,			/**< demande le nom dns de l'ordinateur, on attend une QString */
			DATA_NAME = 6,				/**< demande le nom de famille de l'utilisateur, on attend une QString */
			DATA_FIRSTNAME = 7,		/**< demande le pr�nom de l'utilisateur, on attend une QString */
			DATA_SURNAME = 8,			/**< demande le surnom de l'utilisateur, on attend une QString */
			DATA_IP = 9,				/**< demande l'ip sous la forme xxx.xxx.xxx.xxx, on attend une QString */
			DATA_CHAT = 10,			/**< indique que le chat actif a chang�, qRezix envoie pour information un QTextEdit* qui n'est autre que les donn�es du champs d'�dition de la fen�tre de chat actuelle, <i>ce QTextEdit* est envoy� � la place du QVariant*</i> */
			DATA_CHATEMIT = 11,		/**< indique que le contenu de la fen�tre de chat va �tre envoy�, aucune donn�e n'est envoy�e */
			DATA_CHATRECEIVE = 12,	/**< indique qu'un chat a �t� re�u dans le fen�tre de chat actuelle, qRezix envoie un QString* qui contient la cha�ne de caract�re re�ue, <i>ce QString* est envoy� � la place du QVariant*</i> */
			DATA_WORKSPACE = 13,		/**< demande le r�pertoire de travail pour les clients ftp, http..., on attend une QString */
			DATA_ITEMSELECTED = 14,	/**< indique que l'item du r�zal actuellement s�lectionn� � chang�, qRezix envoie un QListViewItem* qui contient l'item actuellement s�lectionn�, <i>ce QListViewItem* est envoy� � la place du QVariant*</i> */
			DATA_FAVORITESELECTED = 15,	/**< indique que l'item du r�zal de la fen�tre favoris actuellement s�lectionn� � chang�, qRezix envoie un QListViewItem* qui contient l'item actuellement s�lectionn�, <i>ce QListViewItem* est envoy� � la place du QVariant*</i> */
			DATA_DISPFTP = 16,		/**< demande si le serveur ftp de l'ordinateur est affich� ou pas, on attend un bool�en en retour */
			DATA_DISPHTTP = 17,		/**< demande si le serveur http de l'ordinateur est affich� ou pas, on attend un bool�en en retour */
			DATA_DISPNEWS = 18,		/**< demande si le serveur news de l'ordinateur est affich� ou pas, on attend un bool�en en retour */
			DATA_DISPSMB = 19,		/**< demande si le serveur samba de l'ordinateur est affich� ou pas, on attend un bool�en en retour */
			DATA_LANGUAGE = 20,		/**< demande le langage utilis� par le programme, on attend une QString */
			DATA_THEME = 21,			/**< demande le th�me utilis� par qRezix, on attend une QString en retour */
			DATA_AWAY = 22,			/**< demande si l'utilisateur est marqu� comme loin, on attend un bool�en en retour */
			DATA_ICONFTP = 23,		/**< demande l'ic�ne ftp du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONHTTP = 24,		/**< demande l'ic�ne http du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNEWS = 25,		/**< demande l'ic�ne new du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONSAMBA = 26,		/**< demande l'ic�ne samba du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNOFTP = 27,		/**< demande l'ic�ne no_ftp du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNOHTTP = 28,	/**< demande l'ic�ne no_http du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNONEWS = 29,	/**< demande l'ic�ne no_news du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNOSAMBA = 30,	/**< demande l'ic�ne no_samba du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONSAMEGATEWAY = 31,	/**< demande l'ic�ne same_gateway du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONDIFFGATEWAY = 32, /**< demande l'ic�ne diff_gateway du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONJONE = 33,		/**< demande l'ic�ne jone (chic � la j�ne) du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONORANGE = 34,	/**< demande l'ic�ne orange du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONROUJE = 35,		/**< demande l'ic�ne rouje du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONUNKOS = 36,		/**< demande l'ic�ne os_0 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONUNKOSLARGE = 37,	/** < demande l'ic�ne os_0_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONWIN = 38,		/**< demande l'ic�ne os_1 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONWINLARGE = 39,	/**< demande l'ic�ne os_1_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONWINNT = 40,		/**< demande l'ic�ne os_2 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONWINNTLARGE = 41,	/**< demande l'ic�ne os_2_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONLINUX = 42,		/**< demande l'ic�ne os_3 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONLINUXLARGE = 43,	/**< demande l'ic�ne os_3_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONMAC = 44,		/**< demande l'ic�ne os_4 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONMACLARGE = 45, /**< demande l'ic�ne os_4_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONMACX = 46,		/**< demande l'ic�ne os_5 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONMACXLARGE = 47,	/**< demande l'ic�ne os_5_large du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONSPEAKER = 48,	/**< demande l'ic�ne haut_parleur1 du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONNOSPEAKER = 49,	/**< demande l'ic�ne haut_parleur2 du th�me actuel, on attend un QPixmap en retour */
			DATA_PLUGINPATH = 50,	/**< envoi du r�pertoire o� est install� le plug-in, �mit d�s le d�marrage du plug-in et NE PEUT PAS ETRE DEMANDE, on attend un QString */
			DATA_ICONPLUGIN = 51,	/**< demande l'ic�ne du menu 'Plug-ins' du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONHERE = 52,		/**< demande l'ic�ne 'Away' cliqu�e du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONAWAY = 53,		/**< demande l'ic�ne 'Away non-cliqu�e du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONCOLUMNS = 54,	/**< demande l'ic�ne 'Adjust columns' du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONPREF = 55,		/**< demande l'ic�ne 'Preferences' du th�me actuel, on attend un QPixmap en retour */
			DATA_ICONSEND = 56,		/**< demande l'ic�ne send du th�me actuel, on attend un QPixmap en retour*/
			DATA_ICONHIST = 57,		/**< demande l'ic�ne historique du th�me actuel, on attend un QPixmap en retour*/
			DATA_ICONPROP = 58, 		/**< demande l'ic�ne prop du th�me actuel, on attend un QPixmap en retour*/
			DATA_ICONCHAT = 59,		/**< demande l'ic�ne chat du th�me actuel, on attend un QPixmap en retour*/
			DATA_ICONSIZE = 60,		/**< demande la taille des ic�nes du menu, on attend un int en retour avec 0 pas d'ic�ne, 1 petite ic�ne, 2 grande ic�ne*/
			DATA_ICONTEXT = 61,		/**< demande la position du texte des ic�nes du menu, on attend un int en retour avec 0 pas de texte, 1 � c�t�, 2 en dessous*/
			DATA_ICONFAVORITE = 62,	/**< demande l'ic�ne favorite du th�me actuel, on attend un QPixmap en retour*/
			DATA_ICONNOTFAVORITE = 63	/**< demande l'ic�ne not_favorite du th�me actuel, on attend un QPixmap en retour*/
		};

	protected:
		///Indique si le plug-in contient une fen�tre de propri�t�s
		bool prop;

		///Contient l'ic�ne du plug-in
		QPixmap *icon;

		///Contribution du plug-in au menu de la tray-icon
		QPopupMenu *tray;
		///Contribution du plug-in au menu contextuel des �l�ments du rezal
		QPopupMenu *item;
		///Contribution du plug-in au menu plug-in de la fen�tre principale
		QPopupMenu *action;
		///Contribution du plug-in au menu plug-in de la fen�tre de chat
		QPopupMenu *chat;

		/*Les fonctions qui suivents fournissent au plug-in
		le mat�riel pour enregistrer et lire des donn�es de
		configuration de mani�re organis�e*/
		///Outils de stockage des propri�t�s coh�rent avec celui de qRezix
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
		//de mettre � jour les champs name, description et prop
		//pour que ces valeurs fonction renvoient des donn�es sens�es
		QString getDescription();
		bool hasProp();
		QPixmap *getIcon();

		/* Partie abstraite devant �tre r�impl�ment�e dans chaque
		classe fille qui d�finira un plug-in */
		///Envoi de la contribution au menu de la trayicon
		virtual QPopupMenu *getTrayMenu() = 0;
		///Envoi de la contribution au menu contextuel des �l�ments du rezal
		virtual QPopupMenu *getItemMenu() = 0;
		///Envoi de la contribution au menu de la fen�tre principale
		virtual QPopupMenu *getActionMenu() = 0;
		///Envoi de la contribution au menu de la fen�tre de chat
		virtual QPopupMenu *getChatMenu() = 0;

		///Initialisation du programme principale du plug-in
		/** Cette m�thode abstraite doit impl�ment�e le lancement de l'ex�cution du plug-in.
		 * <br>Cette ex�cution ne doit pas bloquer l'ex�cution du qRezix, il faut donc utiliser le programmation asynchr�ne pour r�aliser le plug-in.
		 * \param set le QSetting utilis� par qRezix pour stocker les donn�es de configuration
		 * \param ip ip � laquelle on peut trouver l'ordinateur local
		 */
		virtual void init(QSettings *set, const QString& ip) = 0;
		///Arr�t de l'ex�cution du programme principale du plug-in
		/** Cette m�thode abstraite doit impl�menter l'arr�t de l'ex�uction du plug-in
		 */
		virtual void stop() = 0;

		///Affichage de la boite de dialogue des propri�t�s
		/** Dans le but d'impl�menter la bo�te de dialogue de mani�re asynchr�ne, il faut utiliser les slots propertiesOK() et propertiesCancel() pour la gestion des modifications des propri�t�s */
		virtual void properties(QWidget *prop = 0) = 0;

		//slots et signaux g�n�rique assurant les relations entre le plug-in lui m�me
		//et l'interface de qrezix
		//ce sont les seuls qui puissent �tre g�r� par l'interface de rezix
	public slots:
		///R�ception de donn�es depuis qRezix
		/** Cette m�thode abstraite doit permettre de r�partir les donn�es entre les diff�rents composant du plug-in.
		 * \param data le type de donn�e que l'on re�oit
		 * \param value la valeur envoy�e par qRezix au plug-in. Le QVariant n'est valide que pendant l'ex�cution de la m�thode, il est donc de la responsabilit� de l'utilisateur que de stocker le r�sultat.
		 */
		virtual void getData(Data data, const QVariant *value) = 0;
		void writeEntry(const QString& name, const QString& value);
		void writeEntry(const QString& name, int value);
		void writeEntry(const QString& name, bool value);
		void writeEntry(const QString& name, const QStringList& value);

	protected slots:
		void sender(const QString& msg);
		void querySender(Data data);
		
		///Fermeture de la fen�tre des propri�t�s par OK
		/** Cette m�thode abstraite va de paire avec la m�thode properties, elle permet d'utiliser la programmation asynchr�ne pour la bo�te de dialogue des propri�t�s. Ce connecteur doit �tre connect� au bouton OK de la bo�te de dialogue */
		virtual void propertiesOK() = 0;
		
		///Fermeture de la fen�tre des propri�t�s par Cancel
		/** Cette m�thode abstraite va de paire avec la m�thode properties, elle permet d'utiliser la programmation asynchr�ne pour la bo�te de dialogue des propri�t�s. Ce connecteur doit �tre connect� au bouton OK de la bo�te de dialogue */
		virtual void propertiesCancel() = 0;

	signals:
		///Demande d'envoi d'un message au serveur
		/** Ce signal demande l'envoi d'un message au serveur. Ce message doit donc �tre reconnu par le serveur.
		 * <br><b>L'UTILISATION DE CETTE FONCTION NE DOIT PAS SURCHARGER LE SERVEUR</b>
		 * \param msg contient le message complet envoy� au serveur
		 */
		void send(const QString& msg);
		///Demande de donn�es � qRezix
		/** Ce signal constitue la seule mani�re qu'� le programme � obtenir des donn�es depuis qRezix, il est � la charge du d�veloppeur de plug-in de faire attention car il s'agit d'un �l�ment asynchrone.
		 * <br>On pourrait imaginer l'utilisation d'une fonction synchr�ne, mais forcer � la programmation asynchr�ne permet d'�tre sur que qRezix garde la main sur le plug-in
		 * \param data indique ce qu'on recherche comme donn�e
		 * \param plugin indique quel plug-in requiert cette donn�e. On a donc n�cessairement <i>plugin = this</i>
		 */
		void queryData(RzxPlugIn::Data data, RzxPlugIn *plugin);
};

#endif

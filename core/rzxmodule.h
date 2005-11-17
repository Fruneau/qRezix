/***************************************************************************
                               rzxmodule.h
        Interface à implémenter pour développer un module pour qRezix
                             -------------------
    begin                : Sat Aug 6 2005
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
#include <RzxBaseModule>

#ifndef RZXMODULE_H
#define RZXMODULE_H

#include <QFlags>

class RzxComputer;

/**
 @author Florent Bruneau
 */

///Structure de base à implémenter pour réaliser un plug-in de qRezix
/** Un plug-in a accès à toutes les fonctions de qRezix, mais il doit tout de
 * même être construit et charger de manière à être reconnu par qRezix.
 * Contrairement à RzxPlugIn de qRezix 1.6 qui implémente en plus d'une structure de travail
 * tout un système de communication entre le plug-in et qRezix, RzxModule ne
 * réalise aucun travail de connexion pour le transfert de données.
 *
 * Ce qu'on pourra tout de même remarqué c'est la présence de signaux dont le but
 * est de permettre d'informer qRezix de demandes d'actions importantes telles que
 * quitter qRezix, demander l'affichage des préférences, ou changer l'état de répondeur.
 *
 * Il est important de noter qu'un module chargé doit implémenter la fonction
 * \ref isInitialised qui permet d'indiquer si l'initialisation c'est bien déroulée
 * un module mal initialisé sera automatiquement déchargé.
 *
 * RzxModule constitue une architecture tout à fait nouvelle de qRezix, beaucoup plus
 * souple et puissante que le RzxPlugIn des version précédentes.
 *
 * L'implémentation de cette classe de fait de la forme suivant :
 * \code
 * ** Dans mymodule.h **
 * #define RZX_BUILTIN (ou RZX_PLUGIN selon les options de compilation)
 *
 *	#include <RzxModule>
 *
 * class MyModule: public RzxModule
 * {
 * 	Q_OBJECT
 * 	public:
 * 		MyModule();
 * 		~MyModule();
 * 		virtual bool isInitialised() const;
 * };
 *
 * ** Dans mymodule.cpp **
 * #include "mymodule.h"
 *
 * RZX_MODULE_EXPORT(MyModule)
 *
 * MyModule::MyModule()
 * {
 * 	beginLoading();
 * 	...
 * 	endLoading();
 * }
 *
 * MyModule::~MyModule()
 * {
 * 	beginClosing();
 * 	...
 * 	endClosing();
 * }
 *
 * bool MyModule::isInitialised() const
 * {
 * 	return true;
 * }
 * \endcode
 *
 * Ensuite, il faut réaliser un fichier de projet avec des options particulières telles que :
 * 	- TEMPLATE = lib
 * 	- CONFIG += plugin
 * 	- TARGET = sprintf(rzx%1, $$TARGET)
 * 	- INCLUDEPATH += (path de qRezix core lib includes)
 * 	- LIBS += -L(path de la lib qrezix) -lqrezix
 */
class Q_DECL_EXPORT RzxModule:public QObject, public RzxBaseModule
{
	Q_OBJECT

	Q_PROPERTY(Type type READ type)
	Q_PROPERTY(QWidget* mainWindow READ mainWindow)
	Q_ENUMS(TypeFlags)
	Q_FLAGS(Type)


	public:
		///Définition du type du module
		/** Tous les modules ne sont pas du même type,
		 * en effet, ils ont ou non une interface graphique
		 * et ont ou non un importance particulière pour l'interface
		 * avec l'utilisateur...
		 */
		enum TypeFlags
		{
			MOD_NONE = 0,
			MOD_GUI = 1, 	/**< Le module utilise une interface graphique. */
			MOD_MAINUI = 2, 	/**< Le module est une interface principale d'intéraction et d'observation pour l'utilisateur.
								 *
								 * Ceci n'inclus pas les modules qui implémente une nouvelle fonctionnalité se basant sur les
								 * données de qRezix, mais uniquement ceux qui établissent une interface permettant d'observer
								 * l'état de connexion.
								 *
								 * Le module doit implémenter les slots \ref toggleVisible, \ref show et \ref hide
								 */

			MOD_CHAT = 4, 	/**< Le module implémente un protocole par défaut de chat utilisable */
			MOD_CHATUI = 8, 	/**< Le module implémentante une interface de chat */
			MOD_PROPERTIES = 16, /**< Le module implémente un protocole par défaut pour obtenir les propriétés */
			MOD_PROPERTIESUI = 32, /**< Le module implémente une interface par défaut pour afficher les propriétés */

			MOD_HIDE = 64, 	/**< Le module a la faculté de demander à cacher l'interface graphique 
								 * Le module dans ce cas doit émettre \ref wantToggleVisible, \ref wantShow et \ref wantHide
								 */
			MOD_MAINGUI = MOD_MAINUI | MOD_GUI, /**< Simple surcharge pour une interface graphique principale */
			MOD_CHATGUI = MOD_CHATUI | MOD_GUI, /**< Simple surcharge pour une interface graphique de chat */
			MOD_PROPGUI = MOD_PROPERTIESUI | MOD_GUI
		};
		Q_DECLARE_FLAGS(Type, TypeFlags)

	private:
		Type m_type;

	//Chargement du module
	protected:
		RzxModule(const QString&, const QString&, int, int, int, const QString& = QString());
		RzxModule(const QString&, const QString&, const Rzx::Version&);
		RzxModule(const QString&, const QString&);

	public:
		~RzxModule();
		virtual bool isInitialised() const = 0;
		const Type &type() const;
		virtual QWidget *mainWindow() const;

	protected slots:
		void setType(const Type&);

	//Communication avec qRezix
	public slots:
		virtual void show();
		virtual void hide();
		virtual void toggleVisible();
		
		virtual void chat(RzxComputer*);
		virtual void properties(RzxComputer*);
		virtual void history(RzxComputer*);

		virtual void showProperties(RzxComputer*);

		///Demande l'envoie d'un message
		/** Ce message n'a pour but d'être utilisé par que les objets de type \ref MOD_CHAT
		 *
		 * \sa receiveChatMessage
		 *
		 * L'implémentation par défaut ne fait rien
		 */
		virtual void sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

		///Averti que le module a reçu un message de type Chat
		/** Ce message ne doit être envoyé qu'aux objets de type \ref MOD_CHATUI
		 *
		 * \sa sendChatMessage
		 *
		 * L'implémentation par défaut ne fait rien
		 */
		virtual void receiveChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

	signals:
		///Demande de fermeture de qRezix
		/** Passer par ce signal permet de réaliser l'arrêt de qRezix proprement
		 * avec en particulier l'enregistrement des paramètres du programme.
		 * l'utilisation de QApplication::quit() ou exit() est fortement déconseillée
		 */
		void wantQuit();

		///Demande l'affichage des préférences du qRezix
		/** Ce signal demande l'affichage des préférences pour que l'utilisateur
		 * puisse faire ses modifications sur sa configuration.
		 *
		 * Une modification future de ce signal risque d'être l'ajout de paramètre
		 * indiquant la page sur laquelle on veut afficher les préférences.
		 */
		void wantPreferences();

		///Demande l'inversion du répondeur
		/** Si l'utilisateur n'est pas sur répondeur ont demande à passer sur répondeur
		 * et vice-versa
		 */
		void wantToggleResponder();

		///Demande l'activation du répondeur
		void wantActivateResponder();

		///Demande la désactivation du répondeur
		void wantDeactivateResponder();

		///Demande l'inversion de l'état d'affichage de l'interface
		/** Si l'interface est affiché on la cache et vice-versa. Ce signal est connecté aux
		 * modules de type \ref MOD_MAINUI si le module a le type \ref MOD_HIDE
		 */
		void wantToggleVisible();

		///Demande l'affichage de l'interface
		/** Ce signal est connecté aux modules de type \ref MOD_MAINUI si
		 * le module a le type \ref MOD_HIDE
		 */
		void wantShow();

		///Demande que l'interface soit cachée
		/** Ce signal est connecté aux modules de type \ref MOD_MAINUI si
		 * le module a le type \ref MOD_HIDE
		 */
		void wantHide();

		///Avertis de la réception de nouvelles propriétés pour un RzxComputer
		/** Ce signal indique que de nouvelles propriétés ont été obtenues pour une
		 * machine. Chacun devra alors utiliser ce signal pour stocker ou afficher les
		 * propriétés correspondante.
		 *
		 * Ce signal doit être émis après le stockage officiel des propriétés.
		 *
		 * \param displayed doit être mis à true si l'information a été interceptée
		 * et communiquée à l'utilisateur ou autre par un objet.
		 */
		void haveProperties(RzxComputer*);
};

///Exportation du module
/** Défini une fonction qui exporte le module. Cette macro doit être appelée pour tout
 * module. Elle sert à la création d'une entité du module en pour les plug-ins.
 *
 * Il est important de définir RZX_PLUGIN si le module est un plugin et RZX_BUILTIN si
 * le module est un builtin avant l'inclusion de RzxModule. Cet appel doit
 * impérativement être réalisé un dans un fichier sources et non un fichier d'en-têtes.
 *
 * Pas besoin de s'inquiéter de la propagation de RZX_PLUGIN et RZX_BUILTIN. Les deux
 * variables sont en effet 'undefined' à la fin de rzxmodule.h sauf si RZX_PRESERVE_MODULETYPE
 * est défini.
 */
#define RZX_MODULE_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getModule, RzxModule, MODULE)

#endif

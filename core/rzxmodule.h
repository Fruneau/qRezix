/***************************************************************************
                               rzxmodule.h
        Interface � impl�menter pour d�velopper un module pour qRezix
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

///Structure de base � impl�menter pour r�aliser un plug-in de qRezix
/** Un plug-in a acc�s � toutes les fonctions de qRezix, mais il doit tout de
 * m�me �tre construit et charger de mani�re � �tre reconnu par qRezix.
 * Contrairement � RzxPlugIn de qRezix 1.6 qui impl�mente en plus d'une structure de travail
 * tout un syst�me de communication entre le plug-in et qRezix, RzxModule ne
 * r�alise aucun travail de connexion pour le transfert de donn�es.
 *
 * Ce qu'on pourra tout de m�me remarqu� c'est la pr�sence de signaux dont le but
 * est de permettre d'informer qRezix de demandes d'actions importantes telles que
 * quitter qRezix, demander l'affichage des pr�f�rences, ou changer l'�tat de r�pondeur.
 *
 * Il est important de noter qu'un module charg� doit impl�menter la fonction
 * \ref isInitialised qui permet d'indiquer si l'initialisation c'est bien d�roul�e
 * un module mal initialis� sera automatiquement d�charg�.
 *
 * RzxModule constitue une architecture tout � fait nouvelle de qRezix, beaucoup plus
 * souple et puissante que le RzxPlugIn des version pr�c�dentes.
 *
 * L'impl�mentation de cette classe de fait de la forme suivant :
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
 * Ensuite, il faut r�aliser un fichier de projet avec des options particuli�res telles que :
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
		///D�finition du type du module
		/** Tous les modules ne sont pas du m�me type,
		 * en effet, ils ont ou non une interface graphique
		 * et ont ou non un importance particuli�re pour l'interface
		 * avec l'utilisateur...
		 */
		enum TypeFlags
		{
			MOD_NONE = 0,
			MOD_GUI = 1, 	/**< Le module utilise une interface graphique. */
			MOD_MAINUI = 2, 	/**< Le module est une interface principale d'int�raction et d'observation pour l'utilisateur.
								 *
								 * Ceci n'inclus pas les modules qui impl�mente une nouvelle fonctionnalit� se basant sur les
								 * donn�es de qRezix, mais uniquement ceux qui �tablissent une interface permettant d'observer
								 * l'�tat de connexion.
								 *
								 * Le module doit impl�menter les slots \ref toggleVisible, \ref show et \ref hide
								 */

			MOD_CHAT = 4, 	/**< Le module impl�mente un protocole par d�faut de chat utilisable */
			MOD_CHATUI = 8, 	/**< Le module impl�mentante une interface de chat */
			MOD_PROPERTIES = 16, /**< Le module impl�mente un protocole par d�faut pour obtenir les propri�t�s */
			MOD_PROPERTIESUI = 32, /**< Le module impl�mente une interface par d�faut pour afficher les propri�t�s */

			MOD_HIDE = 64, 	/**< Le module a la facult� de demander � cacher l'interface graphique 
								 * Le module dans ce cas doit �mettre \ref wantToggleVisible, \ref wantShow et \ref wantHide
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
		/** Ce message n'a pour but d'�tre utilis� par que les objets de type \ref MOD_CHAT
		 *
		 * \sa receiveChatMessage
		 *
		 * L'impl�mentation par d�faut ne fait rien
		 */
		virtual void sendChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

		///Averti que le module a re�u un message de type Chat
		/** Ce message ne doit �tre envoy� qu'aux objets de type \ref MOD_CHATUI
		 *
		 * \sa sendChatMessage
		 *
		 * L'impl�mentation par d�faut ne fait rien
		 */
		virtual void receiveChatMessage(RzxComputer*, Rzx::ChatMessageType, const QString& = QString());

	signals:
		///Demande de fermeture de qRezix
		/** Passer par ce signal permet de r�aliser l'arr�t de qRezix proprement
		 * avec en particulier l'enregistrement des param�tres du programme.
		 * l'utilisation de QApplication::quit() ou exit() est fortement d�conseill�e
		 */
		void wantQuit();

		///Demande l'affichage des pr�f�rences du qRezix
		/** Ce signal demande l'affichage des pr�f�rences pour que l'utilisateur
		 * puisse faire ses modifications sur sa configuration.
		 *
		 * Une modification future de ce signal risque d'�tre l'ajout de param�tre
		 * indiquant la page sur laquelle on veut afficher les pr�f�rences.
		 */
		void wantPreferences();

		///Demande l'inversion du r�pondeur
		/** Si l'utilisateur n'est pas sur r�pondeur ont demande � passer sur r�pondeur
		 * et vice-versa
		 */
		void wantToggleResponder();

		///Demande l'activation du r�pondeur
		void wantActivateResponder();

		///Demande la d�sactivation du r�pondeur
		void wantDeactivateResponder();

		///Demande l'inversion de l'�tat d'affichage de l'interface
		/** Si l'interface est affich� on la cache et vice-versa. Ce signal est connect� aux
		 * modules de type \ref MOD_MAINUI si le module a le type \ref MOD_HIDE
		 */
		void wantToggleVisible();

		///Demande l'affichage de l'interface
		/** Ce signal est connect� aux modules de type \ref MOD_MAINUI si
		 * le module a le type \ref MOD_HIDE
		 */
		void wantShow();

		///Demande que l'interface soit cach�e
		/** Ce signal est connect� aux modules de type \ref MOD_MAINUI si
		 * le module a le type \ref MOD_HIDE
		 */
		void wantHide();

		///Avertis de la r�ception de nouvelles propri�t�s pour un RzxComputer
		/** Ce signal indique que de nouvelles propri�t�s ont �t� obtenues pour une
		 * machine. Chacun devra alors utiliser ce signal pour stocker ou afficher les
		 * propri�t�s correspondante.
		 *
		 * Ce signal doit �tre �mis apr�s le stockage officiel des propri�t�s.
		 *
		 * \param displayed doit �tre mis � true si l'information a �t� intercept�e
		 * et communiqu�e � l'utilisateur ou autre par un objet.
		 */
		void haveProperties(RzxComputer*);
};

///Exportation du module
/** D�fini une fonction qui exporte le module. Cette macro doit �tre appel�e pour tout
 * module. Elle sert � la cr�ation d'une entit� du module en pour les plug-ins.
 *
 * Il est important de d�finir RZX_PLUGIN si le module est un plugin et RZX_BUILTIN si
 * le module est un builtin avant l'inclusion de RzxModule. Cet appel doit
 * imp�rativement �tre r�alis� un dans un fichier sources et non un fichier d'en-t�tes.
 *
 * Pas besoin de s'inqui�ter de la propagation de RZX_PLUGIN et RZX_BUILTIN. Les deux
 * variables sont en effet 'undefined' � la fin de rzxmodule.h sauf si RZX_PRESERVE_MODULETYPE
 * est d�fini.
 */
#define RZX_MODULE_EXPORT(MODULE) RZX_BASEMODULE_EXPORT(getModule, RzxModule, MODULE)

#endif

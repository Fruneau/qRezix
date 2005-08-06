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

#ifndef RZXMODULE_H
#define RZXMODULE_H

#include <QObject>
#include <QString>
#include <QFlags>
#include <QIcon>

/**
 @author Florent Bruneau
 */

///Structure de base à implémenter pour réaliser un plug-in de qRezix
/** Un plug-in a accès à toutes les fonctions de qRezix, mais il doit tout de
 * même être construit et charger de manière à être reconnu par qRezix.
 * Contrairement à RzxPlugIn qui implémente en plus d'une structure de travail
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
 */
class RzxModule:public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool initialised READ isInitialised)
	Q_PROPERTY(QString name READ name)
	Q_PROPERTY(QString description READ description)
	Q_PROPERTY(Version version READ version)
	Q_PROPERTY(QString versionString READ versionString)
	Q_PROPERTY(QIcon icon READ icon)
	Q_PROPERTY(QFlags<TypeFlags> type READ type)

	Q_PROPERTY(QWidget* mainWindow READ mainWindow)

	Q_ENUMS(TypeFlags)
	Q_FLAGS(Type)

	public:
		///Défini la version d'un module
		/** La version est simplement définie de la forme
		 * major.minor.build-tag
		 *
		 * comme par exemple 1.7.0-svn
		 */
		struct Version
		{
			uint major;
			uint minor;
			uint build;
			QString tag;
		};

		///Définition du type du module
		/** Tous les modules ne sont pas du même type,
		 * en effet, ils ont ou non une interface graphique
		 * et ont ou non un importance particulière pour l'interface
		 * avec l'utilisateur...
		 */
		enum TypeFlags
		{
			MOD_GUI = 1, 	/**< Le module utilise une interface graphique. */
			MOD_MAIN = 2, 	/**< Le module est une interface principale d'intéraction et d'observation pour l'utilisateur.
								 *
								 * Ceci n'inclus pas les modules qui implémente une nouvelle fonctionnalité se basant sur les
								 * données de qRezix, mais uniquement ceux qui établissent une interface permettant d'observer
								 * l'état de connexion.
								 *
								 * Le module doit implémenter les slots \ref toggleVisible, \ref show et \ref hide
								 */
			MOD_CHAT = 4, 	/**< Le module implémente le chat. */
			MOD_HIDE = 8, 	/**< Le module a la faculté de demander à cacher l'interface graphique 
								 * Le module dans ce cas doit émettre \ref wantToggleVisible, \ref wantShow et \ref wantHide
								 */
			MOD_MAINUI = MOD_MAIN | MOD_GUI, /**< Simple surcharge pour une interface graphique principale */
			MOD_CHATUI = MOD_CHAT | MOD_GUI 	/**< Simple surcharge pour une interface graphique de chat */
		};
		Q_DECLARE_FLAGS(Type, TypeFlags)

	private:
		QString m_name;
		QString m_description;
		Version m_version;
		QFlags<TypeFlags> m_type;
		QIcon m_icon;

	protected:
		RzxModule(const QString&, const QString&, int, int, int, const QString& = QString());
		RzxModule(const QString&, const QString&, const Version&);
		RzxModule(const QString&, const QString&);

		void beginLoading() const;
		void endLoading() const;
		void beginClosing() const;
		void endClosing() const;

	protected slots:
		void setType(TypeFlags);
		void setIcon(const QIcon&);

	public:
		~RzxModule();
		const QString &name() const;
		const QString &description() const;
		const Version &version() const;
		QString versionString() const;
		const QFlags<TypeFlags> &type() const;
		const QIcon &icon() const;

		virtual bool isInitialised() const = 0;
		virtual QWidget *mainWindow() const;

	public:
		static QString versionToString(const Version&);

	public slots:
		virtual void show();
		virtual void hide();
		virtual void toggleVisible();

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
};

#endif

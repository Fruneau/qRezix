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

#ifndef RZXMODULE_H
#define RZXMODULE_H

#include <QObject>
#include <QString>
#include <QFlags>
#include <QIcon>

/**
 @author Florent Bruneau
 */

///Structure de base � impl�menter pour r�aliser un plug-in de qRezix
/** Un plug-in a acc�s � toutes les fonctions de qRezix, mais il doit tout de
 * m�me �tre construit et charger de mani�re � �tre reconnu par qRezix.
 * Contrairement � RzxPlugIn qui impl�mente en plus d'une structure de travail
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
		///D�fini la version d'un module
		/** La version est simplement d�finie de la forme
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

		///D�finition du type du module
		/** Tous les modules ne sont pas du m�me type,
		 * en effet, ils ont ou non une interface graphique
		 * et ont ou non un importance particuli�re pour l'interface
		 * avec l'utilisateur...
		 */
		enum TypeFlags
		{
			MOD_GUI = 1, 	/**< Le module utilise une interface graphique. */
			MOD_MAIN = 2, 	/**< Le module est une interface principale d'int�raction et d'observation pour l'utilisateur.
								 *
								 * Ceci n'inclus pas les modules qui impl�mente une nouvelle fonctionnalit� se basant sur les
								 * donn�es de qRezix, mais uniquement ceux qui �tablissent une interface permettant d'observer
								 * l'�tat de connexion.
								 *
								 * Le module doit impl�menter les slots \ref toggleVisible, \ref show et \ref hide
								 */
			MOD_CHAT = 4, 	/**< Le module impl�mente le chat. */
			MOD_HIDE = 8, 	/**< Le module a la facult� de demander � cacher l'interface graphique 
								 * Le module dans ce cas doit �mettre \ref wantToggleVisible, \ref wantShow et \ref wantHide
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
};

#endif

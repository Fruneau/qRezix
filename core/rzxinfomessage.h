/***************************************************************************
                          qrezix.h  -  description
                             -------------------
    begin                : lun jan 28 16:27:20 CET 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXINFOMESSAGE_H
#define RZXINFOMESSAGE_H

#include <QDialog>
#include <RzxThemedIcon>

/**
 @author Florent Bruneau
 */

class QSettings;
namespace Ui { class RzxInfoMessageUI; };

///Fenêtre d'information offrant la possibilité de ne pas être réaffichée dans le futur
/** Implémente simplement une fenêtre avec un champs de texte et une icône choisis par
 * l'utilisateur auxquels se rajoute une case à cocher "Ne plus afficher ce message"
 * comme on trouve dans pas mal de programmes...
 *
 * Cette classe fournit une API permettant de gérer facilement l'enregistrement de l'état
 * et tout ce qui va avec l'intégration à qRezix (thèmes, traductions...).
 */
class RZX_CORE_EXPORT RzxInfoMessage:public QDialog
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText)
	Q_PROPERTY(RzxThemedIcon icon READ icon WRITE setIcon)

	Ui::RzxInfoMessageUI *ui;
	RzxThemedIcon m_icon;
	QSettings *settings;
	QString id;

	bool init(QSettings *, const QString&);

	public:
		RzxInfoMessage(QSettings*, const QString&, QWidget* = NULL);
		RzxInfoMessage(QSettings*, const QString&, const QString&, QWidget* = NULL);
		RzxInfoMessage(QSettings*, const QString&, const RzxThemedIcon&, const QString&, QWidget* = NULL);
		~RzxInfoMessage();

		QString text() const;
		RzxThemedIcon icon() const;

	public slots:
		void setIcon(const RzxThemedIcon&);
		void setText(const QString&);

	protected slots:
		void changeTheme();
};

#endif

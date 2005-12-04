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
#include <QSettings>
#include <RzxIconCollection>

#include <RzxInfoMessage>

///Construction
/** Contrairement aux deux autres constructeur, celui-ci n'affiche pas la bo�te de dialogue.
 */
RzxInfoMessage::RzxInfoMessage(QSettings *m_settings, const QString& m_id, QWidget *parent)
	:QDialog(parent)
{
	if(!init(m_settings, m_id))
	{
		deleteLater();
		return;
	}
}

///Construction en d�finissant le texte
/** Cache l'ic�ne au passage
 */
RzxInfoMessage::RzxInfoMessage(QSettings *m_settings, const QString& m_id, const QString& m_text, QWidget *parent)
	:QDialog(parent)
{
	if(!init(m_settings, m_id))
	{
		deleteLater();
		return;
	}
	lblIcon->setVisible(false);
	setText(m_text);
	raise();
	show();
}

///Construction en d�finissant le texte et l'ic�ne
RzxInfoMessage::RzxInfoMessage(QSettings *m_settings, const QString& m_id, const RzxThemedIcon& m_icon, const QString& m_text, QWidget *parent)
	:QDialog(parent)
{
	if(!init(m_settings, m_id))
	{
		deleteLater();
		return;
	}
	setText(m_text);
	setIcon(m_icon);
	raise();
	show();
}

///Enregistre les param�tre et ferme
RzxInfoMessage::~RzxInfoMessage()
{
	if(settings && !id.isEmpty())
		settings->setValue(id, cbDont->isChecked());
}

///Initialise la fen�tre
/** Retourne false si la fen�tre ne doit pas �tre affich�e d'apr�s les informations
 * qui se trouvent dans le fichier de conf
 */
bool RzxInfoMessage::init(QSettings *m_settings, const QString& m_id)
{
	setupUi(this);
	id = m_id;
	settings = m_settings;
	if(settings && !id.isEmpty())
	{
		cbDont->setChecked(settings->value(id).toBool());
		if(settings->value(id).toBool()) return false;
	}

	RzxIconCollection::connect(this, SLOT(changeTheme()));
	cbDont->setChecked(false);
	changeTheme();
	setAttribute(Qt::WA_DeleteOnClose);
	return true;
}

///Retourne le texte
QString RzxInfoMessage::text() const
{
	return lblMessage->text();
}

///Retourne l'ic�ne
RzxThemedIcon RzxInfoMessage::icon() const
{
	return m_icon;
}

///Change l'ic�ne
void RzxInfoMessage::setIcon(const RzxThemedIcon& icon)
{
	lblIcon->setVisible(true);
	m_icon = icon;
	lblIcon->setPixmap(m_icon.icon().pixmap(32));
}

///D�fini le texte � afficher
void RzxInfoMessage::setText(const QString& m_text)
{
	lblMessage->setText(m_text);
}

///Change le th�me d'ic�ne
void RzxInfoMessage::changeTheme()
{
	lblIcon->setPixmap(m_icon.icon().pixmap(32));
	btnOK->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
}

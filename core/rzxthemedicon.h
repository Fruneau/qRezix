/***************************************************************************
                          rzxthemedicon.h  -  description
                             -------------------
    begin                : Thu Aug 11 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : fruneau@melix.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXTHEMEDICON_H
#define RZXTHEMEDICON_H

#include <QIcon>
#include <QObject>

#include <RzxGlobal>

///Ic�ne persistente
/** Cette classe permet de cr�er un objet ic�ne ais�ment cast� en QIcon dont
 * le principal int�r�t est de ne pas avoir a �tre reconstruit � chaque
 * changement de th�me. En effet, le cast vers QIcon est dynamique et
 * s�lectionne de lui m�me le th�me qvb.
 */
class RzxThemedIcon: public QObject
{
	Q_OBJECT

	Q_PROPERTY(Type type READ type)
	Q_PROPERTY(QIcon icon READ icon)

	public:
		enum Type {
			Invalid = 0, /**< objet non valide */
			Name = 1, /**< ic�ne d�finie par le nom de fichier */
			Id = 2, /**< ic�ne d�finie par le Rzx::Icon */
			OS = 3, /**< ic�ne d�finie par le Rzx::SysEx */
			Responder = 4, /**< ic�ne du r�pondeur � 2 �tats */
			Sound = 5, /**< ic�ne du son � 2 �tats */
			OnOff = 6, /**< ic�ne On-Off � 2 �tats */
			Favorite = 7, /**< ic�ne Favoris/Non-Favoris � 2 �tats */
			Ban = 8, /**< ic�ne bann� � 2 �tats */
			Promo = 9, /**< ic�ne de la promo */
			Icon = 10 /**< ic�ne d�finie par un QIcon */
		};

	private:
		Type m_type;
		Rzx::Icon m_icon;
		QString m_name;
		Rzx::SysEx m_sysex;
		Rzx::Promal m_promo;
		QIcon m_qicon;

	public:
		RzxThemedIcon(Rzx::Icon);
		RzxThemedIcon(const QString&);
		RzxThemedIcon(Rzx::SysEx);
		RzxThemedIcon(Rzx::Promal);
		RzxThemedIcon(Type = Invalid);
		RzxThemedIcon(const QIcon&);
		RzxThemedIcon(const RzxThemedIcon&);

		~RzxThemedIcon();

		Type type() const;
		QIcon icon() const;
		bool isValid() const;

		operator QIcon() const;
		const RzxThemedIcon &operator=(const RzxThemedIcon&);
};

///Surcharge pour que le RzxThemedIcon soit utilisable comme un QIcon
inline RzxThemedIcon::operator QIcon() const
{
	return icon();
}

///Indique si l'objet est valide.
/** Si l'object est invalide il sera cast� en une ic�ne nulle
 */
inline bool RzxThemedIcon::isValid() const
{
	return m_type != Invalid;
}

///Retourne le type d'ic�ne
inline RzxThemedIcon::Type RzxThemedIcon::type() const
{
	return m_type;
}

#endif

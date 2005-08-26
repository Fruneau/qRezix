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

///Icône persistente
/** Cette classe permet de créer un objet icône aisément casté en QIcon dont
 * le principal intérêt est de ne pas avoir a être reconstruit à chaque
 * changement de thème. En effet, le cast vers QIcon est dynamique et
 * sélectionne de lui même le thème qvb.
 */
class RzxThemedIcon: public QObject
{
	Q_OBJECT

	Q_PROPERTY(Type type READ type)
	Q_PROPERTY(QIcon icon READ icon)

	public:
		enum Type {
			Invalid = 0, /**< objet non valide */
			Name = 1, /**< icône définie par le nom de fichier */
			Id = 2, /**< icône définie par le Rzx::Icon */
			OS = 3, /**< icône définie par le Rzx::SysEx */
			Responder = 4, /**< icône du répondeur à 2 états */
			Sound = 5, /**< icône du son à 2 états */
			OnOff = 6, /**< icône On-Off à 2 états */
			Favorite = 7, /**< icône Favoris/Non-Favoris à 2 états */
			Ban = 8, /**< icône banné à 2 états */
			Promo = 9, /**< icône de la promo */
			Icon = 10 /**< icône définie par un QIcon */
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
/** Si l'object est invalide il sera casté en une icône nulle
 */
inline bool RzxThemedIcon::isValid() const
{
	return m_type != Invalid;
}

///Retourne le type d'icône
inline RzxThemedIcon::Type RzxThemedIcon::type() const
{
	return m_type;
}

#endif

/***************************************************************************
                          rzxiconcollection  -  description
                             -------------------
    begin                : Sun Jul 24 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXICONCOLLECTION_H
#define RZXICONCOLLECTION_H

#include <QPixmap>
#include <QIcon>
#include <QObject>
#include <QString>
#include <QHash>

#include <RzxGlobal>

class QDir;

/**
@author Florent Bruneau
*/

///Interface simple pour charger les icônes
/** La gestion des icônes nécessite pas mal de flexibilité étant donné qu'elles
 * constitue le centre même de l'interface dans certaines conditions et que les
 * thèmes d'icône sont une part importante de l'interface.
 *
 * Il faut également gérer des problèmes tels que :
 * 	- obligation d'avoir une icône du type donné
 * 	- gestion des thèmes manquants par un système de remplacement
 * 	- ...
 *
 * Cette classe fait partie du coeur de qRezix, dans la catégorie 'Configuration',
 * et n'est qu'une réécriture dans une classe indépendante de RzxConfig de ce
 * qu'on trouvait dans qRezix jusqu'à la version 1.6.x
 */
class RzxIconCollection: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString theme READ theme WRITE setTheme)
	Q_PROPERTY(QStringList themeList READ themeList)
	Q_PROPERTY(QIcon responderIcon READ responderIcon)
	Q_PROPERTY(QIcon soundIcon READ soundIcon)
	Q_PROPERTY(QIcon onOffIcon READ onOffIcon)
	Q_PROPERTY(QIcon favoriteIcon READ favoriteIcon)
	Q_PROPERTY(QIcon banIcon READ banIcon)
	Q_PROPERTY(QPixmap localhostPixmap READ localhostPixmap WRITE setLocalhostPixmap)

	struct IconData
	{
		Rzx::Icon id;
		char *filename;
		bool needed;
	};

	static const IconData data[Rzx::ICON_NUMBER];
	QHash<QString, QDir*> themeDir;
	QHash<QString, QPixmap> icons;
	QHash<quint32, QPixmap> userIcons;
	QString activeTheme;

	RzxIconCollection();
	static RzxIconCollection *object;

	QPixmap hereIcon;
	QPixmap awayIcon;

	public:
		static RzxIconCollection *global();
		virtual ~RzxIconCollection();

		const QString &theme() const;
		QStringList themeList() const;

		//recuperation des icônes à thème
		const QPixmap &pixmap(Rzx::Icon);
		QIcon icon(Rzx::Icon);
		const QPixmap &pixmap(const QString&);
		QIcon icon(const QString&);
		const QPixmap &osPixmap(Rzx::SysEx, bool large = true);
		QIcon osIcon(Rzx::SysEx);
		const QPixmap &promoPixmap(Rzx::Promal);
		QIcon promoIcon(Rzx::Promal);

		//icône à plusieurs états prédéfinies
		QIcon responderIcon();
		QIcon soundIcon();
		QIcon onOffIcon();
		QIcon favoriteIcon();
		QIcon banIcon();

		//gestion des icônes hors-thème : les icônes des personnes
		const QPixmap &hashedIcon(quint32 hash);
		const QPixmap &setHashedIcon(quint32 hash, const QImage& icon);

		//gestion de l'icône de localhost
		QPixmap localhostPixmap();
		void setLocalhostPixmap(const QPixmap&);

		static const QPixmap &getPixmap(Rzx::Icon);

		static const QPixmap& qRezixIcon();
		static const QPixmap& qRezixAwayIcon();
		static QIcon getIcon(Rzx::Icon);
		static QIcon getIcon(const QString&);
		static QIcon getResponderIcon();
		static QIcon getSoundIcon();
		static QIcon getOnOffIcon();
		static QIcon getFavoriteIcon();
		static QIcon getBanIcon();

	protected:
		bool isValid(const QDir&) const;
		QPixmap loadIcon(const QString&, const QString& theme = QString()) const;
	
	public slots:
		void setTheme(const QString&);

	signals:
		void themeChanged(const QString&);
};

///Retourne l'objet global...
inline RzxIconCollection *RzxIconCollection::global()
{
	if(!object)
		object = new RzxIconCollection();
	return object;
}

///Pour simplifier l'écriture...
inline const QPixmap &RzxIconCollection::getPixmap(Rzx::Icon icon)
{ return global()->pixmap(icon); }

inline QIcon RzxIconCollection::getIcon(Rzx::Icon m_icon)
{ return global()->icon(m_icon); }

inline QIcon RzxIconCollection::getIcon(const QString& m_icon)
{ return global()->icon(m_icon); }

inline QIcon RzxIconCollection::getResponderIcon()
{ return global()->responderIcon(); }

inline QIcon RzxIconCollection::getSoundIcon()
{ return global()->soundIcon(); }

inline QIcon RzxIconCollection::getOnOffIcon()
{ return global()->onOffIcon(); }

inline QIcon RzxIconCollection::getFavoriteIcon()
{ return global()->favoriteIcon(); }

inline QIcon RzxIconCollection::getBanIcon()
{ return global()->banIcon(); }

inline const QPixmap& RzxIconCollection::qRezixIcon()
{ return global()->hereIcon; }

inline const QPixmap& RzxIconCollection::qRezixAwayIcon()
{ return global()->awayIcon; }

#endif

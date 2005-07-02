/***************************************************************************
                          rzxitem.h  -  description
                             -------------------
    begin                : Sat Jan 26 2002
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

#ifndef RZXITEM_H
#define RZXITEM_H

#include <QVector>
#include <QPixmap>
#include <Q3ListViewItem>

#include "rzxhostaddress.h"
#include "rzxcomputer.h"

/**
  *@author Sylvain Joyeux
  */

class Q3ListView;
  
class RzxItem : public QObject, public Q3ListViewItem
{
	Q_OBJECT
	friend class RzxRezal;
	
	RzxComputer *getComputer();
	
	public:
		RzxItem(RzxComputer *parent, Q3ListView * view, bool show);
		~RzxItem();
		
		// C caca, mais on se fait pas chier
		int sysex, servers;
		bool gateway,repondeur,ignored;
		QPixmap icon;
	 	int promo;		
		
		QString key(int column, bool ascending) const;
	  	void drawComputerIcon();
		RzxHostAddress ip;

		void setText(int column, const QString& text);
		void setPixmap(int column, const QPixmap& pixmap);

	protected: // Protected methods
		// on sauvegarde les largeurs de colonnes
		// pour detecter un eventuel changement
		// idem, on sauvegarde les valeurs originales
		// des pixmaps et du texte
		void resizeDataVectors(int size);
		QVector<int> colWidth;
		QVector<QString> texts;
		QVector<QStringList> textSplit;
		QVector< QVector<int> > textLengths;
		QVector<QPixmap> pixmaps;
	
		void updatePixmap(int column, int width);
		void updateText(int column, int width, const QFontMetrics& fm);
		
		void paintCell(QPainter * p, const QColorGroup& cg, int column, int width, int align);
		

	public slots: // Public slots
		void update();

	private:
		bool showNotFavorite;
};

inline RzxComputer *RzxItem::getComputer()
{
	return qobject_cast<RzxComputer*>(QObject::parent());
}

#endif

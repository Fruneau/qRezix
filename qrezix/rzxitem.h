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

#include <qpixmap.h>
#include <qlistview.h>
#include "rzxhostaddress.h"

/**
  *@author Sylvain Joyeux
  */

#if (QT_VERSION >= 0x030000)
#include <qptrvector.h>
#include <qmemarray.h>
#else
#include <qvector.h>
#include <qarray.h>
template<class T> class QPtrVector : public QVector<T> {};
template<class T> class QMemArray : public QArray<T> {};
#endif
  
class RzxItem : public QObject {
	Q_OBJECT
	
public:
	class ListViewItem : public QListViewItem	{
	public:
		ListViewItem(QListView * view, RzxItem * item);
		
		// C caca, mais on se fait pas chier
		int sysex, servers;
		bool gateway,repondeur;
		QPixmap icon;
	 	int promo;		
		
		QString key(int column, bool ascending) const;
	  	void drawComputerIcon();
		RzxHostAddress ip;
		
		RzxItem * getItem() const;

		void setText(int column, const QString& text);
		void setPixmap(int column, const QPixmap& pixmap);
	protected: // Protected methods
		// on sauvegarde les largeurs de colonnes
		// pour detecter un eventuel changement
		// idem, on sauvegarde les valeurs originales
		// des pixmaps et du texte
		void resizeDataVectors(int size);
		QMemArray<int> colWidth;
		QPtrVector<QString> texts;
		QPtrVector<QStringList> textSplit;
		QPtrVector< QArray<int> > textLengths;
		QPtrVector<QPixmap> pixmaps;
	
		void updatePixmap(int column, int width);
		void updateText(int column, int width, const QFontMetrics& fm);
		
		void paintCell(QPainter * p, const QColorGroup& cg, int column, int width, int align);
		
		RzxItem * m_item;
	};

	RzxItem(QObject * parent, QListView * view);
	~RzxItem();
public slots: // Public slots
	void update();

private:
	QPixmap * ok, * cancel;
	ListViewItem * item;
};

#endif

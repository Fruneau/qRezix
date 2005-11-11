/***************************************************************************
                          rzxrezalview  -  description
                             -------------------
    begin                : Mon Jul 18 2005
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
#ifndef RZXREZALVIEW_H
#define RZXREZALVIEW_H

#include <QTreeView>
#include <QModelIndex>
#include <QItemSelection>
#include <QRegion>
#include <QItemSelectionModel>
#include <QPoint>

#include <RzxRezal>
#include <RzxRezalSearch>

/**
@author Florent Bruneau
*/

class QMouseEvent;
class QPaintEvent;

///Implémente une visualisation semblable à celle des anciennes version de qRezix
/** Comme pour les versions jusqu'à 1.6, cette classe affiche les objets comme une liste
 * avec l'icône, le nom, les commentaires, les icônes des services, promo...
 */
class RzxRezalView : public QTreeView, public RzxRezal
{
	Q_OBJECT

	RzxRezalSearch search;

	public:
		RzxRezalView(QWidget *parent = 0);
		~RzxRezalView();

		virtual QAbstractItemView *widget();
		virtual QDockWidget::DockWidgetFeatures features() const;
		virtual Qt::DockWidgetAreas allowedAreas() const;
		virtual Qt::DockWidgetArea area() const;
		virtual bool floating() const;
		virtual void updateLayout();

	public slots:
		void afficheColonnes();
		void adapteColonnes();
		virtual void setRootIndex(const QModelIndex&);

	protected:
		virtual void resizeEvent(QResizeEvent * e);
		virtual void mousePressEvent(QMouseEvent *e);
		virtual void mouseDoubleClickEvent(QMouseEvent *e);
		virtual void keyPressEvent(QKeyEvent *e);
		virtual void drawRow(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;

	signals:
		void searchPatternChanged(const QString&);
};

#endif

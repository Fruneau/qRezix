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
#include <QTime>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLVIEW_BUILTIN
#       define RZX_BUILTIN
#else
#       define RZX_PLUGIN
#endif

#include <RzxRezal>
#include <RzxRezalSearch>

/**
@author Florent Bruneau
*/

class QMouseEvent;
class QPaintEvent;
namespace Ui { class RzxRezalViewProp; }

///Implémente une visualisation semblable à celle des anciennes version de qRezix
/** Comme pour les versions jusqu'à 1.6, cette classe affiche les objets comme une liste
 * avec l'icône, le nom, les commentaires, les icônes des services, promo...
 */
class RzxRezalView : public QTreeView, public RzxRezal
{
	Q_OBJECT

	RzxRezalSearch search;
	QTime delayRedraw;
	bool force;

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
		virtual void mouseMoveEvent(QMouseEvent *e);
		virtual void keyPressEvent(QKeyEvent *e);
		virtual void drawRow(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
		virtual void dragEnterEvent(QDragEnterEvent*e);
		virtual void dragMoveEvent(QDragMoveEvent*e);
		virtual void dropEvent(QDropEvent*e);

		virtual void rowsInserted(const QModelIndex&, int, int);

	protected slots:
		void forceRefresh();
		void setDelayRefresh(bool);
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		void findIndex(const QModelIndex&);

	signals:
		void searchPatternChanged(const QString&);

//Préférences
	private:
		Ui::RzxRezalViewProp *ui;
		QWidget *propWidget;
		QList<int> saveColumns;

		void dispColumns(const QList<int>&);

	protected slots:
		virtual void columnOrderChanged();
		QList<int> columnOrder() const;

		virtual void moveDown();
		virtual void moveUp();
		virtual void reinitialisedOrder();

	public:
		virtual QList<QWidget*> propWidgets();
		virtual QStringList propWidgetsName();

	public slots:
		virtual void propInit(bool def = false);
		virtual void propUpdate();
		virtual void propClose();

	protected slots:
		void themeChanged();
};

#endif

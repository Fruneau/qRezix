/***************************************************************************
                         rzxrezaldetail.h  -  description
                             -------------------
    begin                : Sat Aug 13 2005
    copyright            : (C) 2005 Florent Bruneau
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
#ifndef RZXITEM_H
#define RZXITEM_H

#include <QAbstractItemView>
#include <QPointer>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLDETAIL_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif


#include <RzxRezal>

class RzxComputer;
namespace Ui { class RzxItem; };

///ItemView permettant d'afficher le détail
class RzxRezalDetail:public QAbstractItemView, public RzxRezal
{
	Q_OBJECT

	QPointer<RzxComputer> computer;
	QPointer<RzxComputer> waitProp;

	Ui::RzxItem *ui;

	public:
		RzxRezalDetail(QWidget* = NULL);
		~RzxRezalDetail();

		virtual QModelIndex indexAt(const QPoint&) const;
		virtual void scrollTo(const QModelIndex&, ScrollHint = EnsureVisible);
		virtual QRect visualRect(const QModelIndex&) const;

		virtual QAbstractItemView *widget();
		virtual QDockWidget::DockWidgetFeatures features() const;
		virtual Qt::DockWidgetAreas allowedAreas() const;
		virtual Qt::DockWidgetArea area() const;
		virtual bool floating() const;
		
		virtual void updateLayout();

	public slots:
		void clear();
		void drawComputer(RzxComputer*);

	protected:
		virtual void mousePressEvent(QMouseEvent*);
		virtual void mouseMoveEvent(QMouseEvent*);
		virtual void resizeEvent(QResizeEvent*);
		virtual int horizontalOffset() const;
		virtual bool isIndexHidden(const QModelIndex&) const;
		virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
		virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
		virtual int verticalOffset() const;
		virtual QRegion visualRegionForSelection(const QItemSelection&) const;
		virtual void mouseDoubleClickEvent(QMouseEvent *);
		virtual bool viewportEvent(QEvent *e);

	protected slots:
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		virtual void dataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight);
		virtual void rowsAboutToBeRemoved(const QModelIndex&, int, int);
		void propChanged(RzxComputer*, bool*);
		void checkProp();
};

#endif

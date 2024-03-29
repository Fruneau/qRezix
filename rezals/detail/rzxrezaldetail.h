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
#include <QTimer>
#include <QPointer>

#undef RZX_BUILTIN
#undef RZX_PLUGIN
#ifdef RZX_RZLDETAIL_BUILTIN
#	define RZX_BUILTIN
#else
#	define RZX_PLUGIN
#endif


#include <RzxRezal>
#include <RzxRezalAction>

class QSplitter;
class RzxComputer;
namespace Ui {
	class RzxItem;
	class RzxProps;
};

///ItemView permettant d'afficher le d�tail
class RzxRezalDetail:public QAbstractItemView, public RzxRezal
{
	Q_OBJECT

	QPointer<RzxComputer> computer;
	QPointer<RzxComputer> waitProp;

	QTimer timer;

	QSplitter *splitter;
	Ui::RzxItem *uiDetails;
	QWidget *details;
	Ui::RzxProps *uiProps;
	QWidget *props;

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
		virtual QAction* toolButton();
		
		virtual void updateLayout();
		virtual bool eventFilter(QObject*, QEvent*);

	public slots:
		void clear();
		void drawComputer(RzxComputer*);

	protected:
		virtual void mousePressEvent(QMouseEvent*);
		virtual void mouseMoveEvent(QMouseEvent*);
		virtual int horizontalOffset() const;
		virtual bool isIndexHidden(const QModelIndex&) const;
		virtual QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
		virtual void setSelection(const QRect&, QItemSelectionModel::SelectionFlags);
		virtual int verticalOffset() const;
		virtual QRegion visualRegionForSelection(const QItemSelection&) const;
		virtual void mouseDoubleClickEvent(QMouseEvent *);
		virtual bool viewportEvent(QEvent *e);
		virtual void changeEvent(QEvent *e);
		RzxRezalAction::Action action(const QWidget*) const;

	protected slots:
		virtual void resizeEvent(QResizeEvent* = NULL);
		virtual void currentChanged(const QModelIndex&, const QModelIndex&);
		virtual void dataChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight);
		virtual void rowsAboutToBeRemoved(const QModelIndex&, int, int);
		void propChanged(RzxComputer*, bool*);
		void checkProp();
		void redraw();
};

#endif

/***************************************************************************
                          rzxrezalsearch  -  description
                             -------------------
    begin                : Mon Jul 25 2005
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
#ifndef RZXREZALSEARCH_H
#define RZXREZALSEARCH_H

#include <QObject>
#include <QString>
#include <QTime>
#include <QTreeView>
#include <QListView>

#include <RzxRezalModel>

#include "rzxmainuiglobal.h"

/**
@author Florent Bruneau
*/

///Classe fournissant une API simple pour permettre des recherches
class RZX_MAINUI_EXPORT RzxRezalSearch:public QObject
{
	Q_OBJECT
	Q_PROPERTY(QAbstractItemView* view READ view)
	Q_PROPERTY(RzxRezalModel* model READ model)
	Q_PROPERTY(QString pattern READ pattern WRITE setPattern RESET resetPattern)
	Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
	Q_PROPERTY(Mode mode READ mode WRITE setMode)

	QString searchPattern;
	QTime searchTimeout;
	int timeLimit;

	public:
		enum Mode {
			Config = 0, /**> Use the global configuration mode */
			Lite = 1,   /**> Use the light mode (nearest computer name) */
			Full = 2    /**> Use the full mode (all text fields) */
		};
		Q_ENUMS(Mode)
	
	private:
		Mode searchMode;
	
	public:
		RzxRezalSearch(QAbstractItemView *, int timeout = 5000, bool connected = true);
		~RzxRezalSearch();

		QAbstractItemView *view() const;
		RzxRezalModel *model() const;
		const QString &pattern() const;
		int timeout() const;
		Mode mode() const;

	public slots:
		void setPattern(const QString&);
		void addToPattern(const QString&);
		void reducePattern(int size = 1);
		void resetPattern();
		void setTimeout(int);
		void setMode(RzxRezalSearch::Mode);
		void filterView();

	protected:
		template <class T>
		void applyFilter(const QModelIndex&, RzxRezalModel*, T*);
		template <class T>
		void hideIndex(const QModelIndex&, const QModelIndex&, int,  RzxRezalModel*, T*, bool);
		
		bool matches(const QModelIndex&, RzxRezalModel*) const;
		void testTimeout();

	signals:
		void searchPatternChanged(const QString&);
		void findItem(const QModelIndex&);
};

///Applique le filtre à tous les fils du model indiqué
template <class T>
void RzxRezalSearch::applyFilter(const QModelIndex& parent, RzxRezalModel *model, T *view)
{
	const int rows = model->rowCount(parent);
	for(int i = 0 ; i < rows ; i++)
	{
		const QModelIndex index = parent.child(i, 0);
		if(index.isValid())
			hideIndex<T>(index, parent, i, model, view, matches(index, model));
	}
}

///Cache l'index donné
/** Implémentation spécifique pour les QListView
 */
template <>
inline void RzxRezalSearch::hideIndex<QListView>(const QModelIndex& index, const QModelIndex& parent, int i,  RzxRezalModel *model, QListView *view, bool match)
{
	Q_UNUSED(index)
	Q_UNUSED(parent)
	Q_UNUSED(model)
	Q_UNUSED(view)
	view->setRowHidden(i, !match);
}

///Cache l'index donné ou effectue une récursion de applyFilter
/** Spécifique à QTreeView
 */
template <>
inline void RzxRezalSearch::hideIndex<QTreeView>(const QModelIndex& index, const QModelIndex& parent, int i,  RzxRezalModel *model, QTreeView *view, bool match)
{
	if(model->isIndex(index))
		applyFilter<QTreeView>(index, model, view);
	else
		view->setRowHidden(i, parent, !match);
}

#endif

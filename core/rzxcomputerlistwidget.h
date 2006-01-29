/***************************************************************************
      rzxcomputerlistwidget  -  list widget avec drag&drop de RzxComputer
                             -------------------
    begin                : Sun Jan 29 2006
    copyright            : (C) 2006 by Florent Bruneau
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
#ifndef RZXCOMPUTERLISTWIDGET_H
#define RZXCOMPUTERLISTWIDGET_H

#include <QListWidget>
#include <RzxGlobal>

class RzxComputer;
class RzxHostAddress;

/**
 @author Florent Bruneau
 */

///Amélioration de QListWidget pour le drag&drop de RzxComputer
/** Permet de gérer le drag&drop des RzxComputer.
 * Pour ceci on utilise un mimeData de la forme :
 * 	- type : text/plain
 * 	- text : nom1|nom2|nom3|...
 *
 * les noms peuvent être soit des noms DNS, soit des adresses ip
 *
 * Pour récupérer les noms, mimeData cherche tout d'abord si l'utilisateur
 * a stocké des données en data(Qt::UserRole), si c'est le cas, cette
 * donnée est utilisée comme nom, sinon le Qt::DisplayRole est utilisé
 * (équivalent de item->text())
 */
class RZX_CORE_EXPORT RzxComputerListWidget: public QListWidget
{
	Q_OBJECT

	public:
		RzxComputerListWidget(QWidget *);

	protected:
		virtual bool dropMimeData(int, const QMimeData*, Qt::DropAction);
		virtual QMimeData *mimeData(const QList<QListWidgetItem*>) const;
		virtual QStringList mimeTypes() const;

	signals:
		void computerDropped(RzxComputer *);
		void computerDropped(const RzxHostAddress&);
		void computerDropped(const QString&);
		void dropFinished();
};

#endif

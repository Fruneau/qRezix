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
#include <RzxConnectionLister>
#include <RzxComputer>
#include <RzxHostAddress>
#include <QMimeData>

#include <RzxComputerListWidget>

///Constructeur...
RzxComputerListWidget::RzxComputerListWidget(QWidget *parent)
	:QListWidget(parent)
{
}

///Retourne la liste des types utilisables
QStringList RzxComputerListWidget::mimeTypes() const
{
	return QStringList() << "text/plain";
}

///Gestion des ajouts d'items
bool RzxComputerListWidget::dropMimeData(int, const QMimeData *data, Qt::DropAction)
{
	if(!data->hasText()) return false;

	const QStringList items = data->text().split('|');
	foreach(QString name, items)
	{
		const RzxHostAddress ip(name);
		RzxComputer *computer = RzxConnectionLister::global()->getComputerByName(name);
		if(computer)
		{
			emit computerDropped(computer);
			emit computerDropped(name);
		}
		else if(!ip.isNull())
		{
			computer = RzxConnectionLister::global()->getComputerByIP(ip);
			emit computerDropped(ip);
			if(computer)
				emit computerDropped(computer);
		}
		else
			emit computerDropped(name);
	}
	emit dropFinished();
	return true;
}

///Générère un QMimeData pour les objets sélectionnés
QMimeData *RzxComputerListWidget::mimeData(const QList<QListWidgetItem*> items) const
{
	QStringList names;
	foreach(QListWidgetItem *item, items)
		names << item->text();
	QMimeData *data = new QMimeData;
	data->setText(names.join("|"));
	return data;
}

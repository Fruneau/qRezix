/***************************************************************************
                          rzxfilewidget  -  description
                             -------------------
    begin                : Mon Jul 24 2006
    copyright            : (C) 2006 by Guillaume Bandet
    email                : guillaume.bandet@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RZXFILEWIDGET_H
#define RZXFILEWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QToolButton>

class RzxFileWidget : public QWidget
{
    Q_OBJECT

	QProgressBar *progress;
	QLabel *title;
	QLabel *info;
	QToolButton *accept;
	QToolButton *reject;
	QToolButton *cancel;

public slots:
	void emitCancel();
	void emitReject();
	void emitAccept();



public:
    RzxFileWidget(QWidget *parent = 0, QString texte = "", int value = 0, bool modeCancel = true);
	void setTitle(QString texte);
	void setInfo(QString texte);
	void setValue(int value);
	void setModeCancel();
	void setModeAccept();

signals:
	void cancelClicked();
	void acceptClicked();
	void rejectClicked();
};

#endif


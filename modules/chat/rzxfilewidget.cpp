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
#include <QGridLayout>

#include "rzxfilewidget.h"

RzxFileWidget::RzxFileWidget(QWidget *parent, QString texte, int value, bool modeCancel)
    : QWidget(parent)
{
	progress = new QProgressBar(this);
	progress->setAlignment(Qt::AlignCenter);
	progress->setMaximumHeight(20);
	progress->setValue(value);

	title =  new QLabel(this);
	title->setText(texte);

	accept = new QPushButton(tr("Accept"),this);
	accept->setMaximumHeight(25);
	connect(accept,SIGNAL(clicked()),this,SLOT(emitAccept()));
	reject = new QPushButton(tr("Reject"),this);
	reject->setMaximumHeight(25);
	connect(reject,SIGNAL(clicked()),this,SLOT(emitReject()));
	cancel = new QPushButton(tr("Cancel"),this);
	cancel->setMaximumHeight(25);
	connect(cancel,SIGNAL(clicked()),this,SLOT(emitCancel()));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(progress,2,0,1,3);
	layout->addWidget(title,0,0,1,3);
	layout->addWidget(accept,1,0);
	layout->addWidget(cancel,1,1);
	layout->addWidget(reject,1,2);
	setLayout(layout);
	setMaximumSize(100,70);
	setMinimumSize(100,70);

	if(modeCancel)
		setModeCancel();
	else
		setModeAccept();
}

void RzxFileWidget::setTitle(QString texte)
{
	title->setText(texte);
}

void RzxFileWidget::setValue(int value)
{
	progress->setValue(value);
}

void RzxFileWidget::setModeCancel()
{
	cancel->show();
	reject->hide();
	accept->hide();
}

void RzxFileWidget::setModeAccept()
{
	cancel->hide();
	reject->show();
	accept->show();
}

void RzxFileWidget::emitAccept()
{
	emit acceptClicked();
}

void RzxFileWidget::emitCancel()
{
	emit cancelClicked();
}

void RzxFileWidget::emitReject()
{
	emit rejectClicked();
}
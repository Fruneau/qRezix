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

#include <RzxIconCollection>
#include "rzxfilewidget.h"

RzxFileWidget::RzxFileWidget(QWidget *parent, QString texte, int value, bool modeCancel)
    : QWidget(parent)
{
	progress = new QProgressBar(this);
	progress->setAlignment(Qt::AlignCenter);
	progress->setValue(value);

	title =  new QLabel(this);
	title->setText(texte);
	info = new QLabel(this);

	accept = new QToolButton(this);
	accept->setIcon(RzxIconCollection::getIcon(Rzx::ICON_OK));
	accept->setToolTip(tr("Accept the file"));
	connect(accept,SIGNAL(clicked()),this,SLOT(emitAccept()));
	reject = new QToolButton(this);
	reject->setIcon(RzxIconCollection::getIcon(Rzx::ICON_REFUSE));
	reject->setToolTip(tr("Reject the file"));
	connect(reject,SIGNAL(clicked()),this,SLOT(emitReject()));
	cancel = new QToolButton(this);
	cancel->setIcon(RzxIconCollection::getIcon(Rzx::ICON_CANCEL));
	cancel->setToolTip(tr("Cancel the transfer"));
	connect(cancel,SIGNAL(clicked()),this,SLOT(emitCancel()));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(progress,2,0,1,3);
	layout->addWidget(title,0,0,1,3);
	layout->addWidget(info,3,0,1,3);
	layout->addWidget(accept,1,0);
	layout->setAlignment(accept, Qt::AlignHCenter);
	layout->addWidget(cancel,1,1);
	layout->setAlignment(cancel, Qt::AlignHCenter);
	layout->addWidget(reject,1,2);
	layout->setAlignment(reject, Qt::AlignHCenter);
	setLayout(layout);

	if(modeCancel)
		setModeCancel();
	else
		setModeAccept();
}

void RzxFileWidget::setTitle(QString texte)
{
	title->setText(texte);
}

void RzxFileWidget::setInfo(QString texte)
{
	info->setText(texte);
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


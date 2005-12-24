/***************************************************************************
                          rzxsmileyui.cpp  -  description
                             -------------------
    begin                : mer nov 16 2005
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
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextStream>
#include <QFile>

#include "rzxsmileyui.h"

#include "rzxchatconfig.h"
#include "rzxsmileys.h"


RzxSmileyButton::RzxSmileyButton(const QString& text, const QIcon & ic, QWidget * parent )
	:QPushButton(ic, text, parent )
{
	msg = text;
	if(!icon().isNull())
	{
		setToolTip(text);
		setText("");
	}
	connect(this, SIGNAL(clicked(bool)), this, SLOT(wantAdd()));
}

RzxSmileyUi::RzxSmileyUi(QAbstractButton *btn, QWidget *parent)
	:RzxChatPopup(btn, parent)
{
	setFrameStyle(QFrame::WinPanel | QFrame::Raised);
	setWindowTitle(tr("Smileys"));
	
	// chargement de la config
	QGridLayout *smileyLayout = new QGridLayout;
	const QStringList smileys = RzxSmileys::baseSmileyList();
	int rowcpt=0;
	int colcpt=0;
	foreach(QString smiley, smileys)
	{
		RzxSmileyButton *tmp = new RzxSmileyButton(smiley, RzxSmileys::pixmap(smiley), this);
		connect(tmp, SIGNAL(clicked(const QString&)), this, SIGNAL(clickedSmiley(const QString&)));
		smileyLayout->addWidget(tmp, colcpt, rowcpt++);
		if(rowcpt > 4)
		{
			rowcpt = 0;
			colcpt++;
		}
	}

	setLayout(smileyLayout);
	raise();
}


RzxSmileyUi::~RzxSmileyUi()
{
	
}


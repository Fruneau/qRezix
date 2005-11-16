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
#include "rzxsmileyui.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextStream>
#include <QFile>

RzxSmileyButton::RzxSmileyButton(const QString& text, const QIcon & ic, QWidget * parent )
	:QPushButton(ic, text, parent )
{
	msg = text;
	if(!icon().isNull()){
		setToolTip(text);
		setText("");
	}
	connect(this, SIGNAL(clicked(bool)), this, SLOT(wantAdd()));
}

RzxSmileyUi::RzxSmileyUi(QWidget *parent)
#ifdef Q_OS_MAC
	:QFrame(parent, Qt::Drawer)
#else
	:QFrame(parent, Qt::Window | Qt::FramelessWindowHint)
#endif
{
	setAttribute(Qt::WA_DeleteOnClose);
	setFrameStyle(QFrame::WinPanel | QFrame::Raised);
	setWindowTitle(tr("Smileys"));
	
	// chargement de la config
	QString filename = "resources/smileys/basic/basic.theme";
	QGridLayout *smileyLayout = new QGridLayout;
	int rowcpt=0;
	int colcpt=0;
	if (!filename.isNull()){
		QString text;
		QFile file(filename);
		if(file.exists()){
			file.open(QIODevice::ReadOnly);
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			while(!stream.atEnd()) {
				text = stream.readLine();
				QStringList list = text.split("###");
				if(list.count() == 2){
					QStringList rep = list[0].split("$$");
					RzxSmileyButton *tmp = new RzxSmileyButton(rep[0],QIcon(list[1]),this);
					connect(tmp,SIGNAL(clicked(QString)), this, SIGNAL(clickedSmiley(QString)));
					smileyLayout->addWidget(tmp,colcpt,rowcpt++);
					if(rowcpt > 4){
						rowcpt = 0;
						colcpt++;
					}
				}
			}
			file.close();
		}
	}
	setLayout(smileyLayout);
	raise();
}


RzxSmileyUi::~RzxSmileyUi()
{
	
}
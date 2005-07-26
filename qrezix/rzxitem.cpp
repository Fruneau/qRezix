/***************************************************************************
                          rzxitem.cpp  -  description
                             -------------------
    begin                : Sat Jan 26 2002
    copyright            : (C) 2002 by Sylvain Joyeux
    email                : sylvain.joyeux@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QBitmap>
#include <QImage>
#include <QBrush>
#include <QPainter>
#include <QFont>
#include <QPixmap>

#include "rzxitem.h"

#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxrezal.h"
#include "rzxconfig.h"

RzxItem::RzxItem(RzxComputer *parent, Q3ListView * view, bool show)
	 : QObject(parent), Q3ListViewItem(view), showNotFavorite(show)
{
	RzxComputer* computer = getComputer();
	//Le Q_ASSERT est quand m�me super violent ici... � supprimer � mon avis
	Q_ASSERT(computer != NULL);

	ip = computer->ip();
	repondeur =  computer->repondeur();
	ignored = RzxConfig::global()->isBan(*computer);
	promo =  computer->promo();

	setVisible(show || RzxConfig::global()->isFavorite(*computer));
}

RzxItem::~RzxItem(){
}

//import� de rzxrezal.cpp
	// Numero des colonnes
//	enum NumColonne
//		{ ColIcone = 0, ColNom = 1, ColRemarque = 2, ColSamba = 3, ColFTP = 4,
//		  /*ColHotline = 5,*/ ColHTTP = 5, ColNews = 6, ColOS = 7, ColGateway = 8,
//		  ColPromo = 9, numColonnes = 10 };

/** No descriptions */
void RzxItem::update(){
	RzxComputer *computer = getComputer();
	if (!computer)
		return;

	icon = computer->icon();
	drawComputerIcon();
	setText(RzxRezal::ColNom, computer->name());
	setText(RzxRezal::ColRemarque, computer->remarque());
	setText(RzxRezal::ColIP, computer->ip().toString());
	setText(RzxRezal::ColClient, computer->client());
	setText(RzxRezal::ColResal, computer->rezal());
	
	QVector<QPixmap*> yesno = RzxConfig::yesnoIcons();
	int imgIdx, base = RzxRezal::ColSamba, codeIdx, mask = 1;
	/*MAINTENANT*/ /* --> tr�s sale d'apr�s prout@steak */
	for (codeIdx = 0; codeIdx < 5; codeIdx++) {
		if(codeIdx!=2) {
			imgIdx = computer->servers() & mask ? 1 : 0;
			int code=codeIdx+base +(codeIdx>2 ? -1: 0);
			setPixmap(code, *yesno[imgIdx*5+codeIdx]);
		}
		mask=mask<<1;
	}
	
	QVector<QPixmap *> os = RzxConfig::osIcons();
	setPixmap(RzxRezal::ColOS, *os[computer->sysEx()]);
	
	gateway = computer->isSameGateway();
	QVector<QPixmap*> l_gateway = RzxConfig::gatewayIcons();
	// gateway[0] contient l'icone qu'il faut afficher si les deux passerelles sont !=
	// gateway[1] contient celle lorsqu'elles sont identiques.
	if(!l_gateway[gateway?1:0]) qDebug(QString("No gateway pixmap for %1").arg(gateway));
	setPixmap(RzxRezal::ColGateway, *(l_gateway[gateway ? 1 : 0]));

	sysex = computer->sysEx();
	servers = computer->servers();
	repondeur = (computer->repondeur() == RzxComputer::REP_ON || computer->repondeur() == RzxComputer::REP_REFUSE);
	if(sysex < 3)
		sysex += 7;

	setPixmap(RzxRezal::ColPromo, *RzxConfig::promoIcons()[computer->promo()]);

	setup();
}

/** No descriptions */
QString RzxItem::key(int column, bool ascending) const{
	QString prefix;
	 bool test;

	switch(column) {
		case RzxRezal::ColIcone: case RzxRezal::ColNom:
			return text(1).lower();
		case RzxRezal::ColSamba: case RzxRezal::ColFTP:
			test = (servers >> (column - 3)) & 1;
			prefix += QString::number(!test);
			return prefix + text(1).lower();
		case RzxRezal::ColHTTP: case RzxRezal::ColNews:
			test = (servers >> (column - 2)) & 1;
			prefix += QString::number(!test);
			return prefix + text(1).lower();
		case RzxRezal::ColOS:
			prefix += QString::number(sysex);
			return prefix + text(1).lower();
		case RzxRezal::ColGateway:
			prefix += gateway ? "0" : "1";
			return prefix + text(1).lower();
		case RzxRezal::ColPromo:
			prefix+= QString::number(promo);
			return prefix + text(1).lower();
		
		default:
			return Q3ListViewItem::key(column, ascending);
	};
}

/** No descriptions */
void RzxItem::drawComputerIcon(){
	QPixmap tempIcon;
	if(!icon.isNull() && !RzxConfig::computerIconSize() && (!isSelected() || !RzxConfig::computerIconHighlight()))
		tempIcon = icon.scaled(32,32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	else
		tempIcon = icon;
	setPixmap(0, tempIcon);
}

void RzxItem::resizeDataVectors(int size) {
	colWidth.resize(size);
	pixmaps.resize(size);
	texts.resize(size);
	textSplit.resize(size);
	textLengths.resize(size);
}


void RzxItem::updatePixmap(int column, int width) {
	if (pixmaps.size() <= column)
		resizeDataVectors(column + 1);
	const QPixmap *pix = pixmap(column);	
	if(!pix) return;
	
	QPixmap &tempPix = pixmaps[column];
	
	// redimensionnement de l'image a afficher
	int w = pix -> width(),
		h = pix -> height();
	if (width < pix -> width()) w = width;
	if (height() < pix -> height()) h = height();
	if (tempPix.height() != h || tempPix.width() != w)
	{
		tempPix = *pix;
		tempPix.resize(w,h);
	}
}

void RzxItem::updateText(int column, int width, const QFontMetrics& fm) {
	if ((int)texts.size() <= column) resizeDataVectors(column + 1);
	QString str = text(column);
	if (str.isEmpty()) return;
	
	int strWidth = fm.width(str);
	
	QString &text = texts[column];

	if (width < strWidth) {
		text = "";
		if(!textSplit[column].count()) {
			textSplit.insert(column, QStringList::split(' ', str, true));

			const QStringList &split = textSplit[column];
			textLengths.insert(column, QVector<int>(split.count()));
			
			int wordIdx = 0;
			QString line;
			QVector<int> &lengths = textLengths[column];
			foreach(QString it, split)
			{
				if (!line.isEmpty()) line += " ";
				line = line + it;
				lengths[wordIdx++] = fm.width(line);
			}
		}
		
		const QStringList &split = textSplit[column];
		QVector<int> &lengths = textLengths[column];
		QString line;
		int curLength = 0;
		int wordIdx = 0;
		for(QStringList::ConstIterator it = split.begin() ; it != split.end() ; )
		{
			line = "";
			do
			{
				if (!line.isEmpty()) line += " ";
				line = line + *it;
			}
			while (++it != split.end() && lengths[++wordIdx] - curLength < width);
			if (it != split.end())
				curLength = lengths[wordIdx - 1];
			else 
				curLength = lengths[wordIdx];
			text += "\n" + line;
		}
	}
	// end of if (width < strWidth)
	else
		text = str;
}

/** No descriptions */
void RzxItem::paintCell(QPainter * p, const QColorGroup& cg, int column, int width, int align){
	// ATTENTION: on considere que TOUTES les images sont centrees (on ne prend pas en compte align)
	if (!width) return;
	int height = Q3ListViewItem::height();
	
	QColor backgroundColor;
	QColor textColor;
	if (ignored) {
		backgroundColor = QColor(RzxConfig::ignoredBGColor());
		textColor = QColor(RzxConfig::ignoredText());
	} else {
		if (repondeur) {
			backgroundColor = isSelected() ? 
				QColor(RzxConfig::repondeurHighlight())
				: QColor(RzxConfig::repondeurBase());
			textColor = isSelected() ?
				QColor(RzxConfig::repondeurHighlightedText())
				: QColor(RzxConfig::repondeurNormalText());
		} else { 
			backgroundColor = isSelected() ? cg.highlight() : cg.base(); 
			textColor = isSelected() ? cg.highlightedText() : cg.text();
		}
	}
	
	p->setBackgroundColor(QBrush(backgroundColor));
	p->setPen(QPen(textColor));
	p->fillRect(0, 0, width, height, p->background());
	
	// il y en a qui n'ont pas leur rezix a jour
	// => en particulier, ils n'ont pas de promo
	// et la colonne 10 n'est pas gérée par colWidth
	// vu que ca peut se reproduire dans de prochaines
	// maj de qrezix, on gere ici
	if(column >= (int) colWidth.size()) return;
	
	if(width != colWidth[column]) {
		updatePixmap(column, width);
		updateText(column, width, p -> fontMetrics());
	}
	colWidth[column] = width;
	
	const QPixmap &pix = pixmaps[column];
	const QString &txt = texts[column];
	
	if(pix.isNull() && txt.isEmpty())
		return;
	
	if(pix.isNull()) 
	{
		QFont font = p->font();
		font.setBold(isSelected() && column == RzxRezal::ColNom);
#ifndef WIN32 //Parce que le texte en italique passe mal sous windows
		font.setItalic(repondeur || ignored);
#endif
		p->setFont(font);
		p->setBackgroundMode(Qt::OpaqueMode);
		p->drawText(0, 0, width, height, align, txt);
		return;
	}
	
	// on dessine l'image
	int x,y;
	x = (width - pix.width()) / 2;
	y = (height - pix.height()) / 2;
	p->drawPixmap(x, y, pix);
}

void RzxItem::setText(int column, const QString& text) {
	Q3ListViewItem::setText(column, text);
	if(textSplit.size() <= column)
		resizeDataVectors(column + 1);
	colWidth[column] = 0;
	
	if (text.isEmpty()) {
		textSplit.remove(column);
		texts.remove(column);
	} 
}

void RzxItem::setPixmap(int column, const QPixmap& pix) {
	Q3ListViewItem::setPixmap(column, pix);
	if(pixmaps.size() <= column)
		resizeDataVectors(column + 1);
	colWidth[column] = 0;
	pixmaps[column] = pix;
}

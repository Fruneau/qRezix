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
#include <qimage.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qfont.h>

#include "rzxitem.h"

#include "rzxcomputer.h"
#include "rzxserverlistener.h"
#include "rzxrezal.h"
#include "rzxconfig.h"

RzxItem::RzxItem(RzxComputer *parent, QListView * view, bool show)
	 : QObject(parent), QListViewItem(view)
{
	
	
	pixmaps.setAutoDelete(true);
	texts.setAutoDelete(true);
	textLengths.setAutoDelete(true);
	textSplit.setAutoDelete(true);
	
	showNotFavorite = show;
	
	RzxComputer* computer = getComputer();
	Q_ASSERT(computer != NULL);
	ip = computer->getIP();
	repondeur =  computer->getRepondeur();
	promo =  computer->getPromo();
	setVisible(show || RzxConfig::globalConfig()->favorites->find( computer->getName()));
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

	icon = computer  -> getIcon();
	drawComputerIcon();
	setText(RzxRezal::ColNom, computer -> getName());
	setText(RzxRezal::ColRemarque, computer -> getRemarque());
	
	RzxComputer::options_t options = computer -> getOptions();
	QArray<QPixmap *> yesno = RzxConfig::yesnoIcons();
	int imgIdx, base = RzxRezal::ColSamba, codeIdx, mask = 1;
	//TRES SALE mais dsl sinon faut changer le serveur .. prout@steak
	/*AVANT*//*for (codeIdx = 0; codeIdx < 5; codeIdx++) {
		imgIdx = options.Server & mask ? 1 : 0;
		item -> setPixmap(codeIdx + base, *yesno[imgIdx*5+codeIdx]);
		mask=mask<<1;
	}*/
	/*MAINTENANT*/
	for (codeIdx = 0; codeIdx < 5; codeIdx++) {
		if(codeIdx!=2) {
			imgIdx = options.Server & mask ? 1 : 0;
			int code=codeIdx+base +(codeIdx>2 ? -1: 0);
			setPixmap(code, *yesno[imgIdx*5+codeIdx]);
		}
		mask=mask<<1;
	}
	
	QArray<QPixmap *> os = RzxConfig::osIcons();
	setPixmap(RzxRezal::ColOS, *os[(int) options.SysEx]);
	
	RzxHostAddress ip = RzxServerListener::object() -> getIP();
	gateway = ip.sameGateway(computer -> getIP());
	QArray<QPixmap *> l_gateway = RzxConfig::gatewayIcons();
	// gateway[0] contient l'icone qu'il faut afficher si les deux passerelles sont !=
	// gateway[1] contient celle lorsqu'elles sont identiques.
	if(!l_gateway[gateway?1:0]) qDebug(QString("No gateway pixmap for %1").arg(gateway));
	setPixmap(RzxRezal::ColGateway, *(l_gateway[gateway ? 1 : 0]));

	sysex = options.SysEx;
	servers = options.Server;
	repondeur = (options.Repondeur==RzxComputer::REP_ON);
	if (sysex < 3) sysex += 6;

	int promo=options.Promo;
	QArray<QPixmap *> promos = RzxConfig::promoIcons();
	if(promo==RzxComputer::PROMAL_ORANGE)
		setPixmap(RzxRezal::ColPromo,*promos[0]);
	else if(promo==RzxComputer::PROMAL_ROUJE)
		setPixmap(RzxRezal::ColPromo,*promos[1]);
	else if(promo==RzxComputer::PROMAL_JONE)
		setPixmap(RzxRezal::ColPromo,*promos[2]);
		
	setup();
	setVisible(showNotFavorite || RzxConfig::globalConfig()->favorites->find((computer -> getName())));
	
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
			return QListViewItem::key(column, ascending);
	};
}
/** No descriptions */
void RzxItem::drawComputerIcon(){
	QPixmap tempIcon = icon;
	if(!icon.isNull() && !RzxConfig::computerIconSize() && (!isSelected() || !RzxConfig::computerIconHighlight()))
	{
		QImage img = icon.convertToImage();
		if(!img.isNull())
		{
			img = img.smoothScale(32, 32);
			tempIcon.convertFromImage(img);
		}
	}
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
	if ((int)pixmaps.size() <= column) resizeDataVectors(column + 1);
	const QPixmap * pix = pixmap(column);	
	if (!pix) return;
	
	QPixmap * tempPix = pixmaps[column];
	
	// redimensionnement de l'image a afficher
	int w = pix -> width(), h = pix -> height();
	if (width < pix -> width()) w = width;
	if (height() < pix -> height()) h = height();
	if (tempPix -> height() != h || tempPix -> width() != w) {
		*tempPix = *pix;
		tempPix -> resize(w,h);
	}
}

void RzxItem::updateText(int column, int width, const QFontMetrics& fm) {
	if ((int)texts.size() <= column) resizeDataVectors(column + 1);
	QString str = text(column);
	if (str.isEmpty()) return;
	
	int strWidth = fm.width(str);
	
	QString * text = texts[column];

	if (width < strWidth) {
		*text = "";
		const QStringList * split = textSplit[column];
		QArray<int> * lengths = textLengths[column];
		if (!split) {
			textSplit.insert(column, new QStringList(QStringList::split(' ', str, true)));
			split = textSplit[column];
			
			textLengths.insert(column, new QArray<int>(split -> count()));
			lengths = textLengths[column];
			
			int wordIdx = 0;
			QString line;
			QStringList::ConstIterator it;
			for (it = split -> begin(); it != split -> end(); ++it) {
				if (!line.isEmpty()) line += " ";
				line = line + *it;
				(*lengths)[wordIdx++] = fm.width(line);
			}
		}
		
		QStringList::ConstIterator it (split -> begin());
				
		QString line;
		int curLength = 0;
		int wordIdx = 0;
		while (it != split -> end()) {
			line = "";
			do {
				if (!line.isEmpty()) line += " ";
				line = line + *it;
			} while (++it != split -> end() && (*lengths)[++wordIdx] - curLength < width);
			if (it != split -> end())
				curLength = (*lengths)[wordIdx - 1];
			else 
				curLength = (*lengths)[wordIdx];
			*text += "\n" + line;
		}
	} else {
		*text = str;
	}
}

/** No descriptions */
void RzxItem::paintCell(QPainter * p, const QColorGroup& cg, int column, int width, int align){
	// ATTENTION: on considere que TOUTES les images sont centrees (on ne prend pas en compte align)
	if (!width) return;
	int height = QListViewItem::height();
	
	QColor backgroundColor;
	QColor textColor;
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
	
	p -> setBackgroundColor(backgroundColor);
	p -> setPen(QPen(textColor));
	p -> fillRect(0, 0, width, height, QBrush(backgroundColor));
	
	// il y en a qui n'ont pas leur rezix a jour
	// => en particulier, ils n'ont pas de promo
	// et la colonne 10 n'est pas gérée par colWidth
	// vu que ca peut se reproduire dans de prochaines
	// maj de qrezix, on gere ici
	if (column >= (int) colWidth.size()) return;
	
	if (width != colWidth[column]) {
		updatePixmap(column, width);
		updateText(column, width, p -> fontMetrics());
	}
	colWidth[column] = width;
	
	const QPixmap * pix = pixmaps[column];
	const QString * txt = texts[column];
	
	if (!pix && !txt) {
		return;
	}
	
	if (!pix) 
	{
		QFont font = p->font();
		font.setBold(isSelected() && column == RzxRezal::ColNom);
#ifndef WIN32 //Parce que le texte en italique passe mal sous windows
		font.setItalic(repondeur);
#endif
		p->setFont(font);
		p -> setBackgroundMode(OpaqueMode);
		p -> drawText(0, 0, width, height, align, *txt);
		return;
	}
	
	// on dessine l'image
	int x,y;
	x = (width - pix -> width()) / 2;
	y = (height - pix -> height()) / 2;
	p -> drawPixmap(x, y, *pix);
}

void RzxItem::setText(int column, const QString& text) {
	QListViewItem::setText(column, text);
	if ((int) textSplit.size() <= column) resizeDataVectors(column + 1);
	colWidth[column] = 0;
	
	if (text.isEmpty()) {
		textSplit.remove(column);
		texts.remove(column);
		return;
	} 

	if (!texts[column]) texts.insert(column, new QString);
}

void RzxItem::setPixmap(int column, const QPixmap& pix) {
	QListViewItem::setPixmap(column, pix);
	if ((int) pixmaps.size() <= column) resizeDataVectors(column + 1);
	colWidth[column] = 0;
	
	if (pix.isNull()) {
		pixmaps.remove(column);
		return;
	} 
	
	if (!pixmaps[column]) pixmaps.insert(column, new QPixmap);
	*pixmaps[column] = pix;
}

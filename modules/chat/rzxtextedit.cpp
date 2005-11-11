/***************************************************************************
                       rzxtextedit  -  description
                             -------------------
    begin                : Fri Nov 11 2005
    copyright            : (C) 2005 Florent Bruneau
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
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextBlock>

#include <RzxComputer>

#include "rzxtextedit.h"

#include "rzxchat.h"

/***************************************************
* RzxTextEdit::ListText
***************************************************/
RzxTextEdit::ListText::ListText(QString t, ListText * pN)
{
	texte = t;
	pNext = pN;
	pPrevious = 0;
	if(pNext != 0)
		pNext -> pPrevious = this;
}

RzxTextEdit::ListText::~ListText()
{
	delete pNext;
}


/***************************************************
* RzxTextEdit
***************************************************/
///Construction
RzxTextEdit::RzxTextEdit(QWidget *parent)
	:QTextEdit(parent), chat(NULL)
{
	curLine = history = NULL;
}

///Destruction
RzxTextEdit::~RzxTextEdit()
{
	curLine = NULL;
	delete history;
}

///Interception de la frappe au clavier
void RzxTextEdit::keyPressEvent(QKeyEvent *e)
{
	QKeyEvent * eMapped=e;
//	bool down=false;
//	int para, index, line;
	
	//Saut de ligne - Envoie du message
	switch(eMapped->key())
	{
		case Qt::Key_Enter: case Qt::Key_Return:
			if(eMapped->modifiers() != Qt::SHIFT)
			{
				emit enterPressed();
				break;
			}
			//eMapped =new QKeyEvent(QEvent::KeyRelease, Qt::Key_Enter, e->text(), e->modifiers());
			QTextEdit::keyPressEvent(eMapped);
			break;

		//Autocompletion
		case Qt::Key_Tab:
			//Pour que quand on appuie sur tab ça fasse la complétion du nick
			if(!nickAutocompletion())
			{
				QTextEdit::keyPressEvent(eMapped);
				onTextEdited();
			}
			break;
	
	//Parcours de l'historique
/*	case Qt::Key_Down: 
		down=true;
	case Qt::Key_Up:
		//Pour pouvoir éviter d'avoir à appuyer sur Shift ou Ctrl si on est à l'extrémité de la boite
		getCursorPosition(&para, &index);
		line = lineOfChar(para, index);
		for(int i = 0 ; i<para ; i++) line += linesOfParagraph(i);
		
		//Et op, parcours de l'historique si les conditions sont réunies
		if((eMapped->state() & Qt::ShiftButton) || (eMapped->state() & Qt::ControlButton) || (down && (line == lines()-1)) || (!down && !line)) {
			onArrowPressed(down);
			break;
		}
		eMapped =new QKeyEvent(QEvent::KeyRelease, e->key(), e->ascii(), e->state());
*/	
	//Texte normal
	default:
		QTextEdit::keyPressEvent(eMapped);
		onTextEdited();
	}
}

///Fait une completion automatique du nick au niveau du curseur.
/** La completion n'est réalisée que si aucune sélection n'est actuellement définie */
bool RzxTextEdit::nickAutocompletion()
{
	QTextCursor cursor = textCursor();
	
	//Si y'a une sélection, on zappe
	if(cursor.hasSelection())
		return false;
	
	//On récupère la position du curseur et la paragraphe concerné
	int index = cursor.position();
	index -= cursor.block().position();
	if(!index) return false;
	
	QRegExp mask("[^-A-Za-z0-9]([-A-Za-z0-9]+)$");
	QString textPara = cursor.block().text();
	
	//Juste pour se souvenir des pseudos possibles
	QString localName = RzxComputer::localhost()->name();
	QString remoteName = chat->computer()->name();
	
	for(int i = 1 ; i <= index && (localName.length() > i || remoteName.length() > i) ; i++)
	{
		//Chaine de caractère qui précède le curseur de taille i
		QString nick = textPara.mid(index-i, i);
		
		if(mask.indexIn(nick) != -1 || i == index)
		{
			if(mask.indexIn(nick) != -1) nick = mask.cap(1);
			if(!remoteName.indexOf(nick, false) && localName.indexOf(nick, false))
			{
				cursor.insertText(remoteName.right(remoteName.length()-nick.length()) + " ");
				return true;
			}
			else if(remoteName.indexOf(nick, false) && !localName.indexOf(nick, false))
			{
				cursor.insertText(localName.right(localName.length()-nick.length()) + " ");
				return true;
			}
			return false;
		}
	}
	return false;
}

///Pacours de l'historique
void RzxTextEdit::onArrowPressed(bool down)
{
	if(history==0)
		return;
	ListText * newCur=0;
	if(down)
		newCur = curLine->pPrevious;
	else
		newCur = curLine->pNext;
	if(!newCur)
		newCur = history;
	setHtml(newCur->texte);
	curLine = newCur;
}

///En cas d'édition du texte, on met à jour l'historique
void RzxTextEdit::onTextEdited()
{
	emit textWritten();

	if(!history)
	{
		history = new ListText(toHtml(), 0);
		curLine = history;
		return;
	}
	history->texte = toHtml();
}

///Valide le contenue
/** La validation consiste à l'envoie du contenu de la fenêtre
 * donc on incrémente l'historique et on vide la fenêtre.
 */
void RzxTextEdit::validate()
{
	history = new ListText(toHtml(), history);
	curLine = history;
	clear();
}

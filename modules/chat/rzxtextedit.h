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
#ifndef RZXTEXTEDIT_H
#define RZXTEXTEDIT_H

#include <QTextEdit>
#include <QTextLine>

class RzxChat;

/**
 @author Florent Bruneau
 */

///Pour customiser la boîte d'édition
/** Rajout de Stéphane Lescuyer 2004/05/27 */
class RzxTextEdit : public QTextEdit 
{
	Q_OBJECT
	
	friend class RzxChat;
	RzxChat *chat;

	///Liste chaînée simple et rapide pour gérer l'historique
	class ListText
	{
		public:
			ListText * pPrevious;
			QString texte;
			ListText * pNext;
			
		public:	
			ListText(QString t, ListText * pN);
			~ListText();
	};

	ListText *history;
	ListText *curLine;

	QTextLine currentTextLine() const;
	bool atBeginning() const;
	bool atEnd() const;

public:
	RzxTextEdit(QWidget *parent=0);
	~RzxTextEdit();

public slots:
	void validate();
	void setBold(bool);
	void setItalic(bool);
	void setColor(const QColor&);
	void setFont(const QString&);
	void setSize(int);
	void setUnderline(bool);
	void useHtml(bool);
	
protected:
	void setChat(RzxChat*);
	void keyPressEvent(QKeyEvent*);
	bool nickAutocompletion();

	void onArrowPressed(bool);
	void onTextEdited();

signals:
	void enterPressed();
	void textWritten();
};

///Défini la fenêtre d'attachement
inline void RzxTextEdit::setChat(RzxChat *m_chat)
{ chat = m_chat; }

///Passe l'édition en gras
/** Cette fonction est implémentée car elle a disparu avec le
 * passage en Qt4... ce qui est d'ailleurs dommage
 *
 * Le code est extrait directement du code de la partie Qt3Support de QTextEdit
 */
inline void RzxTextEdit::setBold(bool bold)
{
	setFontWeight(bold ? QFont::Bold : QFont::Normal);
}

///Passe l'édition en italique
inline void RzxTextEdit::setItalic(bool ital)
{
	setFontItalic(ital);
}

///Passe l'édition en souligné
inline void RzxTextEdit::setUnderline(bool under)
{
	setFontUnderline(under);
}

///Change la police
inline void RzxTextEdit::setFont(const QString& font)
{
	setFontFamily(font);
}

///Change la taille de la police
inline void RzxTextEdit::setSize(int size)
{
	setFontPointSize(size);
}

///Change la couleur de la police
inline void RzxTextEdit::setColor(const QColor& c)
{
	setTextColor(c);
}

///Change le formatage du texte
inline void RzxTextEdit::useHtml(bool html)
{
	if(!html)
	{
#ifdef WIN32
//parce que Terminal n'existe pas sous Win !
		setFont("Arial");
		setSize(8);
#else
		setFont("Terminal");
		setSize(11);
#endif
		setBold(false);
		setItalic(false);
		setUnderline(false);
		setPlainText(toPlainText());
	}
}

#endif

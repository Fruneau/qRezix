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

public:
	RzxTextEdit(QWidget *parent=0);
	~RzxTextEdit();

public slots:
	void validate();
	
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

inline void RzxTextEdit::setChat(RzxChat *m_chat)
{ chat = m_chat; }

#endif

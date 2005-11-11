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
	
public:
	RzxTextEdit(QWidget *parent=0);
	~RzxTextEdit();
	
protected:
	void setChat(RzxChat*);
	void keyPressEvent(QKeyEvent*);
	bool nickAutocompletion();

signals:
	void enterPressed();
	void arrowPressed(bool);
	void textWritten();
};

inline RzxTextEdit::RzxTextEdit(QWidget *parent)
	:QTextEdit(parent), chat(NULL) { }

inline void RzxTextEdit::setChat(RzxChat *m_chat)
{ chat = m_chat; }

#endif

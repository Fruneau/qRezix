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

#ifndef Q_OS_WIN
#	define DefaultFont "Terminal"
#	define DefaultSize 11
#else
#	define DefaultFont "Arial"
#	define DefaultSize 8
#endif

class RzxChat;

/**
 @author Florent Bruneau
 */

///Pour customiser la boîte d'édition
/** Rajout de Stéphane Lescuyer 2004/05/27 */
class RzxTextEdit : public QTextEdit 
{
	Q_OBJECT
	Q_PROPERTY(QString font READ font WRITE setFont)
	Q_PROPERTY(int size READ size WRITE setSize)
	Q_PROPERTY(bool bold READ bold WRITE setBold)
	Q_PROPERTY(bool italic READ italic WRITE setItalic)
	Q_PROPERTY(bool underline READ underline WRITE setUnderline)
	Q_PROPERTY(QColor color READ color WRITE setColor)
	Q_PROPERTY(bool useHtml READ useHtml WRITE useHtml)
	
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

	QString m_font;
	QString m_defaultFont;
	QColor m_color;
	int m_size;
	int m_defaultSize;
	bool m_italic;
	bool m_bold;
	bool m_underline;
	bool m_html;
	

	QTextLine currentTextLine() const;
	bool atBeginning() const;
	bool atEnd() const;

	QString convertStyle(const QString&, const QString&, const QString& = QString()) const;

public:
	RzxTextEdit(QWidget *parent=0);
	~RzxTextEdit();

	const QString &font() const;
	int size() const;
	bool bold() const;
	bool italic() const;
	bool underline() const;
	const QColor &color() const;
	bool useHtml() const;

	QString toSimpleHtml() const;

public slots:
	void validate(bool = true);
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
	m_bold = bold;
	setFontWeight(m_html && bold ? QFont::Bold : QFont::Normal);
	setFocus();
}

///Retourne l'état de l'utilisation du gras
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline bool RzxTextEdit::bold() const
{
	return m_bold;
}

///Passe l'édition en italique
inline void RzxTextEdit::setItalic(bool ital)
{
	m_italic = ital;
	setFontItalic(m_html && ital);
	setFocus();
}

///Retourne l'état de l'utilisation de l'italique
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline bool RzxTextEdit::italic() const
{
	return m_italic;
}

///Passe l'édition en souligné
inline void RzxTextEdit::setUnderline(bool under)
{
	m_underline = under;
	setFontUnderline(under);
	setFocus();
}

///Retourne l'état de l'utilisation du soulignement
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline bool RzxTextEdit::underline() const
{
	return m_underline;
}

///Change la police
inline void RzxTextEdit::setFont(const QString& font)
{
	m_font = font;
	if(m_html)
		setFontFamily(font);
	setFocus();
}

///Retourne l'état de l'utilisation de la police
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline const QString &RzxTextEdit::font() const
{
	return m_font;
}

///Change la taille de la police
inline void RzxTextEdit::setSize(int size)
{
	m_size = size;
	if(m_html)
		setFontPointSize(size);
	setFocus();
}

///Retourne l'état de l'utilisation de la taille de la police
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline int RzxTextEdit::size() const
{
	return m_size;
}

///Change la couleur de la police
inline void RzxTextEdit::setColor(const QColor& c)
{
	m_color = c;
	if(m_html)
		setTextColor(c);
	setFocus();
}

///Retourne l'état de l'utilisation de la couleur de la police
/** L'information retournée décrit le formatage du texte
 * dans le cas ou l'édition est en mode html
 */
inline const QColor &RzxTextEdit::color() const
{
	return m_color;
}

///Retourne l'état de l'utilisation du formatage html
inline bool RzxTextEdit::useHtml() const
{
	return m_html;
}

#endif

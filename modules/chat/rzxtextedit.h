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

#ifdef Q_OS_WIN
#	define DefaultFont "Arial"
#	define DefaultSize 8
#endif

#ifdef Q_OS_MAC
#	define DefaultFont "Lucida Grande"
#	define DefaultSize 13
#endif

#ifndef DefaultFont
#       define DefaultFont "Terminal"
#       define DefaultSize 11
#endif


class RzxChat;

/**
 @author Florent Bruneau
 */

///Pour customiser la bo�te d'�dition
/** Rajout de St�phane Lescuyer 2004/05/27 */
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

	///Liste cha�n�e simple et rapide pour g�rer l'historique
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

	///Pour stocker les donn�es de formatage d'un texte
	struct Format
	{
		QString family;
		int size;
		QColor color;
		bool bold;
		bool italic;
		bool underlined;
	};

	ListText *history;
	ListText *curLine;

	Format defaultFormat;
	Format currentFormat;
	bool m_html;

	bool init;

	QTextLine currentTextLine() const;
	bool atBeginning() const;
	bool atEnd() const;

	QString convertStyle(const QString&, const QRegExp&, const QString& = QString()) const;
	friend Format operator-(const Format&, const Format&);
	friend Format operator%(const Format&, const Format&);

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
	void setFormat(const Format&);
	void useHtml(bool);
	void insertPlainText(const QString&);
	
protected:
	void setChat(RzxChat*);
	void keyPressEvent(QKeyEvent*);
	bool nickAutocompletion();

	void onArrowPressed(bool);
	void onTextEdited();

	static Format formatFromMarket(const QString&, const QString&);
	static Format formatFromStyle(const QString&);
	static Format formatFromFont(const QString&);
	static QString formatHtml(const QString&, const Format&);
	static QString formatStyle(const QString&, const Format&, const QString&);

signals:
	void enterPressed();
	void textWritten();
};

///D�fini la fen�tre d'attachement
inline void RzxTextEdit::setChat(RzxChat *m_chat)
{ chat = m_chat; }

///Passe l'�dition en gras
/** Cette fonction est impl�ment�e car elle a disparu avec le
 * passage en Qt4... ce qui est d'ailleurs dommage
 *
 * Le code est extrait directement du code de la partie Qt3Support de QTextEdit
 */
inline void RzxTextEdit::setBold(bool bold)
{
	currentFormat.bold = bold;
	setFontWeight(m_html && bold ? QFont::Bold : QFont::Normal);
	setFocus();
}

///Retourne l'�tat de l'utilisation du gras
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline bool RzxTextEdit::bold() const
{
	return currentFormat.bold;
}

///Passe l'�dition en italique
inline void RzxTextEdit::setItalic(bool ital)
{
	currentFormat.italic = ital;
	setFontItalic(m_html && ital);
	setFocus();
}

///Retourne l'�tat de l'utilisation de l'italique
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline bool RzxTextEdit::italic() const
{
	return currentFormat.italic;
}

///Passe l'�dition en soulign�
inline void RzxTextEdit::setUnderline(bool under)
{
	currentFormat.underlined = under;
	setFontUnderline(under);
	setFocus();
}

///Retourne l'�tat de l'utilisation du soulignement
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline bool RzxTextEdit::underline() const
{
	return currentFormat.underlined;
}

///Change la police
inline void RzxTextEdit::setFont(const QString& font)
{
	currentFormat.family = font;
	if(m_html)
		setFontFamily(font);
	setFocus();
}

///Retourne l'�tat de l'utilisation de la police
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline const QString &RzxTextEdit::font() const
{
	return currentFormat.family;
}

///Change la taille de la police
inline void RzxTextEdit::setSize(int size)
{
	if(size < 0) return;
	currentFormat.size = size;
	if(m_html && size)
		setFontPointSize(size);
	setFocus();
}

///Retourne l'�tat de l'utilisation de la taille de la police
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline int RzxTextEdit::size() const
{
	return currentFormat.size;
}

///Change la couleur de la police
inline void RzxTextEdit::setColor(const QColor& c)
{
	currentFormat.color = c;
	if(m_html && c.isValid())
		setTextColor(c);
	setFocus();
}

///Retourne l'�tat de l'utilisation de la couleur de la police
/** L'information retourn�e d�crit le formatage du texte
 * dans le cas ou l'�dition est en mode html
 */
inline const QColor &RzxTextEdit::color() const
{
	return currentFormat.color;
}

///Retourne l'�tat de l'utilisation du formatage html
inline bool RzxTextEdit::useHtml() const
{
	return m_html;
}

#endif

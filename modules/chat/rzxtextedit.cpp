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
#include <QTextLayout>

#include <RzxComputer>

#include "rzxtextedit.h"

#include "rzxchatconfig.h"
#include "rzxchat.h"

/***************************************************
* RzxTextEdit::ListText
***************************************************/
///Construit un nouveau maillon de la liste
RzxTextEdit::ListText::ListText(QString t, ListText * pN)
{
	texte = t;
	pNext = pN;
	pPrevious = 0;
	if(pNext != 0)
		pNext -> pPrevious = this;
}

///Supprime la liste r�cursivement
RzxTextEdit::ListText::~ListText()
{
	delete pNext;
}

/***************************************************
* Format
***************************************************/
///Diff�rence de deux formats...
/** On retourne le premier format avec :
 * 	- si le champs est diff�rent -> la valeur du premier
 * 	- si le champs est identique, rien
 */
RzxTextEdit::Format operator-(const RzxTextEdit::Format& f1, const RzxTextEdit::Format& f2)
{
	RzxTextEdit::Format newFormat = f1;
	if(f1.family == f2.family)
		newFormat.family = QString();
	if(f1.size == f2.size)
		newFormat.size = 0;
	if(f1.color == f2.color)
		newFormat.color = QColor();
	if(f1.bold == f2.bold)
		newFormat.bold = false;
	if(f1.italic == f2.italic)
		newFormat.italic = false;
	if(f1.underlined == f2.underlined)
		newFormat.underlined = false;
	return newFormat;
}


/***************************************************
* RzxTextEdit
***************************************************/
///Construction
RzxTextEdit::RzxTextEdit(QWidget *parent)
	:QTextEdit(parent), chat(NULL)
{
	curLine = history = NULL;
	m_html = false;
	defaultFormat.family = RzxChatConfig::nearestFont(DefaultFont);
	defaultFormat.size = RzxChatConfig::nearestSize(currentFormat.family, DefaultSize);
	defaultFormat.bold = defaultFormat.italic = defaultFormat.underlined = false;
	defaultFormat.color = Qt::black;
	currentFormat = defaultFormat;
	init = false;
	validate(false);
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
	QKeyEvent *eMapped = e;
	bool down=false;
	
	//Saut de ligne - Envoie du message
	switch(eMapped->key())
	{
		case Qt::Key_Enter: case Qt::Key_Return:
			if(eMapped->modifiers() != Qt::SHIFT)
			{
				emit enterPressed();
				break;
			}
			QTextEdit::keyPressEvent(eMapped);
			break;

		//Autocompletion
		case Qt::Key_Tab:
			//Pour que quand on appuie sur tab �a fasse la compl�tion du nick
			if(!nickAutocompletion())
			{
				QTextEdit::keyPressEvent(eMapped);
				onTextEdited();
			}
			break;

		//Parcours de l'historique
		case Qt::Key_Down:
			down=true;
		case Qt::Key_Up:
			//Et op, parcours de l'historique si les conditions sont r�unies
			if((eMapped->modifiers() & Qt::ShiftModifier) || (eMapped->modifiers() & Qt::ControlModifier) 
				|| (down && atEnd()) || (!down && atBeginning()))
			{
				onArrowPressed(down);
				break;
			}
			eMapped = new QKeyEvent(QEvent::KeyRelease, e->key(), e->modifiers(), e->text());

		//Texte normal
		default:
			QTextEdit::keyPressEvent(eMapped);
			onTextEdited();
	}
}

///Retourne un QTextLine indiquant la position du curseur
QTextLine RzxTextEdit::currentTextLine() const
{
	const QTextBlock block = textCursor().block();
	if (!block.isValid())
		return QTextLine();

	const QTextLayout *layout = block.layout();
	if (!layout)
		return QTextLine();

	const int relativePos = textCursor().position() - block.position();
	return layout->lineForTextPosition(relativePos);
}

///Indique si on est � la premi�re ligne
bool RzxTextEdit::atBeginning() const
{
	QTextBlock block = textCursor().block();
	if(block.previous().isValid()) return false;

	QTextLine line = currentTextLine();
	return line.isValid() && !line.lineNumber();
}

///Indique si on est � la derni�re ligne
bool RzxTextEdit::atEnd() const
{
	QTextBlock block = textCursor().block();
	if(block.next().isValid()) return false;

	QTextLine line = currentTextLine();
	return line.isValid() && line.lineNumber() == block.layout()->lineCount() - 1;
}

///Fait une completion automatique du nick au niveau du curseur.
/** La completion n'est r�alis�e que si aucune s�lection n'est actuellement d�finie */
bool RzxTextEdit::nickAutocompletion()
{
	QTextCursor cursor = textCursor();
	
	//Si y'a une s�lection, on zappe
	if(cursor.hasSelection())
		return false;
	
	//On r�cup�re la position du curseur et la paragraphe concern�
	int index = cursor.position();
	index -= cursor.block().position();
	if(!index) return false;
	
	QRegExp mask("[^-A-Za-z0-9]([-A-Za-z0-9]+)$");
	const QString textPara = cursor.block().text();
	
	//Juste pour se souvenir des pseudos possibles
	const QString localName = RzxComputer::localhost()->name();
	const QString remoteName = chat->computer()->name();
	const QString localLower = localName.toLower();
	const QString remoteLower = remoteName.toLower();
	
	for(int i = 1 ; i <= index && (localName.length() > i || remoteName.length() > i) ; i++)
	{
		//Chaine de caract�re qui pr�c�de le curseur de taille i
		QString nick = textPara.mid(index-i, i).toLower();
		
		if(mask.indexIn(nick) != -1 || i == index)
		{
			if(mask.indexIn(nick) != -1) nick = mask.cap(1);
			if(!remoteLower.indexOf(nick, false) && localLower.indexOf(nick, false))
			{
				for(int i = 0; i< nick.length();i++)
				{
					cursor.deletePreviousChar ();
				}
				cursor.insertText(remoteName + " ");
				return true;
			}
			else if(remoteLower.indexOf(nick, false) && !localLower.indexOf(nick, false))
			{
				for(int i = 0; i< nick.length();i++)
				{
					cursor.deletePreviousChar ();
				}
				cursor.insertText(localName + " ");
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
	if(!history)
		return;

	ListText *newCur = NULL;
	if(down)
		newCur = curLine->pPrevious;
	else
		newCur = curLine->pNext;

	if(!newCur)
		newCur = history;
	if(curLine != newCur)
	{
		setHtml(newCur->texte);
		curLine = newCur;
	}
}

///En cas d'�dition du texte, on met � jour l'historique
void RzxTextEdit::onTextEdited()
{
	//Supprime un <span></span> en trop qui appara�t lors de la premi�re �dition de l'objet....
	//J'aimerais bien savoir d'o� il sort, mais bon... ce hack crade r�soud le probl�me
	//C'est transparent pour l'utilisateur et l�ger (ex�cut� une seule fois)
	if(!init)
	{
		QString text = toHtml();
		QRegExp maskSpan("^(.*)<span style=\"([^\"]*)\">((?:[^<]|<(?!/span>))*)</span>(.*)$");
		QRegExp maskP("^(.*)<p style=\"([^\"]*)\">((?:[^<]|<(?!/p>))*)</p>(.*)$");
		if(maskSpan.indexIn(text) != -1)
		{
			QTextCursor cursor = textCursor(); //sauvegarde de la position du curseur
			setHtml(maskSpan.cap(1) + maskSpan.cap(3) + maskSpan.cap(4));
			setTextCursor(cursor); //restauration de la position du curseur
			init = true;
		}
		else if(maskP.indexIn(text) != -1)
		{
			QTextCursor cursor = textCursor(); //sauvegarde de la position du curseur
			const Format formatP = formatFromStyle(maskP.cap(2));
			const Format formatBody = formatFromMarket(text, "body");
			setHtml(maskP.cap(1) + formatStyle(maskP.cap(3), currentFormat - formatP - formatBody) + maskP.cap(4));
			setTextCursor(cursor); //restauration de la position du curseur
			init = true;
		}
	}

	//On informe que le texte est �dit�
	emit textWritten();

	//Ajout du nouveau texte � l'historique
	if(!history)
	{
		history = new ListText(toHtml(), NULL);
		curLine = history;
		return;
	}
	history->texte = toHtml();
	curLine = history;
}

void RzxTextEdit::setFormat(const Format& format)
{
	setFont(format.family);
	setSize(format.size);
	setBold(format.bold);
	setItalic(format.italic);
	setUnderline(format.underlined);
	setColor(format.color);
}

///Change le formatage du texte
void RzxTextEdit::useHtml(bool html)
{
	m_html = true;
	if(!html)
		setFormat(defaultFormat);
	else
		setFormat(currentFormat);
	m_html = html;
	init = false;
	setAcceptRichText(html);
	setPlainText(toPlainText());
	setFocus();
}

///Valide le contenue
/** La validation consiste � l'envoie du contenu de la fen�tre
 * donc on incr�mente l'historique et on vide la fen�tre.
 */
void RzxTextEdit::validate(bool val)
{
	if(val)
	{
		history = new ListText(toHtml(), history);
		curLine = history;
	}
	useHtml(m_html);
	clear();
}

///Sort le texte dans un format html simplifi�
/** Le HTML de Qt4 est relavitement complexe (formatage via style CSS)
 * il faut donc le simplifier pour obtenir qqch de compatible avec
 * les versions ant�rieures de qRezix
 */
QString RzxTextEdit::toSimpleHtml() const
{
	QString text = toHtml();
	//On supprime tous les en-t�tes
	QRegExp mask("<body[^>]*>(.*)</body>");
	if(mask.indexIn(text) != -1)
		text = mask.cap(1);

	//On vire les <span style=" font-family:Terminal [DEC]; font-size:22pt; color:#000000;"
	text = convertStyle(text, "span");
	text = convertStyle(text, "p", "<br>");
	if(text.right(4) == "<br>")
		text = text.left(text.length() - 4);
	qDebug() << text;
	return text;
}

///Converti du format CSS au format HTML
QString RzxTextEdit::convertStyle(const QString& m_text, const QString& balise, const QString& suffix) const
{
	QRegExp mask("<" + balise + " style=\"([^\"]*)\">((?:[^<]|<(?!/" + balise + ">))*)</" + balise + ">");
	int pos;
	QString text = m_text;
	while((pos = mask.indexIn(text)) != -1)
	{
		const QString style = mask.cap(1);

		Format format = formatFromStyle(style);
		QString rep = formatHtml(mask.cap(2), format);

		if(!suffix.isEmpty())
			rep = rep + suffix;
		text.replace(pos, mask.matchedLength(), rep);
	}
	return text;
}

///Extrait le format associ� � une balise
RzxTextEdit::Format RzxTextEdit::formatFromMarket(const QString& text, const QString& market) const
{
	const Format format = { QString(), 0, QColor(), false, false, false };
	QRegExp mask("<" + market + " style=\"([^\"]*)\">");
	if(mask.indexIn(text) != -1)
		return formatFromStyle(mask.cap(1));
	return format;
}

///G�n�re une structure 'Format' � partir des donn�es "styles"
RzxTextEdit::Format RzxTextEdit::formatFromStyle(const QString& style) const
{
	Format format = { QString(), 0, QColor(), false, false, false };
	QRegExp select;

	select.setPattern("font-family:([^;]+);");
	if(select.indexIn(style) != -1)
		format.family = select.cap(1);

	select.setPattern("font-size:(\\d+)pt;");
	if(select.indexIn(style) != -1)
		format.size = select.cap(1).toInt();

	select.setPattern("color:([^;]+);");
	if(select.indexIn(style) != -1)
		format.color = select.cap(1);

	if(style.contains("font-weight:600;"))
		format.bold = true;

	if(style.contains("font-style:italic;"))
		format.italic = true;

	if(style.contains("text-decoration: underline;"))
		format.underlined = true;

	return format;
}

///G�n�re une balise "Font" et des donn�es de formatage � partir du format indiqu�
QString RzxTextEdit::formatHtml(const QString& text, const Format& format) const
{
	QString rep; //param�tres de <font >
	QString tagDebut;
	QString tagFin;

	if(!format.family.isEmpty())
		rep += " face=\"" + format.family + "\"";

	if(format.size)
		rep += " size=\"" + QString::number(format.size) + "\"";

	if(format.color.isValid())
		rep += " color=\"" + format.color.name() + "\"";

	if(format.bold)
	{
		tagDebut += "<B>";
		tagFin = "</B>" + tagFin;
	}

	if(format.italic)
	{
		tagDebut += "<I>";
		tagFin = "</I>" + tagFin;
	}

	if(format.underlined)
	{
		tagDebut += "<U>";
		tagFin = "</U>" + tagFin;
	}

	if(!rep.isEmpty())
	{
		tagDebut += "<font" + rep + ">";
		tagFin = "</font>" + tagFin;
	}
	return tagDebut + text + tagFin;
}

///G�n�re une balise "Span" et des donn�es de formatage � partir du format indiqu�
QString RzxTextEdit::formatStyle(const QString& text, const Format& format) const
{
	QString rep; //param�tres de <span >

	if(!format.family.isEmpty())
		rep += "font-family:" + format.family + "; ";

	if(format.size)
		rep += "font-size:" + QString::number(format.size) + "; ";

	if(format.color.isValid())
		rep += "color:" + format.color.name() + "; ";

	if(format.bold)
		rep += "font-weight:600; ";

	if(format.italic)
		rep += "font-style:italic; ";

	if(format.underlined)
		rep += "text-decoration: underline; ";

	if(!rep.isEmpty())
		return "<span style=\"" + rep + "\">" + text + "</span>";
	return text;
}

///Surcharge de l'insertion de text pour corriger le probl�me de formatage initial
void RzxTextEdit::insertPlainText(const QString& text)
{
	QTextEdit::insertPlainText(text);
	onTextEdited();
}

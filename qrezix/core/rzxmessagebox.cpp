/***************************************************************************
                          rzxmessagebox.h  -  description
                             -------------------
    begin                : Fri Jan 24 2003
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QApplication>

#include <RzxMessageBox>

///Affiche une fen�tre d'information
/** L'utilisateur d�fini le titre et le contenu du message. Il a �galement 
 * le choix de la fen�tre m�re... NULL si l'objet est ind�pendant.
 *  Pour permettre le fonctionnement asynchr�ne du programme, il faut absolument 
 * �viter les fen�tre modales.
 */
int RzxMessageBox::information( QWidget *parent, const QString& caption,
	const QString& text, bool modal )
{
	if(QApplication::type() == QApplication::Tty)
		qDebug("[%s] %s", caption.toLatin1().constData(), text.toLatin1().constData());
	else
	{
		QMessageBox *mb = new QMessageBox(caption, text, QMessageBox::Information,
										  0, 0, 0, parent);
		mb->setAttribute(Qt::WA_DeleteOnClose);
		mb->setModal(modal);
		mb->show();
	}
	return 0;
}


///Affiche une fen�tre d'avertissement
/** L'utilisateur d�fini le titre et le contenu du message. Il a �galement 
* le choix de la fen�tre m�re... NULL si l'objet est ind�pendant.
*  Pour permettre le fonctionnement asynchr�ne du programme, il faut absolument 
* �viter les fen�tre modales.
*/
int RzxMessageBox::warning( QWidget *parent, const QString& caption,
	const QString& text, bool modal )
{
	if(QApplication::type() == QApplication::Tty)
		qDebug("[%s] %s", caption.toLatin1().constData(), text.toLatin1().constData());
	else
	{
		QMessageBox *mb = new QMessageBox( caption, text, QMessageBox::Warning,
										   0, 0, 0, parent);
		mb->setAttribute(Qt::WA_DeleteOnClose);
		mb->setModal(modal);
		mb->show();
	}
	return 0;
}


///Affiche une fen�tre d'erreur
/** L'utilisateur d�fini le titre et le contenu du message. Il a �galement 
* le choix de la fen�tre m�re... NULL si l'objet est ind�pendant.
*  Pour permettre le fonctionnement asynchr�ne du programme, il faut absolument 
* �viter les fen�tre modales.
*/
int RzxMessageBox::critical( QWidget *parent, const QString& caption,
	const QString& text, bool modal )
{
	if(QApplication::type() == QApplication::Tty)
		qDebug("[%s] %s", caption.toLatin1().constData(), text.toLatin1().constData());
	else
	{
		QMessageBox *mb = new QMessageBox( caption, text, QMessageBox::Critical,
										   0, 0, 0, parent);
		mb->setAttribute(Qt::WA_DeleteOnClose);
		mb->setModal(modal);
		mb->show();
	}
	return 0;
}


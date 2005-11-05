/***************************************************************************
                          rzxtranslator  -  description
                             -------------------
    begin                : Sat Nov 5 2005
    copyright            : (C) 2005 by Florent Bruneau
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
#include <QDir>
#include <QTranslator>
#include <QApplication>

#include <RzxTranslator>
#include <RzxConfig>

RZX_GLOBAL_INIT(RzxTranslator)

///Construction du gestionnaire de traduction
RzxTranslator::RzxTranslator()
{
	Rzx::beginModuleLoading("Translations");

	object = this;
	loadTranslators();	

	Rzx::endModuleLoading("Translations");
}

///Destruction du gestionnaire de traduction
RzxTranslator::~RzxTranslator()
{
	languageNames.clear();
	foreach(QList<QTranslator*> list, translations)
		qDeleteAll(list);
	translations.clear();
}

///Chargement des traductions disponibles
void RzxTranslator::loadTranslators()
{
	qDebug("Searching for translations...");
	languageNames.insert("en", "English");
	lang = "en";
	QList<QDir> dirs = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "translations", true);
	foreach(QDir dir, dirs)
		loadTranslatorsInDir(dir);

	qDebug("Loading translation...");
	setLanguage(language());
}

///Chargement des traductions contenues dans le répertoire indiqué
void RzxTranslator::loadTranslatorsInDir(const QDir &rep)
{
	QDir sourceDir(rep);

	QStringList trans=sourceDir.entryList(QStringList() << "qrezix_*.qm", QDir::Files|QDir::Readable);
	foreach(QString it, trans)
	{
		QRegExp mask("qrezix_(.+)\\.qm");
		mask.indexIn(it);
		QString langId = mask.cap(1);

		QTranslator *cur = new QTranslator;
		cur->load(it, sourceDir.path());
		QString lang = cur->translate("RzxConfig", "English");
		
		if(!lang.isEmpty() && !translations.keys().contains(langId))
		{
			languageNames.insert(langId, lang);
			QStringList transMods = sourceDir.entryList(QStringList() << "*_" + langId + ".qm", QDir::Files|QDir::Readable);
			QList<QTranslator*> transList;
			transList << cur;
			foreach(QString mod, transMods)
			{
				QTranslator *modTrans = new QTranslator;
				modTrans->load(mod, sourceDir.path());
				transList << modTrans;
			}
			translations.insert(langId, transList);
			qDebug() << "*" << lang << "(" << langId << ") in" << sourceDir.path();
		}
		else
			delete cur;
	}
}

///Retourne la liste des traductions disponibles
QStringList RzxTranslator::translationsList()
{
	QStringList list = global()->languageNames.values();
	qSort(list);
	return list;
}

///Retourne la traduction actuelle
/** Contrairement à language qui retourne la langue demandée par l'utilisateur
 * cette fonction retourne le nom de la langue actuellement chargée... ce qui
 * peut être différent dans certaines conditions.
 */
QString RzxTranslator::translation()
{
	return tr("English");
}

///Sélection de la langue à utiliser
void RzxTranslator::setLanguage(const QString& language)
{
	QString newLang = global()->languageNames.keys(language)[0];
	if(language != global()->translation() && global()->translations.keys().contains(newLang))
	{
		RzxConfig::global()->setValue("language", language);
		foreach(QTranslator *trans, global()->translations[global()->lang])
			QApplication::removeTranslator(trans);
		global()->lang = newLang;
		foreach(QTranslator *trans, global()->translations[global()->lang])
			QApplication::installTranslator(trans);
		emit global()->languageChanged(newLang);
	}
	qDebug() << "Language set to" << tr("English");
}

///Retourne le language actuel
QString RzxTranslator::language()
{
	return RzxConfig::global() -> value("language", "English").toString();
}

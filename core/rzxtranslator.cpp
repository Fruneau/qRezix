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
	foreach(const QList<QTranslator*> &list, translations)
		qDeleteAll(list);
	translations.clear();
	RZX_GLOBAL_CLOSE
}

///Chargement des traductions disponibles
void RzxTranslator::loadTranslators()
{
	qDebug("Searching for translations...");
	languageNames.insert("en", "English");
	lang = "en";
	QList<QDir> dirs = RzxConfig::dirList(RzxConfig::AllDirsExceptTemp, "translations", true);
	foreach(const QDir &dir, dirs)
		loadTranslatorsInDir(dir);

	qDebug("Loading translation...");
	setLanguage(language());
}

///Chargement des traductions contenues dans le répertoire indiqué
void RzxTranslator::loadTranslatorsInDir(const QDir &rep)
{
	const QDir sourceDir(rep);

	QStringList trans=sourceDir.entryList(QStringList() << "qrezix_*.qm", QDir::Files|QDir::Readable);
	foreach(const QString &it, trans)
	{
		const QString langId = it.mid(7, it.size() - 10);

		QTranslator *cur = new QTranslator;
		cur->load(it, sourceDir.path());
		const QString lang = cur->translate("RzxConfig", "English");
		
		if(!lang.isEmpty() && !translations.keys().contains(langId))
		{
			languageNames.insert(langId, lang);
			const QStringList transMods = sourceDir.entryList(QStringList() << "*_" + langId + ".qm",
                                                                          QDir::Files|QDir::Readable);
			QList<QTranslator*> transList;
			transList << cur;
			foreach(const QString &mod, transMods)
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

///Retourne la liste des id des langues disponibles (en, fr,...)
QStringList RzxTranslator::languageIdList()
{
	QStringList list = global()->languageNames.keys();
	qSort(list);
	return list;
}

///Retourne le nom de la langue identifiée par l'id indiqué
/** Retourne une chaîne nulle si la langue n'existe pas
 */
QString RzxTranslator::name(const QString& id)
{
	if(!global()->languageNames.keys().contains(id)) return QString();
	return global()->languageNames[id];
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
	QString newLang = global()->languageNames.key(language);
	if(language != global()->translation() && global()->translations.keys().contains(newLang))
	{
		RzxConfig::setLanguage(language);
		foreach(QTranslator *trans, global()->translations[global()->lang])
			QApplication::removeTranslator(trans);
		global()->lang = newLang;
		foreach(QTranslator *trans, global()->translations[global()->lang])
			QApplication::installTranslator(trans);
		emit global()->languageChanged(newLang);
	}
	else if (language != global()->translation())
	{
		if (language.size() > 2 && language[0].isLower() && language[1].isLower()) {
			if (language.size() > 2 && language[2] != '_') {
				return;
			}
			const QString langid = language.left(2);
			RzxTranslator::setLanguage(global()->languageNames[langid]);
			return;
		}
	}
}

///Retourne le language actuel
QString RzxTranslator::language()
{
        qDebug() << "langage is" << RzxConfig::language();
	return RzxConfig::language();
}

///Indique si la chaîne indiquée est la traduction dans une des langues disponibles de la deuxième chaîne
/** Utile pour pouvoir faire du reverse-translation...
 */
bool RzxTranslator::backTranslate(const QString& string, const char *context, const char *orig)
{
	if(string.isEmpty() || !context || !orig)
		return false;

	foreach(const QList<QTranslator*> &list, global()->translations)
	{
		foreach(const QTranslator *trans, list)
			if(string == trans->translate(context, orig))
				return true;
	}
	return false;
}

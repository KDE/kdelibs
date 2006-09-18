#include "ktranslatable.h"

#include <kglobal.h>
#include <klocale.h>

//#include <qstringlist.h>

KTranslatable::KTranslatable()
{
}

KTranslatable::KTranslatable(QString string)
{
	m_strings[QString()] = string;
}

void KTranslatable::addString(QString lang, QString string)
{
	m_strings[lang] = string;
}

QString KTranslatable::representation()
{
	if(m_strings.isEmpty()) return QString::null;

	//if(map.contains(lang)) return map[lang];
	//else
	//{
	QStringList langs = KGlobal::locale()->languageList();
	for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
		if(m_strings.contains(*it)) return m_strings[*it];
	//}

	if(m_strings.contains(QString::null)) return m_strings[QString::null];
	else return *(m_strings.begin());
}

QString KTranslatable::translated(QString lang)
{
	if(m_strings.contains(lang))
		return m_strings[lang];
	return QString::null;
}

QStringList KTranslatable::languages()
{
	QStringList strlist;

	/*QMapIterator<QString, QString> it(m_strings);
	while(it.hasNext())
	{
		it.next();
		strlist << it.key();
	}*/
	QMap<QString, QString>::Iterator it;
	for(it = m_strings.begin(); it != m_strings.end(); it++)
	{
		strlist << it.key();
	}

	return strlist;
}

QStringList KTranslatable::strings()
{
	QStringList strlist;

	/*QMapIterator<QString, QString> it(m_strings);
	while(it.hasNext())
	{
		it.next();
		strlist << it.value();
	}*/
	QMap<QString, QString>::Iterator it;
	for(it = m_strings.begin(); it != m_strings.end(); it++)
	{
		strlist << it.data();
	}

	return strlist;
}

QMap<QString, QString> KTranslatable::stringmap()
{
	return m_strings;
}

bool KTranslatable::isTranslated()
{
	if(m_strings.count() > 1) return true;
	return false;
}

bool KTranslatable::isEmpty()
{
	if(m_strings.count() > 0) return false;
	return true;
}


/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "ktranslatable.h"

#include <kglobal.h>
#include <klocale.h>

#include <QtCore/QMutableStringListIterator>

using namespace KNS;

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
	if(m_strings.isEmpty()) return QString();

	QStringList langs = KGlobal::locale()->languageList();
	for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
		if(m_strings.contains(*it)) return m_strings[*it];

	if(m_strings.contains(QString())) return m_strings[QString()];
	else return *(m_strings.begin());
}

QString KTranslatable::language()
{
	if(m_strings.isEmpty()) return QString();

	QStringList langs = KGlobal::locale()->languageList();
	for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
		if(m_strings.contains(*it)) return (*it);

	if(m_strings.contains(QString())) return QString();
	else return m_strings.begin().key();
}

QString KTranslatable::translated(QString lang)
{
	if(m_strings.contains(lang))
		return m_strings[lang];
	return QString();
}

QStringList KTranslatable::languages()
{
	QStringList strlist;

	QMapIterator<QString, QString> it(m_strings);
	while(it.hasNext())
	{
		it.next();
		strlist << it.key();
	}

	return strlist;
}

QStringList KTranslatable::strings()
{
	QStringList strlist;

	QMapIterator<QString, QString> it(m_strings);
	while(it.hasNext())
	{
		it.next();
		strlist << it.value();
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


/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ktranslatable.h"

#include <kglobal.h>
#include <klocale.h>

#include <QtCore/QMutableStringListIterator>

using namespace KNS;

KTranslatable::KTranslatable()
        : d(0)
{
}

KTranslatable::KTranslatable(const KTranslatable& other)
        : d(0)
{
    m_strings = other.m_strings;
}

KTranslatable& KTranslatable::operator=(const KTranslatable & other)
{
    if (this == &other) {
        return *this;
    }
    m_strings = other.m_strings;
    return *this;
}

KTranslatable::~KTranslatable()
{
//    delete d;
}

KTranslatable::KTranslatable(const QString& string)
        : d(0)
{
    m_strings[QString()] = string;
}

void KTranslatable::addString(const QString& lang, const QString& string)
{
    m_strings[lang] = string;
}

QString KTranslatable::representation() const
{
    if (m_strings.isEmpty()) return QString();

    const QStringList langs = KGlobal::locale()->languageList();
    for (QStringList::ConstIterator it = langs.begin(); it != langs.end(); ++it)
        if (m_strings.contains(*it)) return m_strings[*it];

    if (m_strings.contains(QString())) return m_strings[QString()];
    // NOTE: this could be the source of crashes I've seen occasionally
    else return *(m_strings.begin());
}

QString KTranslatable::language() const
{
    if (m_strings.isEmpty()) return QString();

    const QStringList langs = KGlobal::locale()->languageList();
    for (QStringList::ConstIterator it = langs.begin(); it != langs.end(); ++it)
        if (m_strings.contains(*it)) return (*it);

    if (m_strings.contains(QString())) return QString();
    else return m_strings.begin().key();
}

QString KTranslatable::translated(const QString& lang) const
{
    if (m_strings.contains(lang))
        return m_strings[lang];
    return QString();
}

QStringList KTranslatable::languages() const
{
    return m_strings.keys();
}

QStringList KTranslatable::strings() const
{
    return m_strings.values();
}

QMap<QString, QString> KTranslatable::stringmap() const
{
    return m_strings;
}

bool KTranslatable::isTranslated() const
{
    return m_strings.count() > 1;
}

bool KTranslatable::isEmpty() const
{
    return m_strings.isEmpty();
}


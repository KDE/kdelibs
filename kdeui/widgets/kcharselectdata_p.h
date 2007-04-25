/* This file is part of the KDE libraries

   Copyright (C) 2007 Daniel Laidig <d.laidig@gmx.de>

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

#ifndef KCHARSELECTDATA_H
#define KCHARSELECTDATA_H

#include <QtCore/QChar>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

// Internal class used by KCharSelect

class KCharSelectData
{
public:
    enum SearchRange {Default, CJK};

    static QString formatCode(ushort code, int length = 4, const QString& prefix = "U+", int base = 16);

    static QList<QChar> blockContents(int block);
    static QList<int> sectionContents(int section);

    static QStringList sectionList();

    static QString block(const QChar& c);
    static QString name(const QChar& c);

    static int blockIndex(const QChar& c);
    static int sectionIndex(int block);

    static QString blockName(int index);

    static QStringList aliases(const QChar& c);
    static QStringList notes(const QChar& c);
    static QList<QChar> seeAlso(const QChar& c);
    static QStringList equivalents(const QChar& c);
    static QStringList approximateEquivalents(const QChar& c);

    static QStringList unihanInfo(const QChar& c);

    static QString categoryText(QChar::Category category);

    static QList<QChar> find(QString s, SearchRange range = Default);
};

#endif  /* #ifndef KCHARSELECTDATA_H */

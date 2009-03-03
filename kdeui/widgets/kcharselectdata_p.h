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
#include <QtCore/QFuture>
#include <QtGui/QFont>

// Internal class used by KCharSelect

typedef QMap<QString, QVector<quint16> > Index;

class KCharSelectData
{
public:
    QString formatCode(ushort code, int length = 4, const QString& prefix = "U+", int base = 16);

    QList<QChar> blockContents(int block);
    QList<int> sectionContents(int section);

    QStringList sectionList();

    QString block(const QChar& c);
    QString section(const QChar& c);
    QString name(const QChar& c);

    int blockIndex(const QChar& c);
    int sectionIndex(int block);

    QString blockName(int index);
    QString sectionName(int index);

    QStringList aliases(const QChar& c);
    QStringList notes(const QChar& c);
    QList<QChar> seeAlso(const QChar& c);
    QStringList equivalents(const QChar& c);
    QStringList approximateEquivalents(const QChar& c);

    QStringList unihanInfo(const QChar& c);

    bool isDisplayable(const QChar& c);
    bool isIgnorable(const QChar& c);
    bool isCombining(const QChar& c);
    QString display(const QChar& c, const QFont& font);
    QString displayCombining(const QChar& c);

    QString categoryText(QChar::Category category);

    QList<QChar> find(const QString& s);

private:
    bool openDataFile();
    quint32 getDetailIndex(const QChar& c) const;
    QSet<quint16> getMatchingChars(const QString& s);

    QStringList splitString(const QString& s);
    void appendToIndex(Index *index, quint16 unicode, const QString& s);
    Index createIndex(const QByteArray& dataFile);

    QByteArray dataFile;
    QFuture<Index> futureIndex;
};

#endif  /* #ifndef KCHARSELECTDATA_H */

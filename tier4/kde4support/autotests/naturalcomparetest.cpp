/* This file is part of the KDE libraries
 * Copyright (C) 1999 Ian Zepp (icszepp@islc.net)
 * Copyright (C) 2006 by Dominic Battre <dominic@battre.de>
 * Copyright (C) 2006 by Martin Pool <mbp@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kdecore/kstringhandler_deprecated.h"
#include <QTest>

class NaturalCompareTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void naturalCompare();
};

QTEST_MAIN( NaturalCompareTest )

void NaturalCompareTest::naturalCompare()
{
    QCOMPARE(KStringHandler::naturalCompare("a", "b", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("b", "a", Qt::CaseSensitive), +1);
    QCOMPARE(KStringHandler::naturalCompare("a", "a", Qt::CaseSensitive), 0);

    QCOMPARE(KStringHandler::naturalCompare("A", "a", Qt::CaseSensitive), QString::localeAwareCompare("A", "a"));
    QCOMPARE(KStringHandler::naturalCompare("A", "a", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("a", "A", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("aAa", "AaA", Qt::CaseInsensitive), 0);
    QCOMPARE(KStringHandler::naturalCompare("aaa", "AAA", Qt::CaseInsensitive), 0);

    QCOMPARE(KStringHandler::naturalCompare("1", "2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("9", "10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "12", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1", "100", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("1a", "2a", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("1b", "1a", Qt::CaseSensitive), +1);

    QCOMPARE(KStringHandler::naturalCompare("a1", "a2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a9", "a10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a12", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1", "a100", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a2a1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a1a2", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a10a1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a1a1", "a1a10", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("Test 1.gif", "Test 2.gif", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Test 1.gif", "Test 10.gif", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Test 9.gif", "Test 10.gif", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_2.4.10", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.10", "cmake_2.4.11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_2.5.6", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("cmake_2.4.6", "cmake_3.4.6", Qt::CaseSensitive), -1);

    QCOMPARE(KStringHandler::naturalCompare("A-123.txt", "A-a.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A-012.txt", "A-a.txt", Qt::CaseSensitive), -1);

    // bug 191865
    QCOMPARE(KStringHandler::naturalCompare("E & G", "E & J", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("E & J", "E & S", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("E & S", "Em & M", Qt::CaseSensitive), -1);

    // bug 181211
    QCOMPARE(KStringHandler::naturalCompare("queen__radio_ga_ga.mp3", "queen__somebody_to_love_live.mp3", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("queen__somebody_to_love_live.mp3", "queens_of_the_stone_age__no_one_knows.mp3", Qt::CaseSensitive), -1);

    // bug 201101
    QCOMPARE(KStringHandler::naturalCompare("text", "text.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("text.txt", "text1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("text1", "text1.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A B", "A.B", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("A.B", "A B", Qt::CaseSensitive), +1);

    // bug 231445
    QCOMPARE(KStringHandler::naturalCompare("1", "a", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("a", "v01 1", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("v01 1", "v01 a", Qt::CaseSensitive), -1);

    // bug 237551
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txt", "sysvinit-2.86-i486-6.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz", "sysvinit-2.86-i486-6.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz.asc", "sysvinit-functions-8.53-i486-2.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txt", "sysvinit-functions-8.53-i486-2.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txz", "sysvinit-functions-8.53-i486-2.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-functions-8.53-i486-2.txz.asc", "sysvinit-scripts-1.2-noarch-31.txt", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-scripts-1.2-noarch-31.txt", "sysvinit-scripts-1.2-noarch-31.txz", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-scripts-1.2-noarch-31.txz", "sysvinit-scripts-1.2-noarch-31.txz.asc", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("sysvinit-2.86-i486-6.txz.asc", "sysvinit-scripts-1.2-noarch-31.txz", Qt::CaseSensitive), -1);

    // bug 237541
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2008-10","Car and Driver Buyer's Guide 2010", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2008-10", "Car and Driver 2009-11", Qt::CaseSensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("Car and Driver 2009-11", "Car and Driver 2010-05", Qt::CaseSensitive), -1);

    // bug 237788
    QCOMPARE(KStringHandler::naturalCompare(" ", ".", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare(".", "_", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare(" ", "_", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc.jpg", "abc1.jpg", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc1.jpg", "abc_a.jpg", Qt::CaseInsensitive), -1);
    QCOMPARE(KStringHandler::naturalCompare("abc.jpg", "abc_a.jpg", Qt::CaseInsensitive), -1);
}

#include "naturalcomparetest.moc"

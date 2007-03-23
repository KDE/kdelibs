/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "krununittest.h"
#include "krununittest.moc"
#include <qtest_kde.h>
QTEST_KDEMAIN( KRunUnitTest, NoGUI )

#include "krun.h"

void KRunUnitTest::testBinaryName_data()
{
    QTest::addColumn<QString>("execLine");
    QTest::addColumn<bool>("removePath");
    QTest::addColumn<QString>("expected");

    QTest::newRow("/usr/bin/ls true") << "/usr/bin/ls" << true << "ls";
    QTest::newRow("/usr/bin/ls false") << "/usr/bin/ls" << false << "/usr/bin/ls";
    QTest::newRow("/path/to/wine \"long argument with path\"") << "/path/to/wine \"long argument with path\"" << true << "wine";
    QTest::newRow("/path/with/a/sp\\ ace/exe arg1 arg2") << "/path/with/a/sp\\ ace/exe arg1 arg2" << true << "exe";
    QTest::newRow("\"progname\" \"arg1\"") << "\"progname\" \"arg1\"" << true << "progname";
    QTest::newRow("'quoted' \"arg1\"") << "'quoted' \"arg1\"" << true << "quoted";
    QTest::newRow(" 'leading space'   arg1") << " 'leading space'   arg1" << true << "leading space";
}

void KRunUnitTest::testBinaryName()
{
    QFETCH(QString, execLine);
    QFETCH(bool, removePath);
    QFETCH(QString, expected);
    QCOMPARE(KRun::binaryName(execLine, removePath), expected);
}

void KRunUnitTest::testProcessDesktopExec()
{

}


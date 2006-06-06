/* This file is part of the KDE libraries
    Copyright (c) 2006 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qtest_kde.h"
#include "kmainwindow_unittest.h"
#include "kmainwindow_unittest.moc"
#include <kmainwindow.h>

QTEST_KDEMAIN( KMainWindow_UnitTest, GUI )

void KMainWindow_UnitTest::testDefaultName()
{
    KMainWindow mw;
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "qttest/MainWindow_1" ) );
    KMainWindow mw2;
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "qttest/MainWindow_2" ) );
}

void KMainWindow_UnitTest::testFixedName()
{
    KMainWindow mw;
    mw.setObjectName( "mymainwindow" );
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "qttest/mymainwindow" ) );
    KMainWindow mw2;
    mw2.setObjectName( "mymainwindow" );
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "qttest/mymainwindow2" ) );
}

void KMainWindow_UnitTest::testNameWithHash()
{
    KMainWindow mw;
    mw.setObjectName( "composer_#" );
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "qttest/composer_1" ) );
    KMainWindow mw2;
    mw2.setObjectName( "composer_#" );
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "qttest/composer_2" ) );
}

void KMainWindow_UnitTest::testNameWithSpecialChars()
{
    KMainWindow mw;
    mw.setObjectName( "a#@_test/" );
    mw.ensurePolished();
    QCOMPARE( mw.objectName(), QString::fromLatin1( "qttest/a___test_" ) );
    KMainWindow mw2;
    mw2.setObjectName( "a#@_test/" );
    mw2.ensurePolished();
    QCOMPARE( mw2.objectName(), QString::fromLatin1( "qttest/a___test_2" ) );
}

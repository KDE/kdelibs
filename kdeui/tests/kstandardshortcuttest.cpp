/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

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

#include <qtest_kde.h>
#include "kstandardshortcuttest.h"
#include "kstandardshortcuttest.moc"

QTEST_KDEMAIN( KStandardShortcutTest, GUI ) // GUI needed by KAccel

#include <kdebug.h>
#include <kstandardshortcut.h>

void KStandardShortcutTest::testShortcutDefault()
{
    QCOMPARE( KStandardShortcut::hardcodedDefaultShortcut( KStandardShortcut::FullScreen ).toString(), QLatin1String( "Ctrl+Shift+F" ) );
    QCOMPARE( KStandardShortcut::hardcodedDefaultShortcut( KStandardShortcut::BeginningOfLine ).toString(), QLatin1String( "Home" ) );
    QCOMPARE( KStandardShortcut::hardcodedDefaultShortcut( KStandardShortcut::EndOfLine ).toString(), QLatin1String( "End" ) );
    QCOMPARE( KStandardShortcut::hardcodedDefaultShortcut( KStandardShortcut::Home ).toString(), QLatin1String( "Alt+Home; Home Page" ) );
}

void KStandardShortcutTest::testName()
{
    QCOMPARE( KStandardShortcut::name( KStandardShortcut::BeginningOfLine ), QLatin1String( "BeginningOfLine" ) );
    QCOMPARE( KStandardShortcut::name( KStandardShortcut::EndOfLine ), QLatin1String( "EndOfLine" ) );
    QCOMPARE( KStandardShortcut::name( KStandardShortcut::Home ), QLatin1String( "Home" ) );
}

void KStandardShortcutTest::testLabel()
{
    // Tests run in English, right?
    QCOMPARE( KStandardShortcut::label( KStandardShortcut::FindNext ), QLatin1String( "Find Next" ) );
    QCOMPARE( KStandardShortcut::label( KStandardShortcut::Home ), QLatin1String( "Home" ) );
}

void KStandardShortcutTest::testShortcut()
{
    QCOMPARE( KStandardShortcut::shortcut( KStandardShortcut::ZoomIn ).toString(), KStandardShortcut::zoomIn().toString() );
}

void KStandardShortcutTest::testFindStdAccel()
{
    QCOMPARE( KStandardShortcut::find( QString( "Ctrl+F" ) ), KStandardShortcut::Find );
    QCOMPARE( KStandardShortcut::find( QString( "Ctrl+Shift+Alt+G" ) ), KStandardShortcut::AccelNone );
}

void KStandardShortcutTest::testRemoveShortcut()
{
    KShortcut cutShortCut = KStandardShortcut::shortcut( KStandardShortcut::Cut );
    cutShortCut.remove( Qt::SHIFT + Qt::Key_Delete, KShortcut::KeepEmpty );
    cutShortCut.remove( Qt::CTRL + Qt::Key_X, KShortcut::KeepEmpty );
    //qDebug( "%s", qPrintable( cutShortCut.toString() ) );
    QVERIFY( cutShortCut.isEmpty() );

    cutShortCut = KStandardShortcut::shortcut( KStandardShortcut::Cut );
    //remove primary shortcut. We expect the alternate to become primary.
    cutShortCut.remove( Qt::CTRL + Qt::Key_X, KShortcut::RemoveEmpty );
    QVERIFY( cutShortCut.primary() == QKeySequence(Qt::SHIFT + Qt::Key_Delete) );
    QVERIFY( cutShortCut.alternate().isEmpty() );
}

void KStandardShortcutTest::testKShortcut()
{
    KShortcut null;
    QVERIFY( null.isEmpty() );

    KShortcut zero( 0 );
    QVERIFY( zero.isEmpty() );
    QVERIFY( zero.primary().isEmpty() );
    QVERIFY( zero.alternate().isEmpty() );

    KShortcut quit( "Ctrl+X, Ctrl+C; Z, Z" ); // quit in emacs vs. quit in vi :)
    QCOMPARE( quit.primary().toString(), QString::fromLatin1("Ctrl+X, Ctrl+C") );
    QCOMPARE( quit.alternate().toString(), QString::fromLatin1("Z, Z") );
    QCOMPARE( quit.primary(), QKeySequence(Qt::CTRL + Qt::Key_X, Qt::CTRL + Qt::Key_C) );
    QVERIFY( quit != null );
    QVERIFY( !( quit == null ) );

    QVERIFY( !quit.contains( Qt::CTRL+Qt::Key_X ) );
    QVERIFY( !quit.contains( Qt::CTRL+Qt::Key_Z ) );
    QVERIFY( !quit.contains( Qt::CTRL+Qt::Key_C ) );
    QKeySequence seq( Qt::CTRL+Qt::Key_X, Qt::CTRL+Qt::Key_C );
    QVERIFY( quit.contains( seq ) );
    QVERIFY( !null.contains( seq ) );

    quit.setAlternate( seq );
    QCOMPARE( quit.primary().toString(), quit.alternate().toString() );
}

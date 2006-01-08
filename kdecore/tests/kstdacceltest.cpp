/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

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

#include <qtest_kde.h>
#include "kstdacceltest.h"
#include "kstdacceltest.moc"

QTEST_KDEMAIN( KStdAccelTest, GUI ) // GUI needed by KAccel

#include <kdebug.h>
#include <kstdaccel.h>

void KStdAccelTest::testShortcutDefault()
{
    QCOMPARE( KStdAccel::shortcutDefault( KStdAccel::FullScreen ).toString(), QLatin1String( "Ctrl+Shift+F" ) );
    QCOMPARE( KStdAccel::shortcutDefault( KStdAccel::BeginningOfLine ).toString(), QLatin1String( "Home" ) );
    QCOMPARE( KStdAccel::shortcutDefault( KStdAccel::EndOfLine ).toString(), QLatin1String( "End" ) );
}

void KStdAccelTest::testName()
{
    QCOMPARE( KStdAccel::name( KStdAccel::BeginningOfLine ), QLatin1String( "BeginningOfLine" ) );
    QCOMPARE( KStdAccel::name( KStdAccel::EndOfLine ), QLatin1String( "EndOfLine" ) );
}

void KStdAccelTest::testLabel()
{
    // Tests run in English, right?
    QCOMPARE( KStdAccel::label( KStdAccel::FindNext ), QLatin1String( "Find Next" ) );
}

void KStdAccelTest::testShortcut()
{
    QCOMPARE( KStdAccel::shortcut( KStdAccel::ZoomIn ).toString(), KStdAccel::zoomIn().toString() );
}

void KStdAccelTest::testFindStdAccel()
{
    QCOMPARE( KStdAccel::findStdAccel( QString( "Ctrl+F" ) ), KStdAccel::Find );
    QCOMPARE( KStdAccel::findStdAccel( QString( "Ctrl+Shift+Alt+G" ) ), KStdAccel::AccelNone );
}

void KStdAccelTest::testRemoveShortcut()
{
    KShortcut cutShortCut = KStdAccel::shortcut( KStdAccel::Cut );
    cutShortCut.remove( KKey( Qt::SHIFT + Qt::Key_Delete ) );
    cutShortCut.remove( KKey( Qt::CTRL + Qt::Key_X ) );
    //qDebug( "%s", qPrintable( cutShortCut.toString() ) );
    QVERIFY( cutShortCut.isNull() );
}

void KStdAccelTest::testKShortcut()
{
    KShortcut null;
    QVERIFY( null.isNull() );
    QVERIFY( null.count() == 0 );

    KShortcut quit( "CTRL+X,CTRL+C;Z,Z" ); // quit in emacs vs. quit in vi :)
    QCOMPARE( (int)quit.count(), 2 );
    QVERIFY( !quit.isNull() );
    QCOMPARE( quit.seq(0).toString(), QString::fromLatin1("Ctrl+X, Ctrl+C") );
    QCOMPARE( quit.seq(1).toString(), QString::fromLatin1("Z, Z") );
    QCOMPARE( quit.keyCodeQt(), (int)(Qt::CTRL + Qt::Key_X) );
    QVERIFY( quit.compare( null ) > 0 );
    QVERIFY( null < quit );
    QVERIFY( quit != null );
    QVERIFY( !( quit == null ) );

    QVERIFY( quit.contains( KKey( Qt::CTRL+Qt::Key_X ) ) );
    QVERIFY( !quit.contains( KKey( Qt::CTRL+Qt::Key_Z ) ) );
    QVERIFY( !quit.contains( KKey( Qt::CTRL+Qt::Key_C ) ) );
    KKeySequence seq;
    seq.setKey( 0, Qt::CTRL+Qt::Key_X );
    seq.setKey( 1, Qt::CTRL+Qt::Key_C );
    QVERIFY( quit.contains( seq ) );
    QVERIFY( !null.contains( seq ) );

    quit.setSeq( 1, seq );
    QCOMPARE( quit.seq(0).toString(), quit.seq(1).toString() );

    QKeySequence casted = quit; // is this a good idea?
    QCOMPARE( QString(casted), seq.toString() );
    QVERIFY( casted == seq.qt() );

    QVERIFY( KShortcut::null() == null );
}

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

#include <QtTest/qttest_kde.h>
#include "kstdacceltest.h"
#include "kstdacceltest.moc"

QTTEST_KDEMAIN( KStdAccelTest, GUI ) // GUI needed by KAccel

#include <kdebug.h>
#include <kstdaccel.h>

void KStdAccelTest::testShortcutDefault()
{
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::FullScreen ).toString(), QLatin1String( "Ctrl+Shift+F" ) );
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::BeginningOfLine ).toString(), QLatin1String( "Home" ) );
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::EndOfLine ).toString(), QLatin1String( "End" ) );
}

void KStdAccelTest::testName()
{
    COMPARE( KStdAccel::name( KStdAccel::BeginningOfLine ), QLatin1String( "BeginningOfLine" ) );
    COMPARE( KStdAccel::name( KStdAccel::EndOfLine ), QLatin1String( "EndOfLine" ) );
}

void KStdAccelTest::testLabel()
{
    // Tests run in English, right?
    COMPARE( KStdAccel::label( KStdAccel::FindNext ), QLatin1String( "Find Next" ) );
}

void KStdAccelTest::testShortcut()
{
    COMPARE( KStdAccel::shortcut( KStdAccel::ZoomIn ).toString(), KStdAccel::zoomIn().toString() );
}

void KStdAccelTest::testFindStdAccel()
{
    COMPARE( KStdAccel::findStdAccel( QString( "Ctrl+F" ) ), KStdAccel::Find );
    COMPARE( KStdAccel::findStdAccel( QString( "Ctrl+Shift+Alt+G" ) ), KStdAccel::AccelNone );
}

void KStdAccelTest::testRemoveShortcut()
{
    KShortcut cutShortCut = KStdAccel::shortcut( KStdAccel::Cut );
    cutShortCut.remove( KKey( Qt::SHIFT + Qt::Key_Delete ) );
    cutShortCut.remove( KKey( Qt::CTRL + Qt::Key_X ) );
    //qDebug( "%s", qPrintable( cutShortCut.toString() ) );
    VERIFY( cutShortCut.isNull() );
}

void KStdAccelTest::testKShortcut()
{
    KShortcut null;
    VERIFY( null.isNull() );
    VERIFY( null.count() == 0 );

    KShortcut quit( "CTRL+X,CTRL+C;Z,Z" ); // quit in emacs vs. quit in vi :)
    COMPARE( (int)quit.count(), 2 );
    VERIFY( !quit.isNull() );
    COMPARE( quit.seq(0).toString(), QString::fromLatin1("Ctrl+X, Ctrl+C") );
    COMPARE( quit.seq(1).toString(), QString::fromLatin1("Z, Z") );
    COMPARE( quit.keyCodeQt(), (int)(Qt::CTRL + Qt::Key_X) );
    VERIFY( quit.compare( null ) > 0 );
    VERIFY( null < quit );
    VERIFY( quit != null );
    VERIFY( !( quit == null ) );

    VERIFY( quit.contains( KKey( Qt::CTRL+Qt::Key_X ) ) );
    VERIFY( !quit.contains( KKey( Qt::CTRL+Qt::Key_Z ) ) );
    VERIFY( !quit.contains( KKey( Qt::CTRL+Qt::Key_C ) ) );
    KKeySequence seq;
    seq.setKey( 0, Qt::CTRL+Qt::Key_X );
    seq.setKey( 1, Qt::CTRL+Qt::Key_C );
    VERIFY( quit.contains( seq ) );
    VERIFY( !null.contains( seq ) );

    quit.setSeq( 1, seq );
    COMPARE( quit.seq(0).toString(), quit.seq(1).toString() );

    QKeySequence casted = quit; // is this a good idea?
    COMPARE( QString(casted), seq.toString() );
    VERIFY( casted == seq.qt() );

    VERIFY( KShortcut::null() == null );
}

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
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
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::FullScreen ).toString(), QString::fromLatin1( "Ctrl+Shift+F" ) );
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::BeginningOfLine ).toString(), QString::fromLatin1( "Home" ) );
    COMPARE( KStdAccel::shortcutDefault( KStdAccel::EndOfLine ).toString(), QString::fromLatin1( "End" ) );
}

void KStdAccelTest::testName()
{
    COMPARE( KStdAccel::name( KStdAccel::BeginningOfLine ), QString::fromLatin1( "BeginningOfLine" ) );
    COMPARE( KStdAccel::name( KStdAccel::EndOfLine ), QString::fromLatin1( "EndOfLine" ) );
}

void KStdAccelTest::testLabel()
{
    // Tests run in English, right?
    COMPARE( KStdAccel::label( KStdAccel::FindNext ), QString::fromLatin1( "Find Next" ) );
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

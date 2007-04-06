/* This file is part of the KDE libraries
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#ifndef QTEST_KDE_H
#define QTEST_KDE_H

#include <QtTest/QtTest>
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <QtGui/QApplication>

// By default, unit tests get no gui.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1 }; // bitfield, next item is 2!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDEMainFlags)
/**
 * \short KDE Replacement for QTEST_MAIN from QTestLib
 *
 * This macro should be used for classes that need a KComponentData.
 * So instead of writing QTEST_MAIN( TestClass ) you write
 * QTEST_KDEMAIN( TestClass, GUI ).
 *
 * \param TestObject The class you use for testing.
 * \param flags one of KDEMainFlag. This is passed to the QApplication constructor.
 *
 * \see KDEMainFlag
 * \see QTestLib
 */
#define QTEST_KDEMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test" ), 1); \
    KAboutData aboutData( "qttest", "qttest", "version" );  \
    KDEMainFlags mainFlags = flags;                         \
    KComponentData cData(&aboutData); \
    QApplication app( argc, argv, (mainFlags & GUI) != 0 ); \
    app.setApplicationName( "qttest" ); \
    TestObject tc; \
    return QTest::qExec( &tc, argc, argv ); \
}

/**
 * \short KDE Replacement for QTEST_MAIN from QTestLib, for non-gui code.
 *
 * This macro should be used for classes that need a KComponentData.
 * So instead of writing QTEST_MAIN( TestClass ) you write
 * QTEST_KDEMAIN_CORE( TestClass ).
 *
 * \param TestObject The class you use for testing.
 *
 * \see KDEMainFlag
 * \see QTestLib
 */
#define QTEST_KDEMAIN_CORE(TestObject) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test" ), 1); \
    KAboutData aboutData( "qttest", "qttest", "version" );  \
    KComponentData cData(&aboutData); \
    QCoreApplication app( argc, argv ); \
    app.setApplicationName( "qttest" ); \
    TestObject tc; \
    return QTest::qExec( &tc, argc, argv ); \
}

#endif /* QTEST_KDE_H */


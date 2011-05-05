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
#include <assert.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kurl.h>
#include <QtGui/QApplication>
#include <QtCore/QEventLoop>
#include <QtTest/QSignalSpy>

namespace QTest
{
    /**
     * Starts an event loop that runs until the given signal is received.
     * Optionally the event loop can return earlier on a timeout.
     *
     * \param timeout the timeout in milliseconds
     *
     * \return \p true if the requested signal was received
     *         \p false on timeout
     */
    KDECORE_EXPORT bool kWaitForSignal(QObject *obj, const char *signal, int timeout = 0);
} // namespace QTest

// By default, unit tests get no gui.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1 }; // bitfield, next item is 2!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDEMainFlags)
/**
 * \short QTEST_KDEMAIN variant with additional argument for the main component name
 *
 * This variant is useful for testing application classes which rely on the main
 * component having a specific name (for instance to find xmlgui .rc files).
 *
 * This variant should not be needed in kdelibs's own unit tests.
 *
 * \param TestObject The class you use for testing.
 * \param flags one of KDEMainFlag. This is passed to the QApplication constructor.
 * \param componentName the name that will be given to the main component data.
 *
 * \see KDEMainFlag
 * \see QTestLib
 */
#define QTEST_KDEMAIN_WITH_COMPONENTNAME(TestObject, flags, componentName) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    assert( !QDir::homePath().isEmpty() ); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + QString::fromLatin1("/.kde-unit-test") ), 1); \
    setenv("XDG_DATA_HOME", QFile::encodeName( QDir::homePath() + QString::fromLatin1("/.kde-unit-test/xdg/local") ), 1); \
    setenv("XDG_CONFIG_HOME", QFile::encodeName( QDir::homePath() + QString::fromLatin1("/.kde-unit-test/xdg/config") ), 1); \
    setenv("KDE_SKIP_KDERC", "1", 1); \
    unsetenv("KDE_COLOR_DEBUG"); \
    QFile::remove(QDir::homePath() + QString::fromLatin1("/.kde-unit-test/share/config/qttestrc"));  \
    KAboutData aboutData( QByteArray(componentName), QByteArray(), ki18n("KDE Test Program"), QByteArray("version") );  \
    KDEMainFlags mainFlags = flags;                         \
    KComponentData cData(&aboutData); \
    QApplication app( argc, argv, (mainFlags & GUI) != 0 ); \
    app.setApplicationName( QLatin1String("qttest") ); \
    qRegisterMetaType<KUrl>(); /*as done by kapplication*/ \
    qRegisterMetaType<KUrl::List>(); \
    TestObject tc; \
    KGlobal::ref(); /* don't quit qeventloop after closing a mainwindow */ \
    return QTest::qExec( &tc, argc, argv ); \
}

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
#define QTEST_KDEMAIN(TestObject, flags) QTEST_KDEMAIN_WITH_COMPONENTNAME(TestObject, flags, "qttest")

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
 * \since 4.3
 */
#define QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(TestObject, componentName) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    assert( !QDir::homePath().isEmpty() ); \
    setenv("KDEHOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test" ), 1); \
    setenv("XDG_DATA_HOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test/xdg/local" ), 1); \
    setenv("XDG_CONFIG_HOME", QFile::encodeName( QDir::homePath() + "/.kde-unit-test/xdg/config" ), 1); \
    setenv("KDE_SKIP_KDERC", "1", 1); \
    unsetenv("KDE_COLOR_DEBUG"); \
    QFile::remove(QDir::homePath() + "/.kde-unit-test/share/config/qttestrc");  \
    KAboutData aboutData( QByteArray(componentName), QByteArray(), ki18n("KDE Test Program"), QByteArray("version") );  \
    KComponentData cData(&aboutData); \
    QCoreApplication app( argc, argv ); \
    app.setApplicationName( "qttest" ); \
    qRegisterMetaType<KUrl>(); /*as done by kapplication*/ \
    qRegisterMetaType<KUrl::List>(); \
    TestObject tc; \
    KGlobal::ref(); /* don't quit qeventloop after closing a mainwindow */ \
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
#define QTEST_KDEMAIN_CORE(TestObject) QTEST_KDEMAIN_CORE_WITH_COMPONENTNAME(TestObject, "qttest")

#endif /* QTEST_KDE_H */

#ifndef QTEST_KDE_H
#define QTEST_KDE_H

#include <QtTest/QtTest>
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

// TODO remove AutoDcopRegistration

// By default, unit tests get no gui and no dcop registration.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1, AutoDcopRegistration = 2 }; // bitfield, next item is 4!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)

/**
 * \short KDE Replacement for QTEST_MAIN from QTestLib
 *
 * This macro should be used for classes that need a KInstance or KApplication
 * object. So instead of writing QTEST_MAIN( TestClass ) you write
 * QTEST_KDEMAIN( TestClass, GUI ).
 *
 * \param TestObject The class you use for testing.
 * \param flags one of KDEMainFlag
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
    QApplication app( argc, argv, (mainFlags & GUI) != 0 ); \
    KInstance instance( &aboutData ); \
    app.setApplicationName( "qttest" ); \
    TestObject tc; \
    return QTest::qExec( &tc, argc, argv ); \
}

#endif /* QTEST_KDE_H */


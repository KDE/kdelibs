#ifndef QTTEST_KDE_H
#define QTTEST_KDE_H

#include <QtTest/QtTest>
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

// By default, unit tests get no gui and no dcop registration.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1, AutoDcopRegistration = 2 }; // bitfield, next item is 4!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)

/**
 * \short KDE Replacement for QTEST_MAIN from QTestLib
 *
 * This macro should be used for classes that need a KInstance or KApplication
 * object. So instead of writing QTEST_MAIN( TestClass ) you write
 * QTTEST_KDEMAIN( TestClass, GUI ).
 *
 * \param TestObject The class you use for testing.
 * \param flags one of KDEMainFlag
 *
 * \see KDEMainFlag
 * \see QTestLib
 */
#define QTTEST_KDEMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    KAboutData aboutData( "qttest", "qttest", "version" ); \
    KCmdLineArgs::init( argc, argv, &aboutData );   \
    KDEMainFlags mainFlags( flags );                 \
    if ( (mainFlags & AutoDcopRegistration) == 0 ) \
        KApplication::disableAutoDcopRegistration(); \
    KApplication app( (mainFlags & GUI) != 0 ); \
    TestObject tc; \
    return QTest::qExec( &tc, argc, argv ); \
}

#endif /* QTTEST_KDE_H */


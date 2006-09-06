#ifndef QTEST_KDE_H
#define QTEST_KDE_H

#include <QtTest/QtTest>
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kinstance.h>
#include <qapplication.h>

// By default, unit tests get no gui.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1 }; // bitfield, next item is 2!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDEMainFlags)
/**
 * \short KDE Replacement for QTEST_MAIN from QTestLib
 *
 * This macro should be used for classes that need a KInstance.
 * So instead of writing QTEST_MAIN( TestClass ) you write
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


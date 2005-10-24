#ifndef QTTEST_KDE_H
#define QTTEST_KDE_H

#include "qttest.h"
#include <stdlib.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

// By default, unit tests get no gui and no dcop registration.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1, AutoDcopRegistration = 2 }; // bitfield, next item is 4!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)

#define QTTEST_KDEMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    setenv("LC_ALL", "C", 1); \
    KAboutData aboutData( "qttest", "qttest", "version" ); \
    KCmdLineArgs::init( argc, argv, &aboutData );   \
    KDEMainFlags mainFlags( flags );                 \
    if ( (mainFlags & AutoDcopRegistration) == 0 ) \
        KApplication::disableAutoDcopRegistration(); \
    KApplication app( true, (mainFlags & GUI) != 0 ); \
    TestObject tc; \
    return QtTest::exec( &tc, argc, argv ); \
}

#endif /* QTTEST_KDE_H */


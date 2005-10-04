#ifndef QTTEST_KDE_H
#define QTTEST_KDE_H

#include "qttest.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

// By default, unit tests get no gui and no dcop registration.
// Pass GUI if you use any GUI classes
enum KDEMainFlag { NoGUI = 0, GUI = 1, AutoDcopRegistration = 2 }; // bitfield, next item is 4!
Q_DECLARE_FLAGS(KDEMainFlags, KDEMainFlag)

#ifdef Q_WS_X11

// Ok this is a hack to pass the command line arguments to the test object.
// Fix me when KApplication goes bye bye because this now only works on X11

#define QTTEST_KDEMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    KAboutData aboutData( "qttest", "qttest", "version" ); \
    KDEMainFlags mainFlags( flags );                 \
    if ( (mainFlags & AutoDcopRegistration) == 0 ) \
        KApplication::disableAutoDcopRegistration(); \
    KApplication app( 0, argc, argv, "qtest", true, (mainFlags & GUI) != 0); \
    TestObject tc; \
    return QtTest::exec( &tc, argc, argv ); \
}

#else

// The portable version, but command line args don't work

#define QTTEST_KDEMAIN(TestObject, flags) \
int main(int argc, char *argv[]) \
{ \
    KAboutData aboutData( "qttest", "qttest", "version" ); \
    KCmdLineArgs::init( argc, argv, &aboutData );   \
    KDEMainFlags mainFlags( flags );                 \
    if ( (mainFlags & AutoDcopRegistration) == 0 ) \
        KApplication::disableAutoDcopRegistration(); \
    KApplication app( true, (mainFlags & GUI) != 0 ); \
    TestObject tc; \
    return QtTest::exec( &tc, argc, argv ); \
}

#endif

#endif /* QTTEST_KDE_H */


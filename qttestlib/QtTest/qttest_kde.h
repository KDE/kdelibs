#ifndef QTTEST_KDE_H
#define QTTEST_KDE_H

#include "qttest.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#define QTTEST_KDEMAIN(TestObject, GUIenabled) \
int main(int argc, char *argv[]) \
{ \
    KAboutData aboutData( "qttest", "qttest", "version" ); \
    KCmdLineArgs::init( argc, argv, &aboutData );   \
    KApplication app( true, GUIenabled ); \
    TestObject tc; \
    return QtTest::exec( &tc, argc, argv ); \
}

#endif /* QTTEST_KDE_H */


/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#include "kuniqueapplication.h"
#include "kglobalsettings.h"

#include <stdio.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <QtCore/QTimer>

class TestApp : public KUniqueApplication
{
public:
    TestApp() : KUniqueApplication("TestApp") { }
    virtual int newInstance( );
};


int
TestApp::newInstance( )
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    kDebug() << "NewInstance";
    for ( int i = 0; i < args->count(); i++ )
    {
        kDebug() << "argument " << i << " : " << args->arg(i);
    }

    // Auto-terminate this process, so that we can run it as part of the automated unittests,
    // without ending up with a process lying around
    // You have 10s to call it again, when doing manual testing ;)
    QTimer::singleShot( 10000, this, SLOT(quit()) );

    return 0;
}

int
main(int argc, char *argv[])
{
    KCmdLineOptions options;
    options.add("!+[argument]", ki18n("arguments passed to new instance"));

    KAboutData about("kuniqueapptest", 0, ki18n("kuniqueapptest"), "version");
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();

    if (!TestApp::start())
    {
        return 1;
    }
    TestApp a;

    printf("Running.\n");
    kapp->exec();
    printf("Terminating.\n");

    return 0;
}

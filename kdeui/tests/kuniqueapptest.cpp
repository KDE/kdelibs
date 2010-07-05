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

#include <unistd.h>
#include "kuniqueapplication.h"
#include "kglobalsettings.h"

#include <stdio.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kprocess.h>

#include <QTimer>
#include <QFile>

class TestApp : public KUniqueApplication
{
    Q_OBJECT
public:
    TestApp() : KUniqueApplication("TestApp"), m_callCount(0) { }
    virtual int newInstance();
    int callCount() const { return m_callCount; }

private Q_SLOTS:
    void executeNewChild() {
        // Duplicated from kglobalsettingstest.cpp - make a shared helper method?
        KProcess* proc = new KProcess(this);
        const QString appName = "kuniqueapptest";
#ifdef Q_OS_WIN
        (*proc) << appName + ".exe";
#else
        if (QFile::exists(appName+".shell"))
            (*proc) << "./" + appName+".shell";
        else {
            Q_ASSERT(QFile::exists(appName));
            (*proc) << "./" + appName;
        }
#endif
        proc->start();
    }
private:
    int m_callCount;
};


int TestApp::newInstance()
{
    ++m_callCount;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    kDebug() << "NewInstance";
    for ( int i = 0; i < args->count(); i++ )
    {
        kDebug() << "argument " << i << " : " << args->arg(i);
    }

    return 0;
}

int main(int argc, char *argv[])
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

    // Testcase for the problem coming from the old fork-on-startup solution:
    // the "newInstance" D-Bus call would time out if the app took too much time
    // to be ready.
    //printf("Sleeping.\n");
    //sleep(200);

    QTimer::singleShot( 400, &a, SLOT(executeNewChild()) );
    QTimer::singleShot( 800, &a, SLOT(quit()) );

    printf("Running.\n");
    kapp->exec();
    printf("Terminating.\n");

    Q_ASSERT(a.callCount() == 2);
    const bool ok = a.callCount() == 2;

    return ok ? 0 : 1;
}

#include "kuniqueapptest.moc"

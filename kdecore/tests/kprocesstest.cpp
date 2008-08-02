/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kprocess.h>
#include <qtest_kde.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

class KProcessTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void test_channels();
    void test_setShellCommand();
};

// IOCCC nomination pending

static char **gargv;

static QString recurse(KProcess::OutputChannelMode how)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(gargv[0], QStringList() << QString::number((int)how));
    p.waitForFinished();
    return p.readAllStandardOutput();
}

#define EOUT "foo - stdout"
#define EERR "bar - stderr"
#define POUT "program output:\n"
#define ROUT "received stdout:\n"
#define RERR "received stderr:\n"

#define EO EOUT "\n"
#define EE EERR "\n"
#define TESTCHAN(me,ms,pout,rout,rerr) \
    e = "mode: " ms "\n" POUT pout ROUT rout RERR rerr; \
    a = "mode: " ms "\n" + recurse(KProcess::me); \
    QCOMPARE(a, e)

void KProcessTest::test_channels()
{
#ifdef Q_OS_UNIX
    QString e, a;
    TESTCHAN(SeparateChannels, "separate", "", EO, EE);
    TESTCHAN(ForwardedChannels, "forwarded", EO EE, "", "");
    TESTCHAN(OnlyStderrChannel, "forwarded stdout", EO, "", EE);
    TESTCHAN(OnlyStdoutChannel, "forwarded stderr", EE, EO, "");
    TESTCHAN(MergedChannels, "merged", "", EO EE, "");
#else
    QSKIP("This test needs a UNIX system", SkipSingle);
#endif
}

void KProcessTest::test_setShellCommand()
{
// Condition copied from kprocess.cpp
#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
    QSKIP("This test needs a free UNIX system", SkipSingle);
#else
    KProcess p;

    p.setShellCommand("cat");
    QCOMPARE(p.program(), QStringList() << "/bin/cat");
    p.setShellCommand("true || false");
    QCOMPARE(p.program(), QStringList() << "/bin/sh" << "-c" << "true || false");
#endif
}

static void recursor(char **argv)
{
    if (argv[1]) {
        KProcess p;
        p.setShellCommand("echo " EOUT "; echo " EERR " >&2");
        p.setOutputChannelMode((KProcess::OutputChannelMode)atoi(argv[1]));
        fputs(POUT, stdout);
        fflush(stdout);
        p.execute();
        fputs(ROUT, stdout);
        fputs(p.readAllStandardOutput(), stdout);
        fputs(RERR, stdout);
        fputs(p.readAllStandardError(), stdout);
        exit(0);
    }
    gargv = argv;
}

QTEST_KDEMAIN_CORE( recursor(argv); KProcessTest )

#include "kprocesstest.moc"

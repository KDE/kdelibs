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

#include "kptyprocesstest.h"

#include <kptydevice.h>
#include <qtest_kde.h>

void KPtyProcessTest::test_pty_basic()
{
    KPtyProcess p;
    p.setShellCommand("read -s VAL; echo \"1: $VAL\"; echo \"2: $VAL\" >&2", "/bin/bash");
    p.setPtyChannels(KPtyProcess::AllChannels);
    p.pty()->setEcho(false);
    p.start();
    p.pty()->write("test\n");
    p.pty()->waitForBytesWritten(1000);
    p.waitForFinished(1000);
    p.pty()->waitForReadyRead(1000);
    QString output = p.pty()->readAll();
    QCOMPARE(output, QLatin1String("1: test\r\n2: test\r\n"));
}


void KPtyProcessTest::slotReadyRead()
{
    delay.start(30);
}

void KPtyProcessTest::slotDoRead()
{
    while (sp.pty()->canReadLine())
        log.append('>').append(sp.pty()->readLine()).append("$\n");
    log.append("!\n");
}

void KPtyProcessTest::slotReadEof()
{
    log.append('|').append(sp.pty()->readAll()).append("$\n");
}

void KPtyProcessTest::slotBytesWritten()
{
    log.append('<');
}

static const char * const feeds[] = {
    "bla\n",
    "foo\x04", "bar\n",
    "fooish\nbar\n",
    "\x04",
    0
};

static const char want[] =
    "<>bla\r\n$\n!\n"
    "<!\n<>foobar\r\n$\n!\n"
    "<>fooish\r\n$\n>bar\r\n$\n!\n"
    "<|$\n"
    ;

void KPtyProcessTest::slotStep()
{
    if (feeds[phase]) {
        sp.pty()->write(feeds[phase]);
        phase++;
    }
}

void KPtyProcessTest::test_pty_signals()
{
    sp.setShellCommand("cat; sleep .1");
    sp.setPtyChannels(KPtyProcess::StdinChannel | KPtyProcess::StdoutChannel);
    sp.pty()->setEcho(false);
    connect(sp.pty(), SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(sp.pty(), SIGNAL(readEof()), SLOT(slotReadEof()));
    connect(sp.pty(), SIGNAL(bytesWritten(qint64)), SLOT(slotBytesWritten()));
    QTimer t;
    connect(&t, SIGNAL(timeout()), SLOT(slotStep()));
    t.start(50);
    connect(&delay, SIGNAL(timeout()), SLOT(slotDoRead()));
    delay.setSingleShot(true);
    sp.start();
    sp.pty()->closeSlave();
    phase = 0;
    QTest::kWaitForSignal(&sp, SIGNAL(finished(int, QProcess::ExitStatus)), 1000);
    QCOMPARE(QLatin1String(log), QLatin1String(want));
}


void KPtyProcessTest::test_ctty()
{
    KPtyProcess p;
    p.setShellCommand("echo this is a test > /dev/tty");
    p.execute(1000);
    p.pty()->waitForReadyRead(1000);
    QString output = p.pty()->readAll();
    QCOMPARE(output, QLatin1String("this is a test\r\n"));
}

QTEST_KDEMAIN_CORE( KPtyProcessTest )

#include "kptyprocesstest.moc"

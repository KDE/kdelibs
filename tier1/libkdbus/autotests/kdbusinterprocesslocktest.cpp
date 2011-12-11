/* This file is part of libkdbus

   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>


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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTimer>

#include <QtTest>

#include <kdbusinterprocesslock.h>

#include <stdio.h>

static const char *counterFileName = "kdbusinterprocesslocktest.counter";

void writeCounter(int value)
{
    QFile file(counterFileName);
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    stream << value;
    file.close();
}

int readCounter()
{
    QFile file(counterFileName);
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);

    int value = 0;
    stream >> value;

    file.close();

    return value;
}

void removeCounter()
{
    QFile::remove(counterFileName);
}

QProcess *executeNewChild()
{
    qDebug() << "executeNewChild";

    // Duplicated from kglobalsettingstest.cpp - make a shared helper method?
    QProcess* proc = new QProcess();
    QString appName = "kdbusinterprocesslocktest";
#ifdef Q_OS_WIN
    appName+= ".exe";
#else
    if (QFile::exists(appName+".shell")) {
        appName = "./" + appName+".shell";
    } else {
        Q_ASSERT(QFile::exists(appName));
        appName = "./" + appName;
    }
#endif
    proc->setProcessChannelMode(QProcess::ForwardedChannels);
    proc->start(appName, QStringList() << "child");
    return proc;
}

void work(int id, KDBusInterProcessLock &lock)
{
    for (int i=0; i<10; i++) {
        qDebug("%d: retrieve lock...", id);
        lock.lock();
        qDebug("%d: waiting...", id);
        lock.waitForLockGranted();
        qDebug("%d: retrieved lock", id);

        int value = readCounter() + 1;
        writeCounter(value);
        qDebug("%d: counter updated to %d", id, value);

        lock.unlock();
        qDebug("%d: sleeping", id);
        QTest::qSleep(20);
    }

    qDebug("%d: done", id);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("kdbusinterprocesslocktest");
    QCoreApplication::setOrganizationDomain("kde.org");

    KDBusInterProcessLock lock("myfunnylock");

    if (argc>=2) {
        work(2, lock);
        return 0;
    }

    writeCounter(0);

    QProcess *proc = executeNewChild();
    work(1, lock);

    proc->waitForFinished();
    delete proc;

    int value = readCounter();
    qDebug("Final value: %d", value);

    const bool ok = (value == 20);

    removeCounter();

    return ok ? 0 : 1;
}


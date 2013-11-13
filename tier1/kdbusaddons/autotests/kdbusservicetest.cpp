/* This file is part of libkdbus

   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2011 David Faure <faure@kde.org>
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

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QMetaObject>
#include <QProcess>
#include <QTimer>

#include <kdbusservice.h>

#include <stdio.h>

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject(KDBusService *service)
        : m_callCount(0),
          m_service(service)
    {}

    int callCount() const { return m_callCount; }

private Q_SLOTS:
    void slotActivateRequested(const QStringList &args)
    {
        qDebug() << "Application executed with args" << args;

        ++m_callCount;

        if (m_callCount == 1) {
            Q_ASSERT(args.count() == 1);
            Q_ASSERT(args.at(0) == QLatin1String("dummy call"));
        } else if (m_callCount == 2) {
            Q_ASSERT(args.count() == 2);
            Q_ASSERT(args.at(1) == QLatin1String("bad call"));
            m_service->setExitValue(4);
        } else if (m_callCount == 3) {
            Q_ASSERT(args.count() == 3);
            Q_ASSERT(args.at(1) == QLatin1String("real call"));
            Q_ASSERT(args.at(2) == QLatin1String("second arg"));
            // OK, all done, quit
            QCoreApplication::instance()->quit();
        }
    }

    void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        Q_UNUSED(exitStatus)
        qDebug() << "Process exited with code" << exitCode;
        if (m_callCount == 2) {
            Q_ASSERT(exitCode == 4);
            secondCall();
        }
    }

    void firstCall()
    {
        QStringList args;
        args << "bad call";
        executeNewChild(args);
    }

    void secondCall()
    {
        QStringList args;
        args << "real call" << "second arg";
        executeNewChild(args);
    }

private:
    void executeNewChild(const QStringList &args)
    {
        qDebug();

        // Duplicated from kglobalsettingstest.cpp - make a shared helper method?
        QProcess* proc = new QProcess(this);
        connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));
        QString appName = "kdbusservicetest";
#ifdef Q_OS_WIN
        appName+= ".exe";
#else
        if (QFile::exists(appName+".shell"))
            appName = "./" + appName+".shell";
        else {
            Q_ASSERT(QFile::exists(appName));
            appName = "./" + appName;
        }
#endif
        proc->start(appName, args);
    }

    int m_callCount;
    KDBusService *m_service;
};


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("kdbusservicetest");
    QCoreApplication::setOrganizationDomain("kde.org");

    KDBusService service(KDBusService::Unique);
    TestObject testObject(&service);
    QObject::connect(&service, SIGNAL(activateRequested(QStringList)),
                     &testObject, SLOT(slotActivateRequested(QStringList)));

    // Testcase for the problem coming from the old fork-on-startup solution:
    // the "Activate" D-Bus call would time out if the app took too much time
    // to be ready.
    //printf("Sleeping.\n");
    //sleep(200);
    QStringList args;
    args << "dummy call";

    QMetaObject::invokeMethod(&service, "activateRequested",
            Qt::QueuedConnection, Q_ARG(QStringList,args) );
    QTimer::singleShot( 400, &testObject, SLOT(firstCall()) );

    qDebug() << "Running.";
    a.exec();
    qDebug() << "Terminating.";

    Q_ASSERT(testObject.callCount() == 3);
    const bool ok = testObject.callCount() == 3;

    return ok ? 0 : 1;
}

#include "kdbusservicetest.moc"

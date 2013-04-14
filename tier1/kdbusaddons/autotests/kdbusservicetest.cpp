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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTimer>

#include <kdbusservice.h>

#include <stdio.h>

class TestObject : public QObject
{
    Q_OBJECT
public:
    TestObject()
        : m_callCount(0) { }

    int callCount() const { return m_callCount; }

private Q_SLOTS:
    void slotActivateRequested()
    {
        qDebug();

        ++m_callCount;

        if (m_callCount == 2) { // OK, all done, quit
            QCoreApplication::instance()->quit();
        }
    }

    void executeNewChild()
    {
        qDebug();

        // Duplicated from kglobalsettingstest.cpp - make a shared helper method?
        QProcess* proc = new QProcess(this);
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
        proc->startDetached(appName);
    }

private:
    int m_callCount;
};


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("kdbusservicetest");
    QCoreApplication::setOrganizationDomain("kde.org");

    KDBusService service(KDBusService::Unique);
    TestObject testObject;
    QObject::connect(&service, SIGNAL(activateRequested()), &testObject, SLOT(slotActivateRequested()));

    // Testcase for the problem coming from the old fork-on-startup solution:
    // the "Activate" D-Bus call would time out if the app took too much time
    // to be ready.
    //printf("Sleeping.\n");
    //sleep(200);

    QTimer::singleShot( 0, &service, SIGNAL(activateRequested()) );
    QTimer::singleShot( 400, &testObject, SLOT(executeNewChild()) );

    qDebug() << "Running.";
    a.exec();
    qDebug() << "Terminating.";

    Q_ASSERT(testObject.callCount() == 2);
    const bool ok = testObject.callCount() == 2;

    return ok ? 0 : 1;
}

#include "kdbusservicetest.moc"

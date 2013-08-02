/*
 *  Copyright (C) 2007 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <klocale.h>
#include <kglobal.h>

#include <QtCore/QObject>
#include <QtConcurrent>

#include <QtTest/QtTest>
#include <qtest_kde.h> // kWaitForSignal

static QString testMethod()
{
    return QLatin1String( "application/octet-stream" );
}

class KGlobalTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStaticQString()
    {
        QCOMPARE(testMethod(), QString::fromLatin1("application/octet-stream"));
    }

    void testFindDirectChild()
    {
        QTimer child(this);
        QCOMPARE(KGlobal::findDirectChild<QTimer *>(this), &child);
        QCOMPARE(KGlobal::findDirectChild<QTimer *>(&child), (QTimer*)0);
        QCOMPARE(KGlobal::findDirectChild<QEventLoop *>(this), (QEventLoop*)0);
    }

    // The former implementation of QTest::kWaitForSignal would return
    // false even if the signal was emitted, when the timeout fired too
    // (e.g. due to a breakpoint in gdb).
    void testWaitForSignal()
    {
        QTimer::singleShot(5, this, SLOT(emitSigFoo()));
        QVERIFY(QTest::kWaitForSignal(this, SIGNAL(sigFoo()), 20));
    }

    void testWaitForSignalTimeout()
    {
        QVERIFY(!QTest::kWaitForSignal(this, SIGNAL(sigFoo()), 1));
    }

    // For testing from multiple threads in testThreads
    void testLocale()
    {
        KLocale::global();
        KLocale::global()->setDecimalPlaces(2);
        QCOMPARE(KLocale::global()->formatNumber(70), QString("70.00"));
    }

    // Calling this directly aborts in KLocale::global(), this is intended.
    // We have to install the qtranslator in the main thread.
    void testThreads()
    {
        QThreadPool::globalInstance()->setMaxThreadCount(10);
        QFutureSynchronizer<void> sync;
        sync.addFuture(QtConcurrent::run(this, &KGlobalTest::testLocale));
        sync.addFuture(QtConcurrent::run(this, &KGlobalTest::testLocale));
        // sync dtor blocks waiting for finished
    }

protected Q_SLOTS:
    void emitSigFoo()
    {
        emit sigFoo();
#ifndef Q_OS_MAC
        // Mac currently uses the unix event loop (not glib) which
        // has issues and blocks in nested event loops such as the
        // one triggered by the below. Since this is a limitation
        // of Qt and working around it here doesn't invalidate the
        // actual test case, let's do that.
        QTest::qWait(10);
#endif
    }

Q_SIGNALS:
    void sigFoo();
};

QTEST_MAIN( KGlobalTest )

#include "kglobaltest.moc"

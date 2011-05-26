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

#include <kglobal.h>

#include <QtCore/QObject>

#include <qtest_kde.h>

static QString testMethod()
{
    static const QString & s_strDefaultMimeType =
        KGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
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
        QTimer::singleShot(0, this, SLOT(emitSigFoo()));
        QVERIFY(QTest::kWaitForSignal(this, SIGNAL(sigFoo()), 1));
    }

    void testWaitForSignalTimeout()
    {
        QVERIFY(!QTest::kWaitForSignal(this, SIGNAL(sigFoo()), 1));
    }

    // For testing from multiple threads in testThreads
    void testLocale()
    {
        KGlobal::locale();
        KGlobal::locale()->setDecimalPlaces(2);
        QCOMPARE(KGlobal::locale()->formatNumber(70), QString("70.00"));
    }

    // Calling this directly aborts in KGlobal::locale(), this is intended.
    // We have to install the qtranslator in the main thread.
    void testThreads()
    {
        QThreadPool::globalInstance()->setMaxThreadCount(10);
        QList<QFuture<void> > futures;
        futures << QtConcurrent::run(this, &KGlobalTest::testLocale);
        futures << QtConcurrent::run(this, &KGlobalTest::testLocale);
        Q_FOREACH(QFuture<void> f, futures) // krazy:exclude=foreach
            f.waitForFinished();
    }

protected Q_SLOTS:
    void emitSigFoo()
    {
        emit sigFoo();
        QTest::qWait(10);
    }

Q_SIGNALS:
    void sigFoo();
};

QTEST_KDEMAIN_CORE( KGlobalTest )

#include "kglobaltest.moc"

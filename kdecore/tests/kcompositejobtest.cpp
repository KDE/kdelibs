/*  This file is part of the KDE project
    Copyright (C) 2013 Kevin Funk <kevin@kfunk.org>

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

#include "kcompositejobtest.h"

#include <QtTest/QtTest>
#include <QTimer>

TestJob::TestJob(QObject *parent)
    : KJob(parent)
{
}

void TestJob::start()
{
    QTimer::singleShot(1000, this, SLOT(doEmit()));
}

void TestJob::doEmit()
{
    emitResult();
}

void CompositeJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

bool CompositeJob::addSubjob(KJob* job)
{
    return KCompositeJob::addSubjob(job);
}

void CompositeJob::slotResult(KJob* job)
{
    KCompositeJob::slotResult(job);

    if (!error() && hasSubjobs()) {
        // start next
        subjobs().first()->start();
    } else {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
    }
}


KCompositeJobTest::KCompositeJobTest()
    : loop(this)
{
}

/**
 * In case a composite job is deleted during execution
 * we still want to assure that we don't crash
 *
 * see bug: https://bugs.kde.org/show_bug.cgi?id=230692
 */
void KCompositeJobTest::testDeletionDuringExecution()
{
    QObject *someParent = new QObject;
    KJob *job = new TestJob(someParent);

    CompositeJob* compositeJob = new CompositeJob;
    compositeJob->setAutoDelete(false);
    QVERIFY(compositeJob->addSubjob(job));

    QCOMPARE(job->parent(), compositeJob);

    QSignalSpy destroyed_spy(job, SIGNAL(destroyed(QObject*)));
    // check if job got reparented properly
    delete someParent; someParent = 0;
    // the job should still exist, because it is a child of KCompositeJob now
    QCOMPARE(destroyed_spy.size(), 0);

    // start async, the subjob takes 1 second to finish
    compositeJob->start();

    // delete the job during the execution
    delete compositeJob; compositeJob = 0;
    // at this point, the subjob should be deleted, too
    QCOMPARE(destroyed_spy.size(), 1);
}

QTEST_GUILESS_MAIN( KCompositeJobTest );

#include "kcompositejobtest.moc"

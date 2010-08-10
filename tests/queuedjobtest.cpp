/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "queuedjobtest.h"
#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include <QtCore/QPointer>
#include <jobqueue.h>

TestJob::TestJob(JobQueue *queue) : QueuedJob(queue)
{
}

TestJob::~TestJob()
{
}

bool TestJob::isImmediate() const
{
   return true;
}

void TestJob::exec()
{
   doWork();
}

void TestJob::start()
{
   QTimer::singleShot(0, this, SLOT(doWork()));
}
   
void TestJob::doWork()
{
   // nothing to do really
   emitResult();
}

void QueuedJobTest::initTestCase()
{
   m_queue = new JobQueue;
}

void QueuedJobTest::testSync()
{
   QPointer<TestJob> job = new TestJob(m_queue);
   QVERIFY(job->isImmediate());
   QVERIFY(!job->isFinished());
   job->exec();
   QVERIFY(job->isFinished());
   
   // make sure the job gets deleted
   QEventLoop loop;
   QTimer::singleShot(0, &loop, SLOT(quit()));
   loop.exec();
   QVERIFY(job.isNull());
}

void QueuedJobTest::testAsync()
{
   QPointer<TestJob> job = new TestJob(m_queue);
   QEventLoop loop;
   loop.connect(job, SIGNAL(result(QueuedJob*)), SLOT(quit()));
   QVERIFY(!job->isFinished());
   job->enqueue();
   loop.exec();
   // if we end up here, the job will already be deleted
   QVERIFY(job.isNull() || job->isFinished());
   if (!job.isNull()) {
      // if the job wasn't deleted yet, make sure it's getting deleted
      QTimer::singleShot(0, &loop, SLOT(quit()));
      loop.exec();
      QVERIFY(job.isNull());
   }
}

void QueuedJobTest::testAsyncOrder()
{
   QPointer<TestJob> job1 = new TestJob(m_queue);
   QPointer<TestJob> job2 = new TestJob(m_queue);
   QEventLoop loop1;
   QEventLoop loop2;
   loop1.connect(job1, SIGNAL(result(QueuedJob*)), SLOT(quit()));
   loop2.connect(job2, SIGNAL(result(QueuedJob*)), SLOT(quit()));
   job1->enqueue();
   job2->enqueue();
   loop1.exec();
   loop2.exec();
   QVERIFY(job1.isNull() || job1->isFinished());
   QVERIFY(job2.isNull() || job2->isFinished());
}

void QueuedJobTest::testAsyncOrderInFront()
{
   QPointer<TestJob> job1 = new TestJob(m_queue);
   QPointer<TestJob> job2 = new TestJob(m_queue);
   QEventLoop loop1;
   QEventLoop loop2;
   loop1.connect(job1, SIGNAL(result(QueuedJob*)), SLOT(quit()));
   loop2.connect(job2, SIGNAL(result(QueuedJob*)), SLOT(quit()));
   job1->enqueue();
   job2->enqueue(true);
   loop2.exec();
   loop1.exec();
   QVERIFY(job1.isNull() || job1->isFinished());
   QVERIFY(job2.isNull() || job2->isFinished());
}

void QueuedJobTest::cleanupTestCase()
{
   delete m_queue;
}

QTEST_MAIN(QueuedJobTest)
#include "queuedjobtest.moc"

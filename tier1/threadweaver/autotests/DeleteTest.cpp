#include "DeleteTest.h"

#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

#include "AppendCharacterJob.h"

DeleteTest::DeleteTest()
{
    ThreadWeaver::setDebugLevel ( true,  1 );
    ThreadWeaver::Weaver::instance()->setMaximumNumberOfThreads(4);
}

void DeleteTest::DeleteSequenceTest()
{
    m_finishCount = 100;

    ThreadWeaver::Weaver::instance()->suspend();
    for (int i = 0; i < m_finishCount; ++i) {
        QSharedPointer<ThreadWeaver::JobSequence> jobSeq(new ThreadWeaver::JobSequence);
        QVERIFY(connect(jobSeq.data(), SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(deleteSequence(ThreadWeaver::Job*))));

        jobSeq->addJob(JobPointer(new BusyJob));
        jobSeq->addJob(JobPointer(new BusyJob));

        ThreadWeaver::Weaver::instance()->enqueue(jobSeq);
    }

    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    QEXPECT_FAIL("", "FIXME The done signal does not get emitted for collections", Continue);
    QCOMPARE(m_finishCount, 0);
}

void DeleteTest::deleteSequence(ThreadWeaver::Job* job)
{
    Q_ASSERT(job);
    delete job;

    //TODO atomic int
    QMutexLocker lock(&m_finishMutex);
    --m_finishCount;
    if (m_finishCount == 0)
        exit(0);
}

QMutex s_GlobalMutex;

QTEST_MAIN(DeleteTest)

#include "DeleteTest.moc"

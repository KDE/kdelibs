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
    return; //MARK_TEMPORARILY_DISABLED
    m_finishCount = 100;

    for (int i = 0; i < m_finishCount; ++i) {
        ThreadWeaver::JobSequence* jobSeq = new ThreadWeaver::JobSequence( this );
        connect ( jobSeq, SIGNAL(done(ThreadWeaver::Job*)),
                  SLOT(deleteSequence(ThreadWeaver::Job*)) );

        jobSeq->addJob( new BusyJob );
        jobSeq->addJob( new BusyJob );

        ThreadWeaver::Weaver::instance()->enqueue( jobSeq );
    }

    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
}

void DeleteTest::deleteSequence(ThreadWeaver::Job* job)
{
    Q_ASSERT(job);
    delete job;

    QMutexLocker lock(&m_finishMutex);
    --m_finishCount;
    if (m_finishCount == 0)
        exit(0);
}

QMutex s_GlobalMutex;

QTEST_MAIN(DeleteTest)

#include "DeleteTest.moc"

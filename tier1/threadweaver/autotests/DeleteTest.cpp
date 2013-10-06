#include <QEventLoop>

#include "DeleteTest.h"

#include <JobPointer.h>
#include <QObjectDecorator.h>
#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

#include "AppendCharacterJob.h"

class InstanceCountingJobSequence : public JobSequence {
public:
    explicit InstanceCountingJobSequence()
        : JobSequence()
    {
        instances_.fetchAndAddAcquire(1);
    }

    ~InstanceCountingJobSequence() {
        instances_.fetchAndAddAcquire(-1);
    }

    static int instances() {
        return instances_.loadAcquire();
    }

private:
    static QAtomicInt instances_;
};

QAtomicInt InstanceCountingJobSequence::instances_;

DeleteTest::DeleteTest()
{
    ThreadWeaver::setDebugLevel(true, 1);
    ThreadWeaver::Weaver::instance()->setMaximumNumberOfThreads(4);
}

void DeleteTest::DeleteSequenceTest()
{
    const int NumberOfSequences = 100;
    ThreadWeaver::Weaver::instance()->suspend();
    for (int i = 0; i < NumberOfSequences; ++i) {
        QJobPointer seq(new QObjectDecorator(new InstanceCountingJobSequence));
        seq->sequence()->addJob(JobPointer(new BusyJob));
        seq->sequence()->addJob(JobPointer(new BusyJob));
        QVERIFY(connect(seq.data(), SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(deleteSequence(ThreadWeaver::JobPointer))));

        ThreadWeaver::Weaver::instance()->enqueue(seq);
        m_finishCount.fetchAndAddRelease(1);
    }
    QCOMPARE(m_finishCount.loadAcquire(), NumberOfSequences);
    QEventLoop loop;
    QVERIFY(connect(this, SIGNAL(deleteSequenceTestCompleted()), &loop, SLOT(quit()), Qt::QueuedConnection));
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    loop.exec();
    QCOMPARE(m_finishCount.loadAcquire(), 0);
    // The used Weaver instance needs to be shut down. The threads may still hold a reference to the previous job while
    // waiting for the next one or blocking because the queue is empty. If all threads have exited, no references to any jobs are
    // held anymore.
    ThreadWeaver::Weaver::instance()->shutDown();
    QCOMPARE(InstanceCountingJobSequence::instances(), 0);
}

void DeleteTest::deleteSequence(ThreadWeaver::JobPointer)
{
    if (m_finishCount.fetchAndAddRelease(-1) == 1) { // if it *was* 1...
        emit deleteSequenceTestCompleted();
    }
}

QMutex s_GlobalMutex;

QTEST_MAIN(DeleteTest)

#include "DeleteTest.moc"

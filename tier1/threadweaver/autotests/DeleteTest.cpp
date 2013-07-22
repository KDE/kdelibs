#include <QEventLoop>

#include "DeleteTest.h"

#include <JobPointer.h>
#include <QObjectJobDecorator.h>
#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

#include "AppendCharacterJob.h"

class InstanceCountingJobSequence : public JobSequence {
public:
    explicit InstanceCountingJobSequence()
        : JobSequence()
    {
        ++instances_;
    }

    ~InstanceCountingJobSequence() {
        --instances_;
    }

    static int instances() {
        return instances_;
    }

private:
    static int instances_;
};

int InstanceCountingJobSequence::instances_;

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
        InstanceCountingJobSequence* seq = new InstanceCountingJobSequence;
        seq->addJob(JobPointer(new BusyJob));
        seq->addJob(JobPointer(new BusyJob));
        QObjectJobDecorator *decorated = new QObjectJobDecorator(seq);
        QVERIFY(connect(decorated, SIGNAL(done(ThreadWeaver::JobPointer)), SLOT(deleteSequence(ThreadWeaver::JobPointer))));
        JobPointer jobSeq(decorated);

        ThreadWeaver::Weaver::instance()->enqueue(jobSeq);
        m_finishCount.fetchAndAddRelease(1);
    }
    QCOMPARE(m_finishCount.fetchAndAddRelease(0), NumberOfSequences);
    QEventLoop loop;
    QVERIFY(connect(this, SIGNAL(deleteSequenceTestCompleted()), &loop, SLOT(quit())));
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    loop.exec();
    QCOMPARE(m_finishCount.fetchAndStoreRelease(0), 0);
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

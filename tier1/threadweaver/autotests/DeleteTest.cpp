#include <QEventLoop>

#include "DeleteTest.h"

#include <JobPointer.h>
#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

#include "AppendCharacterJob.h"

class InstanceCountingJobSequence : public JobSequence {
public:
    explicit InstanceCountingJobSequence(QObject* parent = 0)
        : JobSequence(parent)
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
    : m_finishCount(0)
{
    ThreadWeaver::setDebugLevel(true, 1);
    ThreadWeaver::Weaver::instance()->setMaximumNumberOfThreads(4);
}

void DeleteTest::DeleteSequenceTest()
{
    m_finishCount = 100;

    ThreadWeaver::Weaver::instance()->suspend();
    for (int i = 0; i < m_finishCount; ++i) {
        QSharedPointer<InstanceCountingJobSequence> jobSeq(new InstanceCountingJobSequence);
        QVERIFY(connect(jobSeq.data(), SIGNAL(done(ThreadWeaver::JobPointer)),
                        this, SLOT(deleteSequence(ThreadWeaver::JobPointer))));

        jobSeq->addJob(JobPointer(new BusyJob));
        jobSeq->addJob(JobPointer(new BusyJob));

        ThreadWeaver::Weaver::instance()->enqueue(jobSeq);
    }

    QEventLoop loop;
    QVERIFY(connect(this, SIGNAL(deleteSequenceTestCompleted()), &loop, SLOT(quit())));
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    loop.exec();
    QCOMPARE(m_finishCount, 0);
    QCOMPARE(InstanceCountingJobSequence::instances(), 0);
}

void DeleteTest::deleteSequence(ThreadWeaver::JobPointer)
{
    //TODO atomic int
    QMutexLocker lock(&m_finishMutex);
    --m_finishCount;
    if (m_finishCount == 0) {
        Q_EMIT deleteSequenceTestCompleted();
    }
}

QMutex s_GlobalMutex;

QTEST_MAIN(DeleteTest)

#include "DeleteTest.moc"

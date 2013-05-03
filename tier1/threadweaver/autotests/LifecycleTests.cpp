#include <QString>
#include <QtTest>

#include <ThreadWeaver.h>
#include <JobPointer.h>
#include <Job.h>

class NotifyOnDeletejob : public ThreadWeaver::Job {
    Q_OBJECT

public:
    explicit NotifyOnDeletejob(bool& exists)
        : ThreadWeaver::Job()
        , m_exists(exists)
    {
        exists = true;
    }

    ~NotifyOnDeletejob()
    {
        m_exists = false;
    }

    void run() {}

private:
    bool& m_exists;
};

class LifecycleTests : public QObject
{
    Q_OBJECT
    
public:
    LifecycleTests();
    
private Q_SLOTS:
    void testShutdownOnQApplicationQuit();
    void testJobAutoDeletion();
};

LifecycleTests::LifecycleTests()
{
}

void LifecycleTests::testShutdownOnQApplicationQuit()
{
    {
        int argc = 0;
        QCoreApplication app(argc, (char**)0);
        ThreadWeaver::Weaver::instance()->suspend();
        ThreadWeaver::Weaver::instance()->resume();
        QTest::qWait(10);
    }
    QVERIFY(ThreadWeaver::Weaver::instance()==0);
}

void LifecycleTests::testJobAutoDeletion()
{
    bool job1Exists = false;
    bool job2Exists = false;
    ThreadWeaver::JobPointer job2(new NotifyOnDeletejob(job2Exists));
    QCOMPARE(true, job2Exists);
    {
        ThreadWeaver::JobPointer job1(new NotifyOnDeletejob(job1Exists));
        QCOMPARE(true, job1Exists);
    }
    QCOMPARE(false, job1Exists);
    QCOMPARE(true, job2Exists);
}

QTEST_APPLESS_MAIN(LifecycleTests)

#include "LifecycleTests.moc"

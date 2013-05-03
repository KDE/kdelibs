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
    void testJobAutoDeletionBasics();
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

void LifecycleTests::testJobAutoDeletionBasics()
{
    bool job1Exists = false;
    bool job2Exists = false;
    ThreadWeaver::JobPointer job2(new NotifyOnDeletejob(job2Exists));
    QCOMPARE(true, job2Exists);
    {
        ThreadWeaver::JobPointer job1(new NotifyOnDeletejob(job1Exists));
        QCOMPARE(job1Exists, true);
    }
    QCOMPARE(job1Exists, false);
    QCOMPARE(job2Exists, true);
}

void LifecycleTests::testJobAutoDeletion()
{
    bool job1Exists = false;
    bool job2Exists = false;
    {
        ThreadWeaver::JobPointer job1(new NotifyOnDeletejob(job1Exists));
        QCOMPARE(job1Exists, true);
        int argc = 0;
        QCoreApplication app(argc, (char**)0);
        ThreadWeaver::Weaver::instance()->suspend();
        ThreadWeaver::Weaver::instance()->enqueue(job1);
        ThreadWeaver::Weaver::instance()->enqueue(ThreadWeaver::JobPointer(new NotifyOnDeletejob(job2Exists)));
        QCOMPARE(job2Exists, true);
        ThreadWeaver::Weaver::instance()->resume();
        ThreadWeaver::Weaver::instance()->finish();
        QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
        ThreadWeaver::Weaver::instance()->suspend();
        //QCOMPARE(job2Exists, false);
        QCOMPARE(job1Exists, true);
    }
    QVERIFY(ThreadWeaver::Weaver::instance()==0);
    QCOMPARE(job2Exists, false);
    QCOMPARE(false, job1Exists);
}

QTEST_APPLESS_MAIN(LifecycleTests)

#include "LifecycleTests.moc"

#include "QueueTests.h"

#include <QtCore/QChar>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtTest/QtTest>

#include "AppendCharacterJob.h"

#include <Job.h>
#include <State.h>
#include <QueuePolicy.h>
#include <JobSequence.h>
#include <JobCollection.h>
#include <DebuggingAids.h>
#include <WeaverObserver.h>
#include <DependencyPolicy.h>
#include <ResourceRestrictionPolicy.h>

#include <ThreadWeaver.h>
#include <Thread.h>

// always. ahm. no. never. never show your private parts in public.
#ifdef THREADWEAVER_PRIVATE_API
#error "PARTS OF THREADWEAVER'S PRIVATE API ARE INCLUDED IN PUBLIC HEADERS!"
#endif

QMutex s_GlobalMutex;

LowPriorityAppendCharacterJob::LowPriorityAppendCharacterJob (QChar c, QString* stringref, QObject* parent)
    : AppendCharacterJob (c, stringref, parent)
{}

int LowPriorityAppendCharacterJob ::priority() const
{
    return -1;
}

HighPriorityAppendCharacterJob::HighPriorityAppendCharacterJob (QChar c, QString* stringref,
                                                                QObject* parent)
    : AppendCharacterJob (c, stringref, parent)
{}

int HighPriorityAppendCharacterJob::priority() const
{
    return 1;
}

SecondThreadThatQueues::SecondThreadThatQueues()
    : QThread()
{
}

void SecondThreadThatQueues::run ()
{
    QString sequence;
    AppendCharacterJob a( 'a', &sequence );

    ThreadWeaver::Weaver::instance()->enqueue ( &a );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE( sequence, QString("a" ) );
}

QueueTests::QueueTests( QObject* parent )
    : QObject( parent )
    , autoDeleteJob ( 0 )
{
}

void QueueTests::initTestCase ()
{
    ThreadWeaver::setDebugLevel ( true,  1 );
}

void QueueTests::SimpleQueuePrioritiesTest() {
    ThreadWeaver::Weaver weaver ( 0 );
    weaver.setMaximumNumberOfThreads ( 1 ); // just one thread
    QString sequence;
    LowPriorityAppendCharacterJob jobA ( QChar( 'a' ), &sequence );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence );
    HighPriorityAppendCharacterJob jobC ( QChar( 'c' ), &sequence );

    // queue low priority, then normal priority, then high priority
    // if priorities are processed correctly, the jobs will be executed in reverse order

    weaver.suspend();

    weaver.enqueue ( & jobA );
    weaver.enqueue ( & jobB );
    weaver.enqueue ( & jobC );

    weaver.resume();
    weaver.finish();

    QCOMPARE ( sequence, QString ("cba" ) );
}

void QueueTests::WeaverInitializationTest()
{ // this one mostly tests the sanity of the startup behaviour
    ThreadWeaver::Weaver weaver;
    QCOMPARE (weaver.currentNumberOfThreads(), 0);
    QVERIFY (weaver.isEmpty());
    QVERIFY(weaver.isIdle());
    QVERIFY(weaver.queueLength() == 0);
    weaver.finish();
}

void QueueTests::QueueFromSecondThreadTest()
{
    SecondThreadThatQueues thread;
    thread.start();
    thread.wait();
    QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
}

void QueueTests::deleteJob( ThreadWeaver::Job* job)
{   // test that signals are properly emited (asynchroneously, that is):
    QVERIFY( thread() == QThread::currentThread() );
    QVERIFY( job == autoDeleteJob );
    delete job;
    autoDeleteJob = 0;
}

void QueueTests::DeleteDoneJobsFromSequenceTest()
{
    QString sequence;
    autoDeleteJob = new AppendCharacterJob( QChar( 'a' ), &sequence );
    AppendCharacterJob b( QChar( 'b' ), &sequence );
    AppendCharacterJob c( QChar( 'c' ), &sequence );
    ThreadWeaver::JobCollection jobCollection( this );
    jobCollection.addJob ( autoDeleteJob );
    jobCollection.addJob ( &b );
    jobCollection.addJob ( &c);

    QVERIFY( autoDeleteJob != 0 );
    connect( autoDeleteJob, SIGNAL( done( ThreadWeaver::Job* ) ),
             SLOT( deleteJob( ThreadWeaver::Job* ) ) );
    ThreadWeaver::Weaver::instance()->enqueue ( &jobCollection );
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    ThreadWeaver::Weaver::instance()->finish();
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    // no need to delete a, that should be done in deleteJob
    QVERIFY( autoDeleteJob == 0 );
}

void QueueTests::deleteCollection( Job* collection )
{
    QVERIFY( thread() == QThread::currentThread() );
    QVERIFY( collection = autoDeleteCollection );
    delete autoDeleteCollection;
    autoDeleteCollection = 0;
}

void QueueTests::DeleteCollectionOnDoneTest()
{
    QString sequence;
    autoDeleteCollection = new ThreadWeaver::JobCollection( this );
    connect ( autoDeleteCollection, SIGNAL( done( ThreadWeaver::Job* ) ),
              SLOT( deleteCollection( ThreadWeaver::Job* ) ) );

    AppendCharacterJob a( QChar( 'a' ), &sequence );
    AppendCharacterJob b( QChar( 'b' ), &sequence );
    autoDeleteCollection->addJob( &a );
    autoDeleteCollection->addJob( &b );

    ThreadWeaver::Weaver::instance()->enqueue( autoDeleteCollection );
    // return to event queue to make sure signals are delivered
    // (otherwise, no slot calls would happen before the end of this function)
    // I assume the amount of time that we wait does not matter
    QTest::qWait(10);
    ThreadWeaver::Weaver::instance()->finish();
    // return to event queue to make sure signals are delivered
    QTest::qWait(10);
    // no need to delete a, that should be done in deleteJob
    QVERIFY( sequence.length() == 2 );
    QVERIFY( autoDeleteCollection == 0 );
}

QTEST_MAIN ( QueueTests )

#include "QueueTests.moc"

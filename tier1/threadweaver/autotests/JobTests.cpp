#include "JobTests.h"

#include <cstdlib>

#include <QtCore/QMutex>
#include <QtTest/QtTest>
#include <QSignalSpy>

#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>
#include <JobCollection.h>
#include <ResourceRestrictionPolicy.h>
#include <DependencyPolicy.h>

#include "AppendCharacterJob.h"
#include "AppendCharacterAndVerifyJob.h"

QMutex s_GlobalMutex;

//Ensure that after the object is created, the weaver is idle and resumed.
//Upon desruction, ensure the weaver is idle and suspended.
class WaitForIdleAndFinished {
public:
    explicit WaitForIdleAndFinished(ThreadWeaver::Weaver* weaver)
        : weaver_(weaver)
    {
        Q_ASSERT(weaver);
        weaver_->finish();
        Q_ASSERT(weaver_->isIdle());
        weaver_->resume();
    }

    ~WaitForIdleAndFinished() {
        weaver_->resume();
        weaver_->dequeue();
        weaver_->finish();
        weaver_->suspend();
        Q_ASSERT(weaver_->isIdle());
    }
private:
    ThreadWeaver::Weaver* weaver_;
};

void JobTests::initTestCase ()
{
    ThreadWeaver::setDebugLevel(true, 1);
}

// call finish() before leave a test to make sure the queue is empty

void JobTests::WeaverLazyThreadCreationTest()
{
    ThreadWeaver::Weaver weaver;
    QString sequence;
    AppendCharacterJob a( QChar('a'), &sequence, this);

    WaitForIdleAndFinished w(&weaver);
    Q_ASSERT(weaver.isIdle());
    QCOMPARE (weaver.currentNumberOfThreads(), 0);
    weaver.enqueue( & a);
    weaver.finish();
    QVERIFY(a.isFinished());
    QCOMPARE (weaver.currentNumberOfThreads(), 1);
    Q_ASSERT(weaver.isIdle());
}

void JobTests::SimpleJobTest() {
    QString sequence;
    AppendCharacterJob job( QChar( '1' ), &sequence, this );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &job );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "1" ) );
}

void JobTests::SimpleJobCollectionTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobCollection jobCollection( this );
    jobCollection.addJob ( &jobA );
    jobCollection.addJob ( &jobB );
    jobCollection.addJob ( &jobC );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &jobCollection );
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY( sequence.length() == 3 );
    QVERIFY( sequence.count( 'a' ) == 1 );
    QVERIFY( sequence.count( 'b' ) == 1 );
    QVERIFY( sequence.count( 'c' ) == 1 );
}

void JobTests::EmptyJobCollectionTest() {
    ThreadWeaver::JobCollection collection;

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    Q_ASSERT(ThreadWeaver::Weaver::instance()->isIdle());
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection.isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::CollectionQueueingTest()
{
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobCollection jobCollection( this );
    jobCollection.addJob ( &jobA );
    jobCollection.addJob ( &jobB );
    jobCollection.addJob ( &jobC );

    ThreadWeaver::Weaver weaver;
    WaitForIdleAndFinished w(&weaver);
    weaver.suspend();
    weaver.enqueue(&jobCollection);
    QCOMPARE(weaver.queueLength(), 1); //collection queues itself, and it's elements upon execution of self
    weaver.resume();
    weaver.finish();
    QCOMPARE(sequence.length(), 3);
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::ShortJobSequenceTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence( this );
    jobSequence.addJob ( &jobA );
    jobSequence.addJob ( &jobB );
    jobSequence.addJob ( &jobC );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QLatin1String("abc"));
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::EmptyJobSequenceTest() {
    ThreadWeaver::JobSequence sequence;

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    Q_ASSERT(ThreadWeaver::Weaver::instance()->isIdle());
    QSignalSpy doneSignalSpy(&sequence, SIGNAL(done(ThreadWeaver::Job*)));
    QCOMPARE(doneSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue ( &sequence );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(sequence.isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE(doneSignalSpy.count(), 1);
}

/** This test verifies that the done signal for a collection is only sent after all element of the collection have completed. */
void JobTests::IncompleteCollectionTest()
{
    QString result;
    AppendCharacterJob jobA ( QChar( 'a' ), &result, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &result, this ); //jobB does not get added to the sequence and queued
    ThreadWeaver::JobCollection collection;
    collection.addJob(&jobA);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::DependencyPolicy::instance().addDependency(&jobA, &jobB);
    QSignalSpy collectionDoneSignalSpy(&collection, SIGNAL(done(ThreadWeaver::Job*)));
    QSignalSpy jobADoneSignalSpy(&jobA, SIGNAL(done(ThreadWeaver::Job*)));
    QCOMPARE(collectionDoneSignalSpy.count(), 0);
    QCOMPARE(jobADoneSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    ThreadWeaver::Weaver::instance()->resume();
    QTest::qWait(500);
    QCOMPARE(collectionDoneSignalSpy.count(), 0);
    QCOMPARE(jobADoneSignalSpy.count(), 0);
    ThreadWeaver::DependencyPolicy::instance().removeDependency(&jobA, &jobB);
    ThreadWeaver::Weaver::instance()->finish();
    QTest::qWait(100);
    QVERIFY(collection.isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE(collectionDoneSignalSpy.count(), 1);
    QCOMPARE(jobADoneSignalSpy.count(), 1);
}

/** This test verifies that started() is emitted for a collection at the time the first of any elements of the collection gets
 * executed. */
void JobTests::EmitStartedOnFirstElementTest()
{
    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    QString result;
    AppendCharacterJob jobA ( QChar( 'a' ), &result, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &result, this );
    ThreadWeaver::JobCollection collection;
    collection.addJob(&jobA);
    collection.addJob(&jobB);
    ThreadWeaver::Weaver::instance()->enqueue(&collection);
    QSignalSpy collectionStartedSignalSpy(&collection, SIGNAL(started(ThreadWeaver::Job*)));
    QSignalSpy collectionDoneSignalSpy(&collection, SIGNAL(done(ThreadWeaver::Job*)));
    ThreadWeaver::Weaver::instance()->resume();
    QTest::qWait(500);
    QCOMPARE(collectionStartedSignalSpy.count(), 1);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection.isFinished());
    QCOMPARE(collectionDoneSignalSpy.count(), 1);
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

/* This test verifies that all elements of a collection are only executed after all dependencies for the collection
 * itself have been resolved.
 * Previous tests have already verified that collections without dependencies get executes right away. */
void JobTests::CollectionDependenciesTest()
{
    QString result;
    AppendCharacterJob jobA(QChar('a'), &result, this);
    AppendCharacterJob jobB(QChar('b'), &result, this);
    AppendCharacterJob jobC(QChar('c'), &result, this);
    ThreadWeaver::JobCollection collection;
    QSignalSpy collectionStartedSignalSpy(&collection, SIGNAL(started(ThreadWeaver::Job*)));
    collection.addJob(&jobA);
    collection.addJob(&jobB);
    ThreadWeaver::DependencyPolicy::instance().addDependency(&collection, &jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(&collection);
    ThreadWeaver::Weaver::instance()->resume();
    QTest::qWait(500);
    QCOMPARE(collectionStartedSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue(&jobC);
    QTest::qWait(500);
    QCOMPARE(collectionStartedSignalSpy.count(), 1);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection.isFinished());
    QVERIFY(result.startsWith(jobC.character()));
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::QueueAndDequeueSequenceTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence( this );
    jobSequence.addJob ( &jobA );
    jobSequence.addJob ( &jobB );
    jobSequence.addJob ( &jobC );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence );
    ThreadWeaver::Weaver::instance()->dequeue ( & jobSequence );
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}

void JobTests::RecursiveSequenceTest()
{
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    AppendCharacterJob jobD ( QChar( 'd' ), &sequence, this );
    AppendCharacterJob jobE ( QChar( 'e' ), &sequence, this );
    AppendCharacterJob jobF ( QChar( 'f' ), &sequence, this );
    AppendCharacterJob jobG ( QChar( 'g' ), &sequence, this );
    AppendCharacterJob jobH ( QChar( 'h' ), &sequence, this );
    AppendCharacterJob jobI ( QChar( 'i' ), &sequence, this );
    AppendCharacterJob jobJ ( QChar( 'j' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.setObjectName( "Sequ_1" );
    jobSequence1.addJob ( &jobA );
    jobSequence1.addJob ( &jobB );
    jobSequence1.addJob ( &jobC );
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.setObjectName( "Sequ_2" );
    jobSequence2.addJob ( &jobD );
    jobSequence2.addJob ( &jobE );
    jobSequence2.addJob ( &jobF );
    ThreadWeaver::JobSequence jobSequence3( this );
    jobSequence3.setObjectName( "Sequ_3" );
    jobSequence3.addJob ( &jobG );
    jobSequence3.addJob ( &jobH );
    jobSequence3.addJob ( &jobI );
    jobSequence3.addJob ( &jobJ );
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    ThreadWeaver::JobSequence jobSequence4( this );
    jobSequence4.setObjectName( "Sequ_4" );
    jobSequence4.addJob ( &jobSequence1 );
    jobSequence4.addJob ( &jobSequence2 );
    jobSequence4.addJob ( &jobSequence3 );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence4 );
//    ThreadWeaver::DependencyPolicy::instance().dumpJobDependencies();
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QLatin1String("abcdefghij"));
}

void JobTests::RecursiveQueueAndDequeueSequenceTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    AppendCharacterJob jobD ( QChar( 'd' ), &sequence, this );
    AppendCharacterJob jobE ( QChar( 'e' ), &sequence, this );
    AppendCharacterJob jobF ( QChar( 'f' ), &sequence, this );
    AppendCharacterJob jobG ( QChar( 'g' ), &sequence, this );
    AppendCharacterJob jobH ( QChar( 'h' ), &sequence, this );
    AppendCharacterJob jobI ( QChar( 'i' ), &sequence, this );
    AppendCharacterJob jobJ ( QChar( 'j' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.setObjectName( "Sequ_1" );
    jobSequence1.addJob ( &jobA );
    jobSequence1.addJob ( &jobB );
    jobSequence1.addJob ( &jobC );
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.setObjectName( "Sequ_2" );
    jobSequence2.addJob ( &jobD );
    jobSequence2.addJob ( &jobE );
    jobSequence2.addJob ( &jobF );
    ThreadWeaver::JobSequence jobSequence3( this );
    jobSequence3.setObjectName( "Sequ_3" );
    jobSequence3.addJob ( &jobG );
    jobSequence3.addJob ( &jobH );
    jobSequence3.addJob ( &jobI );
    jobSequence3.addJob ( &jobJ );
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    ThreadWeaver::JobSequence jobSequence4( this );
    jobSequence4.setObjectName( "Sequ_4" );
    jobSequence4.addJob ( &jobSequence1 );
    jobSequence4.addJob ( &jobSequence2 );
    jobSequence4.addJob ( &jobSequence3 );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence4 );
    ThreadWeaver::Weaver::instance()->dequeue ( & jobSequence4 );
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
}

void JobTests::QueueAndDequeueAllSequenceTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence( this );
    jobSequence.addJob ( &jobA );
    jobSequence.addJob ( &jobB );
    jobSequence.addJob ( &jobC );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence );
    ThreadWeaver::Weaver::instance()->dequeue ();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}

void JobTests::RecursiveQueueAndDequeueAllSequenceTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    AppendCharacterJob jobD ( QChar( 'd' ), &sequence, this );
    AppendCharacterJob jobE ( QChar( 'e' ), &sequence, this );
    AppendCharacterJob jobF ( QChar( 'f' ), &sequence, this );
    AppendCharacterJob jobG ( QChar( 'g' ), &sequence, this );
    AppendCharacterJob jobH ( QChar( 'h' ), &sequence, this );
    AppendCharacterJob jobI ( QChar( 'i' ), &sequence, this );
    AppendCharacterJob jobJ ( QChar( 'j' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.setObjectName( "Sequ_1" );
    jobSequence1.addJob ( &jobA );
    jobSequence1.addJob ( &jobB );
    jobSequence1.addJob ( &jobC );
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.setObjectName( "Sequ_2" );
    jobSequence2.addJob ( &jobD );
    jobSequence2.addJob ( &jobE );
    jobSequence2.addJob ( &jobF );
    ThreadWeaver::JobSequence jobSequence3( this );
    jobSequence3.setObjectName( "Sequ_3" );
    jobSequence3.addJob ( &jobG );
    jobSequence3.addJob ( &jobH );
    jobSequence3.addJob ( &jobI );
    jobSequence3.addJob ( &jobJ );
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    ThreadWeaver::JobSequence jobSequence4( this );
    jobSequence4.setObjectName( "Sequ_4" );
    jobSequence4.addJob ( &jobSequence1 );
    jobSequence4.addJob ( &jobSequence2 );
    jobSequence4.addJob ( &jobSequence3 );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence4 );
    ThreadWeaver::Weaver::instance()->dequeue ();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
}

//     This test is not the most efficient, as the mutex locking takes most of
//     the execution time. Anyway, it will fail if the jobs are not executed
//     in the right order, and the order is randomized.
void JobTests::MassiveJobSequenceTest() {
    return; //MARK_TEMPORARILY_DISABLED
    const int NoOfChars = 1024;
    const char* Alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int SizeOfAlphabet = strlen( Alphabet );
    AppendCharacterAndVerifyJob jobs[NoOfChars];
    ThreadWeaver::JobSequence jobSequence( this );
    QString sequence;
    QString in;

    srand ( 1 );
    in.reserve( NoOfChars );
    sequence.reserve ( NoOfChars );
    for ( int i = 0; i<NoOfChars; ++i ) {
        const int position = static_cast<int> ( SizeOfAlphabet * ( ( 1.0 * rand() ) / RAND_MAX ) );
        Q_ASSERT ( 0 <= position && position < SizeOfAlphabet );
        QChar c( Alphabet[position] );
        in.append ( c );
    }

    for ( int i = 0; i<NoOfChars; ++i ) {
        jobs[i].setValues( in.at(i), &sequence, in );
        jobSequence.addJob ( & ( jobs[i] ) );
    }

    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE ( sequence, in );
}

void JobTests::SimpleRecursiveSequencesTest() {
    QString sequence;
    AppendCharacterJob jobB(QChar('b'), &sequence, this);
    ThreadWeaver::JobSequence jobSequence1(this);
    jobSequence1.addJob(&jobB);

    AppendCharacterJob jobC(QChar('c'), &sequence, this);
    AppendCharacterJob jobA(QChar('a'), &sequence, this);
    ThreadWeaver::JobSequence jobSequence2(this);
    jobSequence2.addJob(&jobA);
    jobSequence2.addJob(&jobSequence1);
    jobSequence2.addJob(&jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence2 );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QString("abc"));
}

/* TODO: reenable, they currently hang up */
/*
void JobTests::SequenceOfSequencesTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    AppendCharacterJob jobD ( QChar( 'd' ), &sequence, this );
    AppendCharacterJob jobE ( QChar( 'e' ), &sequence, this );
    AppendCharacterJob jobF ( QChar( 'f' ), &sequence, this );
    AppendCharacterJob jobG ( QChar( 'g' ), &sequence, this );
    AppendCharacterJob jobH ( QChar( 'h' ), &sequence, this );
    AppendCharacterJob jobI ( QChar( 'i' ), &sequence, this );
    AppendCharacterJob jobJ ( QChar( 'j' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.setObjectName( "Sequ_1" );
    jobSequence1.addJob ( &jobA );
    jobSequence1.addJob ( &jobB );
    jobSequence1.addJob ( &jobC );
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.setObjectName( "Sequ_2" );
    jobSequence2.addJob ( &jobD );
    jobSequence2.addJob ( &jobE );
    jobSequence2.addJob ( &jobF );
    ThreadWeaver::JobSequence jobSequence3( this );
    jobSequence3.setObjectName( "Sequ_3" );
    jobSequence3.addJob ( &jobG );
    jobSequence3.addJob ( &jobH );
    jobSequence3.addJob ( &jobI );
    jobSequence3.addJob ( &jobJ );
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    ThreadWeaver::JobSequence jobSequence4( this );
    jobSequence4.setObjectName( "Sequ_4" );
    jobSequence4.addJob ( &jobSequence1 );
    jobSequence4.addJob ( &jobSequence2 );
    jobSequence4.addJob ( &jobSequence3 );

    // now go already:
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence4 );
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "abcdefghij" ) );
}

*/

void JobTests::QueueAndStopTest() {
    QString sequence;
    AppendCharacterJob a( 'a', &sequence );
    AppendCharacterJob b( 'b', &sequence );
    AppendCharacterJob c( 'c', &sequence );
    FailingAppendCharacterJob d( 'd', &sequence );
    AppendCharacterJob e( 'e', &sequence );
    AppendCharacterJob f( 'f', &sequence );
    AppendCharacterJob g( 'g', &sequence );
    ThreadWeaver::JobSequence jobSequence ( this );
    jobSequence.addJob( &a );
    jobSequence.addJob( &b );
    jobSequence.addJob( &c );
    jobSequence.addJob( &d );
    jobSequence.addJob( &e );
    jobSequence.addJob( &f );
    jobSequence.addJob( &g );

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "abcd" ) );
}

void JobTests::ResourceRestrictionPolicyBasicsTest () {
    // this test tests that with resource restrictions assigned, jobs
    // still get executed as expected
    QString sequence;
    ThreadWeaver::ResourceRestrictionPolicy restriction (2);
    AppendCharacterJob a( 'a', &sequence );
    AppendCharacterJob b( 'b', &sequence );
    AppendCharacterJob c( 'c', &sequence );
    AppendCharacterJob d( 'd', &sequence );
    AppendCharacterJob e( 'e', &sequence );
    AppendCharacterJob f( 'f', &sequence );
    AppendCharacterJob g( 'g', &sequence );
    ThreadWeaver::JobCollection collection;
    collection.addJob( &a );
    a.assignQueuePolicy ( &restriction);
    collection.addJob( &b );
    b.assignQueuePolicy ( &restriction);
    collection.addJob( &c );
    c.assignQueuePolicy ( &restriction);
    collection.addJob( &d );
    d.assignQueuePolicy ( &restriction);
    collection.addJob( &e );
    e.assignQueuePolicy ( &restriction);
    collection.addJob( &f );
    f.assignQueuePolicy ( &restriction);
    collection.addJob( &g );
    g.assignQueuePolicy ( &restriction);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
}

void JobTests::jobStarted( ThreadWeaver::Job* )
{
    // qDebug() << "jobStarted";
    QVERIFY( thread() == QThread::currentThread() );
}

void JobTests::jobDone( ThreadWeaver::Job* )
{
    // qDebug() << "jobDone";
    QVERIFY( thread() == QThread::currentThread() );
}

void JobTests::JobSignalsAreEmittedAsynchronouslyTest()
{
    char bits[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
    const int NumberOfBits = sizeof bits / sizeof bits[0];
    QString sequence;
    QList<Job*> jobs;
    ThreadWeaver::JobCollection collection;
    connect( &collection, SIGNAL(started(ThreadWeaver::Job*)),
             SLOT(jobStarted(ThreadWeaver::Job*)) );
    connect( &collection, SIGNAL(done(ThreadWeaver::Job*)),
             SLOT(jobDone(ThreadWeaver::Job*)) );
    for ( int counter = 0; counter < NumberOfBits; ++counter )
    {
        Job* job = new AppendCharacterJob( bits[counter], &sequence, this );

        connect ( job, SIGNAL(started(ThreadWeaver::Job*)),
                  SLOT(jobStarted(ThreadWeaver::Job*)) );
        connect ( job, SIGNAL(done(ThreadWeaver::Job*)),
                  SLOT(jobDone(ThreadWeaver::Job*)) );

        jobs.append( job );
        collection.addJob( job );
    }

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    QTest::qWait( 100 );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY( sequence.length() == NumberOfBits );
}

void JobTests::DequeueSuspendedSequence()
{
    return; //MARK_TEMPORARILY_DISABLED
    ThreadWeaver::Weaver weaver;
    weaver.suspend();
    weaver.enqueue(new ThreadWeaver::JobSequence(this) );
    weaver.dequeue();
    // don't crash
}

QTEST_MAIN ( JobTests )

#include "JobTests.moc"

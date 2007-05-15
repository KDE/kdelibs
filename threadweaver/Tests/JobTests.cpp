#include "JobTests.h"

#include <cstdlib>

#include <QtCore/QMutex>
#include <QtTest/QtTest>

#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>
#include <JobCollection.h>
#include <ResourceRestrictionPolicy.h>

#include "AppendCharacterJob.h"

QMutex s_GlobalMutex;

void JobTests::initTestCase ()
{
    ThreadWeaver::setDebugLevel ( true,  1 );
}

// call finish() before leave a test to make sure the queue is empty

void JobTests::WeaverLazyThreadCreationTest()
{
    ThreadWeaver::Weaver weaver;
    QString sequence;
    QCOMPARE (weaver.currentNumberOfThreads(), 0);
    AppendCharacterJob a( QChar('a'), &sequence, this);
    weaver.enqueue( & a);
    weaver.finish();
    QVERIFY(a.isFinished());
    QCOMPARE (weaver.currentNumberOfThreads(), 1);
}

void JobTests::SimpleJobTest() {
    QString sequence;
    AppendCharacterJob job( QChar( '1' ), &sequence, this );
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
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection.isFinished());
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

    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "abc" ) );
}

void JobTests::EmptyJobSequenceTest() {
    ThreadWeaver::JobSequence sequence;
    ThreadWeaver::Weaver::instance()->enqueue ( &sequence );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(sequence.isFinished());
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

    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence );
    ThreadWeaver::Weaver::instance()->dequeue ( & jobSequence );
    bool empty = ThreadWeaver::Weaver::instance()->isEmpty();
    ThreadWeaver::Weaver::instance()->resume();

    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY ( empty == true );
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

    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence4 );
    ThreadWeaver::Weaver::instance()->dequeue ( & jobSequence4 );
    bool empty = ThreadWeaver::Weaver::instance()->isEmpty();
    ThreadWeaver::Weaver::instance()->resume();

    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY ( empty == true );
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

    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence );
    ThreadWeaver::Weaver::instance()->dequeue ();
    bool empty = ThreadWeaver::Weaver::instance()->isEmpty();

    ThreadWeaver::Weaver::instance()->resume();

    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY ( empty == true );
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

    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue ( & jobSequence4 );
    ThreadWeaver::Weaver::instance()->dequeue ();
    bool empty = ThreadWeaver::Weaver::instance()->isEmpty();
    ThreadWeaver::Weaver::instance()->resume();

    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY ( empty == true );
}

//     This test is not the most efficient, as the mutex locking takes most of
//     the execution time. Anyway, it will fail if the jobs are not executed
//     in the right order, and the order is randomized.
void JobTests::MassiveJobSequenceTest() {
    const int NoOfChars = 1024;
    const char* Alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int SizeOfAlphabet = strlen( Alphabet );
    AppendCharacterJob jobs [NoOfChars];
    ThreadWeaver::JobSequence jobSequence( this );
    QString sequence;
    QString in;

    srand ( 1 );
    in.reserve( NoOfChars );
    sequence.reserve ( NoOfChars );

    for ( int i = 0; i<NoOfChars; ++i )
    {
        const int position = static_cast<int> ( SizeOfAlphabet * ( ( 1.0 * rand() ) / RAND_MAX ) );
        Q_ASSERT ( 0 <= position && position < SizeOfAlphabet );
        QChar c( Alphabet[position] );
        in.append ( c );
        jobs[i].setValues( c, &sequence );
        jobSequence.addJob ( & ( jobs[i] ) );
    }
    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, in );
}

void JobTests::SimpleRecursiveSequencesTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.addJob( &jobB );
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.addJob( &jobA );
    jobSequence2.addJob( &jobSequence1 );
    jobSequence2.addJob( &jobC );

    ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence2 );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "abc" ) );
}

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
    connect( &collection, SIGNAL( started( ThreadWeaver::Job* ) ),
             SLOT( jobStarted( ThreadWeaver::Job* ) ) );
    connect( &collection, SIGNAL( done( ThreadWeaver::Job* ) ),
             SLOT( jobDone( ThreadWeaver::Job* ) ) );
    for ( int counter = 0; counter < NumberOfBits; ++counter )
    {
        Job* job = new AppendCharacterJob( bits[counter], &sequence, this );

        connect ( job, SIGNAL( started( ThreadWeaver::Job* ) ),
                  SLOT( jobStarted( ThreadWeaver::Job* ) ) );
        connect ( job, SIGNAL( done( ThreadWeaver::Job* ) ),
                  SLOT( jobDone( ThreadWeaver::Job* ) ) );

        jobs.append( job );
        collection.addJob( job );
    }

    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    QTest::qWait( 100 );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY( sequence.length() == NumberOfBits );
}

QTEST_MAIN ( JobTests )

#include "JobTests.moc"

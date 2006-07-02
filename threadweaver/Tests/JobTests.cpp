#include <cstdlib>

#include <QMutex>
#include <QMutexLocker>
#include <QtTest/QtTest>

#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>
#include <JobCollection.h>
#include <ResourceRestrictionPolicy.h>

QMutex s_GlobalMutex;

class AppendCharacterJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    AppendCharacterJob ( QChar c = QChar(), QString* stringref = 0 , QObject* parent = 0 )
        : ThreadWeaver::Job ( parent )
    {
        setValues( c, stringref );
    }

    void setValues ( QChar c, QString* stringref )
    {
        m_c = c;
        m_stringref = stringref;
        setObjectName ( tr ( "Job_" ) + m_c );
    }

    void run()
    {
        QMutexLocker locker ( &s_GlobalMutex );
        m_stringref->append( m_c );
        using namespace ThreadWeaver;
        debug( 3, "AppendCharacterJob::run: %c appended, result is %s.\n",
               m_c.toAscii(), qPrintable( *m_stringref ) );
    }

private:
    QChar m_c;
    QString* m_stringref;
};

class FailingAppendCharacterJob : public AppendCharacterJob
{
    Q_OBJECT

public:
    FailingAppendCharacterJob ( QChar c = QChar(), QString* stringref = 0, QObject* parent = 0 )
        : AppendCharacterJob ( c, stringref, parent )
    {
    }

    bool success () const
    {
        return false;
    }
};

class JobTests : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase ()
    {
        ThreadWeaver::setDebugLevel ( true,  1 );
    }

    // call finish() before leave a test to make sure the queue is empty

  void WeaverInitializationTest()
  { // this one mostly tests the sanity of the startup behaviour
    ThreadWeaver::Weaver weaver;
    QCOMPARE (weaver.noOfThreads(), 0);
    QVERIFY (weaver.isEmpty());
    QVERIFY(weaver.isIdle());
    QVERIFY(weaver.queueLength() == 0);
    weaver.finish();
  }

  void WeaverLazyThreadCreationTest()
  {
    ThreadWeaver::Weaver weaver;
    QString sequence;
    QCOMPARE (weaver.noOfThreads(), 0);
    AppendCharacterJob a( QChar('a'), &sequence, this);
    weaver.enqueue( & a);
    weaver.finish();
    QVERIFY(a.isFinished());
    QCOMPARE (weaver.noOfThreads(), 1);
  }

    void SimpleJobTest() {
        QString sequence;
        AppendCharacterJob job( QChar( '1' ), &sequence, this );
        ThreadWeaver::Weaver::instance()->enqueue ( &job );
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "1" ) );
    }

    void SimpleJobCollectionTest() {
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

  void EmptyJobCollectionTest() {
    ThreadWeaver::JobCollection collection;
    ThreadWeaver::Weaver::instance()->enqueue ( &collection );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection.isFinished());
  }
  
    void ShortJobSequenceTest() {
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

  void EmptyJobSequenceTest() {
    ThreadWeaver::JobSequence sequence;
    ThreadWeaver::Weaver::instance()->enqueue ( &sequence );
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(sequence.isFinished());
  }
    void QueueAndDequeueSequenceTest() {
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

    void RecursiveQueueAndDequeueSequenceTest() {
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

    void QueueAndDequeueAllSequenceTest() {
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

    void RecursiveQueueAndDequeueAllSequenceTest() {
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
    void MassiveJobSequenceTest() {
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

    void SimpleRecursiveSequencesTest() {
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

    void SequenceOfSequencesTest() {
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

    void QueueAndStopTest() {
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

  void ResourceRestrictionPolicyBasicsTest () {
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

};

QTEST_MAIN ( JobTests )

#include "JobTests.moc"



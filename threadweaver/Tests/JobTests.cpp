#include <cstdlib>

#include <DebuggingAids.h>
#include <ThreadWeaver.h>
#include <JobSequence.h>
#include <JobCollection.h>
#include <QtTest/QtTest>
#include <QMutex>
#include <QMutexLocker>

QMutex s_GlobalMutex;

class AppendCharacterJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    AppendCharacterJob ( QChar c = QChar(), QString* stringref = 0 , QObject* parent = 0 )
        : ThreadWeaver::Job ( parent )
        , m_c ( c )
        , m_stringref ( stringref )
    {
    }

    void setValues ( QChar c, QString* stringref )
    {
        m_c = c;
        m_stringref = stringref;
    }

    void run()
    {
        QMutexLocker locker ( &s_GlobalMutex );
        m_stringref->append( m_c );
//         ThreadWeaver::debug( 0, "AppendCharacterJob: appended %c, result: %s.\n",
//                              m_c.toLatin1(), qPrintable( m_stringref ) );
    }
private:
    QChar m_c;
    QString* m_stringref;
};


class JobTests : public QObject
{
    Q_OBJECT

private slots:
    void testTest() {
        QCOMPARE( 1, 0+1 );
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
        ThreadWeaver::Job::DumpJobDependencies();
        ThreadWeaver::Weaver::instance()->finish();

        QVERIFY( sequence.length() == 3 );
        QVERIFY( sequence.count( 'a' ) == 1 );
        QVERIFY( sequence.count( 'b' ) == 1 );
        QVERIFY( sequence.count( 'c' ) == 1 );
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
        ThreadWeaver::Job::DumpJobDependencies();
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "abc" ) );
    }

    /* This test is not the most efficient, as the mutex locking takes most of
       the execution time. Anyway, it will fail if the jobs are not executed
       in the right order. */
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

    void SimpleRecursiveSequences() {
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

/*    void SequenceOfSequencesTest() {
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
        jobSequence1.addJob ( &jobA );
        jobSequence1.addJob ( &jobB );
        jobSequence1.addJob ( &jobC );
        ThreadWeaver::JobSequence jobSequence2( this );
        jobSequence2.addJob ( &jobD );
        jobSequence2.addJob ( &jobE );
        jobSequence2.addJob ( &jobF );
        ThreadWeaver::JobSequence jobSequence3( this );
        jobSequence3.addJob ( &jobG );
        jobSequence3.addJob ( &jobH );
        jobSequence3.addJob ( &jobI );
        jobSequence3.addJob ( &jobJ );
        // sequence 4 will contain sequences 1, 2, and 3, in that order:
        ThreadWeaver::JobSequence jobSequence4( this );
        jobSequence4.addJob ( &jobSequence1 );
        jobSequence4.addJob ( &jobSequence2 );
        jobSequence4.addJob ( &jobSequence3 );

        // now go already:
        ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence4 );
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "abcdefghij" ) );
    }
*/

};

QTEST_MAIN ( JobTests )

#include "JobTests.moc"


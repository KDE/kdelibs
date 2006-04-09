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
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "abc" ) );
    }

    /* This test is not the most efficient, as the mutex locking takes most of
       the execution time. Anyway, it will fail if the jobs are not executed
       in the right order. */
    void MassiveJobSequenceTest() {
        const int NoOfChars = 2048;
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
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, in );
    }

    void SimpleJobCollectiontest() {
        QString sequence;
        AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
        AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
        AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
        ThreadWeaver::JobCollection jobCollection( this );
        jobCollection.addJob ( &jobA );
        jobCollection.addJob ( &jobB );
        jobCollection.addJob ( &jobC );

        ThreadWeaver::Weaver::instance()->enqueue ( &jobCollection );
        ThreadWeaver::Weaver::instance()->finish();

        QVERIFY( sequence.length() == 3 );
        QVERIFY( sequence.count( 'a' ) == 1 );
        QVERIFY( sequence.count( 'b' ) == 1 );
        QVERIFY( sequence.count( 'c' ) == 1 );
    }

};

QTEST_MAIN ( JobTests )

#include "JobTests.moc"

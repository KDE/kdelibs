#include <DebuggingAids.h>
#include <ThreadWeaver.h>
#include <JobSequence.h>
#include <JobCollection.h>
#include <QtTest/QtTest>

class AppendCharacterJob : public ThreadWeaver::Job
{
public:
    AppendCharacterJob ( QChar c, QString& stringref, QObject* parent )
        : ThreadWeaver::Job ( parent )
        , m_c ( c )
        , m_stringref ( stringref )
    {
    }

    void run()
    {
        m_stringref.append( m_c );
        ThreadWeaver::debug( 0, "AppendCharacterJob: appended %c, result: %s.\n",
                             m_c.toLatin1(), qPrintable( m_stringref ) );
    }
private:
    QChar m_c;
    QString& m_stringref;
};


class TestTest : public QObject
{
    Q_OBJECT

private slots:
    void testTest() {
        QCOMPARE( 1, 0+1 );
    }

    void SimpleJobTest() {
        QString sequence;
        AppendCharacterJob job( QChar( '1' ), sequence, this );
        ThreadWeaver::Weaver::instance()->enqueue ( &job );
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "1" ) );
    }

    void ShortJobSequenceTest() {
        QString sequence;
        AppendCharacterJob jobA ( QChar( 'a' ), sequence, this );
        AppendCharacterJob jobB ( QChar( 'b' ), sequence, this );
        AppendCharacterJob jobC ( QChar( 'c' ), sequence, this );
        ThreadWeaver::JobSequence jobSequence( this );
        jobSequence.append ( &jobA );
        jobSequence.append ( &jobB );
        jobSequence.append ( &jobC );

        ThreadWeaver::Weaver::instance()->enqueue ( &jobSequence );
        ThreadWeaver::Weaver::instance()->finish();
        QCOMPARE ( sequence, QString( "abc" ) );
    }

    void SimpleJobCollectiontest() {
        QString sequence;
        AppendCharacterJob jobA ( QChar( 'a' ), sequence, this );
        AppendCharacterJob jobB ( QChar( 'b' ), sequence, this );
        AppendCharacterJob jobC ( QChar( 'c' ), sequence, this );
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

QTEST_MAIN ( TestTest )

#include "JobTests.moc"

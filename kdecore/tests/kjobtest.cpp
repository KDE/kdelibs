/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "qtest_kde.h"
#include "kjobtest.h"

#include <qtimer.h>
#include <qsignalspy.h>

QTEST_KDEMAIN( KJobTest, NoGUI )


KJobTest::KJobTest()
    : loop( this )
{

}

void KJobTest::testEmitResult_data()
{
    QTest::addColumn<int>("errorCode");
    QTest::addColumn<QString>("errorText");

    QTest::newRow("no error") << (int)KJob::NoError << QString();
    QTest::newRow("error no text") << 2 << QString();
    QTest::newRow("error with text") << 6 << "oops! an error? naaah, really?";
}

void KJobTest::testEmitResult()
{
    TestJob *job = new TestJob;

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotResult( KJob* ) ) );

    QFETCH(int, errorCode);
    QFETCH(QString, errorText);

    job->setError( errorCode );
    job->setErrorText( errorText );

    QSignalSpy destroyed_spy( job, SIGNAL( destroyed( QObject* ) ) );
    job->start();
    loop.exec();


    QCOMPARE( m_lastError, errorCode );
    QCOMPARE( m_lastErrorText, errorText );


    // Verify that the job is not deleted immediately...
    QCOMPARE( destroyed_spy.size(), 0 );
    QTimer::singleShot( 0, &loop, SLOT( quit() ) );
    // ... but when we enter the event loop again.
    loop.exec();
    QCOMPARE( destroyed_spy.size(), 1 );
}

Q_DECLARE_METATYPE(KJob*)

void KJobTest::testProgressTracking()
{
    TestJob *job = new TestJob;

    qRegisterMetaType<KJob*>("KJob*");
    qRegisterMetaType<qulonglong>("qulonglong");

    QSignalSpy processed_spy( job, SIGNAL( processedSize( KJob*, qulonglong ) ) );
    QSignalSpy total_spy( job, SIGNAL( totalSize( KJob*, qulonglong ) ) );
    QSignalSpy percent_spy( job, SIGNAL( percent( KJob*, unsigned long ) ) );


    /* Process a first item. Corresponding signal should be emited.
     * Total size didn't change.
     * Since the total size is unknown the signaled percentage is 100%.
     */
    job->setProcessedSize( 1 );

    QCOMPARE( processed_spy.size(), 1 );
    QCOMPARE( processed_spy.at( 0 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( processed_spy.at( 0 ).at( 1 ).value<qulonglong>(), ( qulonglong )1 );
    QCOMPARE( total_spy.size(), 0 );
    QCOMPARE( percent_spy.size(), 1 );
    QCOMPARE( percent_spy.at( 0 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 0 ).at( 1 ).value<unsigned long>(), ( unsigned long )100 );


    /* Now, we know the total size. It's signaled.
     * The new percentage is signaled too.
     */
    job->setTotalSize( 10 );

    QCOMPARE( processed_spy.size(), 1 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( total_spy.at( 0 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( total_spy.at( 0 ).at( 1 ).value<qulonglong>(), ( qulonglong )10 );
    QCOMPARE( percent_spy.size(), 2 );
    QCOMPARE( percent_spy.at( 1 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 1 ).at( 1 ).value<unsigned long>(), ( unsigned long )10 );


    /* We announce a new percentage by hand.
     * Total, and processed didn't change, so no signal is emitted for them.
     */
    job->setPercent( 15 );

    QCOMPARE( processed_spy.size(), 1 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( percent_spy.size(), 3 );
    QCOMPARE( percent_spy.at( 2 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 2 ).at( 1 ).value<unsigned long>(), ( unsigned long )15 );


    /* We make some progress.
     * Processed size and percent are signaled.
     */
    job->setProcessedSize( 3 );

    QCOMPARE( processed_spy.size(), 2 );
    QCOMPARE( processed_spy.at( 1 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( processed_spy.at( 1 ).at( 1 ).value<qulonglong>(), ( qulonglong )3 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( percent_spy.size(), 4 );
    QCOMPARE( percent_spy.at( 3 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 3 ).at( 1 ).value<unsigned long>(), ( unsigned long )30 );


    /* We set a new total size, but equals to the previous one.
     * No signal is emitted.
     */
    job->setTotalSize( 10 );

    QCOMPARE( processed_spy.size(), 2 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( percent_spy.size(), 4 );


    /* We 'lost' the previous work done.
     * Signals both percentage and new processed size.
     */
    job->setProcessedSize( 0 );

    QCOMPARE( processed_spy.size(), 3 );
    QCOMPARE( processed_spy.at( 2 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( processed_spy.at( 2 ).at( 1 ).value<qulonglong>(), ( qulonglong )0 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( percent_spy.size(), 5 );
    QCOMPARE( percent_spy.at( 4 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 4 ).at( 1 ).value<unsigned long>(), ( unsigned long )0 );

    /* We process more than the total size!?
     * Signals both percentage and new processed size.
     * Percentage is 150%
     *
     * Might sounds weird, but verify that this case is handled gracefully.
     */
    job->setProcessedSize( 15 );

    QCOMPARE( processed_spy.size(), 4 );
    QCOMPARE( processed_spy.at( 3 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( processed_spy.at( 3 ).at( 1 ).value<qulonglong>(), ( qulonglong )15 );
    QCOMPARE( total_spy.size(), 1 );
    QCOMPARE( percent_spy.size(), 6 );
    QCOMPARE( percent_spy.at( 5 ).at( 0 ).value<KJob*>(), job );
    QCOMPARE( percent_spy.at( 5 ).at( 1 ).value<unsigned long>(), ( unsigned long )150 );

    delete job;
}

void KJobTest::testExec_data()
{
    QTest::addColumn<int>("errorCode");
    QTest::addColumn<QString>("errorText");

    QTest::newRow("no error") << (int)KJob::NoError << QString();
    QTest::newRow("error no text") << 2 << QString();
    QTest::newRow("error with text") << 6 << "oops! an error? naaah, really?";
}

void KJobTest::testExec()
{
    TestJob *job = new TestJob;

    QFETCH(int, errorCode);
    QFETCH(QString, errorText);

    job->setError( errorCode );
    job->setErrorText( errorText );

    QSignalSpy destroyed_spy( job, SIGNAL( destroyed( QObject* ) ) );

    bool status = job->exec();

    QCOMPARE( status, ( errorCode == KJob::NoError ) );
    QCOMPARE( job->error(),  errorCode );
    QCOMPARE( job->errorText(),  errorText );

    // Verify that the job is not deleted immediately...
    QCOMPARE( destroyed_spy.size(), 0 );
    QTimer::singleShot( 0, &loop, SLOT( quit() ) );
    // ... but when we enter the event loop again.
    loop.exec();
    QCOMPARE( destroyed_spy.size(), 1 );
}

void KJobTest::testKill_data()
{
    QTest::addColumn<int>("killVerbosity");
    QTest::addColumn<int>("errorCode");
    QTest::addColumn<QString>("errorText");

    QTest::newRow("killed with result") << (int)KJob::EmitResult
                                        << (int)KJob::KilledJobError
                                        << QString();
    QTest::newRow("killed quietly") << (int)KJob::Quietly
                                    << (int)KJob::NoError
                                    << QString();
}

void KJobTest::testKill()
{
    TestJob *job = new TestJob;

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotResult( KJob* ) ) );

    m_lastError = KJob::NoError;
    m_lastErrorText = QString();

    QFETCH(int, killVerbosity);
    QFETCH(int, errorCode);
    QFETCH(QString, errorText);

    QSignalSpy destroyed_spy( job, SIGNAL( destroyed( QObject* ) ) );

    job->kill( ( KJob::KillVerbosity )killVerbosity );
    loop.processEvents( QEventLoop::AllEvents, 2000 );

    QCOMPARE( m_lastError, errorCode );
    QCOMPARE( m_lastErrorText, errorText );

    QCOMPARE( job->error(),  errorCode );
    QCOMPARE( job->errorText(),  errorText );

    // Verify that the job is not deleted immediately...
    QCOMPARE( destroyed_spy.size(), 0 );
    QTimer::singleShot( 0, &loop, SLOT( quit() ) );
    // ... but when we enter the event loop again.
    loop.exec();
    QCOMPARE( destroyed_spy.size(), 1 );
}

void KJobTest::slotResult( KJob *job )
{
    if ( job->error() )
    {
        m_lastError = job->error();
        m_lastErrorText = job->errorText();
    }
    else
    {
        m_lastError = KJob::NoError;
        m_lastErrorText = QString();
    }

    loop.quit();
}

TestJob::TestJob() : KJob()
{

}

TestJob::~TestJob()
{

}

void TestJob::start()
{
    QTimer::singleShot( 0, this, SLOT( doEmit() ) );
}

bool TestJob::doKill()
{
    return true;
}

void TestJob::setError( int errorCode )
{
    KJob::setError( errorCode );
}

void TestJob::setErrorText( const QString &errorText )
{
    KJob::setErrorText( errorText );
}

void TestJob::setProcessedSize( qulonglong size )
{
    KJob::setProcessedSize( size );
}

void TestJob::setTotalSize( qulonglong size )
{
    KJob::setTotalSize( size );
}

void TestJob::setPercent( unsigned long percentage )
{
    KJob::setPercent( percentage );
}

void TestJob::doEmit()
{
    emitResult();
}

#include "kjobtest.moc"

#include "QueueTests.h"

#include <QChar>
#include <QString>
#include <QObject>
#include <QThread>
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

void QueueTests::DeleteSelfLaterTest()
{
  SecondThreadThatQueues thread;
  thread.start();
  thread.wait();
  QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
}

QTEST_MAIN ( QueueTests )

#include "QueueTests.moc"

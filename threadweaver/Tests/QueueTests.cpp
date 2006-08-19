// This file contains a testsuite for the queueing behaviour in ThreadWeaver. 

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

class LowPriorityAppendCharacterJob : public AppendCharacterJob
{
  Q_OBJECT

public:
  LowPriorityAppendCharacterJob (QChar c = QChar(), QString* stringref = 0,
				 QObject* parent = 0)
    : AppendCharacterJob (c, stringref, parent)
  {}

  int priority() const
  {
    return -1;
  }

};

class HighPriorityAppendCharacterJob : public AppendCharacterJob
{
  Q_OBJECT

public:
  HighPriorityAppendCharacterJob (QChar c = QChar(), QString* stringref = 0,
				 QObject* parent = 0)
    : AppendCharacterJob (c, stringref, parent)
  {}

  int priority() const
  {
    return 1;
  }

};

class SecondThreadThatQueues : public QThread
{
  Q_OBJECT

public:
  SecondThreadThatQueues()
    : QThread()
  {
  }

protected:
  void run ()
  {
    QString sequence;
    AppendCharacterJob a( 'a', &sequence );
    
    ThreadWeaver::Weaver::instance()->enqueue ( &a );
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE( sequence, QString("a" ) );
  }
};

class QueueTests : public QObject
{
  Q_OBJECT

private slots:
  
  void initTestCase ()
  {
    ThreadWeaver::setDebugLevel ( true,  1 );
  }

  void SimpleQueuePrioritiesTest() {
    ThreadWeaver::Weaver weaver ( 0, 1 ); // just one thread
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

  void WeaverInitializationTest()
  { // this one mostly tests the sanity of the startup behaviour
    ThreadWeaver::Weaver weaver;
    QCOMPARE (weaver.numberOfThreads(), 0);
    QVERIFY (weaver.isEmpty());
    QVERIFY(weaver.isIdle());
    QVERIFY(weaver.queueLength() == 0);
    weaver.finish();
  }

  void DeleteSelfLaterTest()
  {
    SecondThreadThatQueues thread;
    thread.start();
    thread.wait();
    QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
  }

};

QTEST_MAIN ( QueueTests )

#include "QueueTests.moc"

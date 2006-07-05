// This file contains a testsuite for the queueing behaviour in ThreadWeaver. 

#include <QChar>
#include <QString>
#include <QObject>
#include <QtTest/QtTest>

#include "AppendCharacterJob.h" 

#include <Job.h>
#include <ThreadWeaver.h>

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

};

QTEST_MAIN ( QueueTests )

#include "QueueTests.moc"

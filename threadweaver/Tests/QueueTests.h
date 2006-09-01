// This file contains a testsuite for the queueing behaviour in ThreadWeaver.

#ifndef QUEUETESTS_H
#define QUEUETESTS_H

#include <QThread>

#include "AppendCharacterJob.h"

class LowPriorityAppendCharacterJob : public AppendCharacterJob
{
  Q_OBJECT

public:
  LowPriorityAppendCharacterJob (QChar c = QChar(), QString* stringref = 0,
                                 QObject* parent = 0);

  int priority() const;
};

class HighPriorityAppendCharacterJob : public AppendCharacterJob
{
  Q_OBJECT

public:
  HighPriorityAppendCharacterJob (QChar c = QChar(), QString* stringref = 0,
                                 QObject* parent = 0);

  int priority() const;
};

class SecondThreadThatQueues : public QThread
{
  Q_OBJECT

public:
  SecondThreadThatQueues();

protected:
  void run ();
};

class QueueTests : public QObject
{
  Q_OBJECT

private slots:

  void initTestCase ();

  void SimpleQueuePrioritiesTest();
  void WeaverInitializationTest();
  void DeleteSelfLaterTest();
};

#endif

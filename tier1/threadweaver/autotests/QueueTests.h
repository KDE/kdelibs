// This file contains a testsuite for the queueing behaviour in ThreadWeaver.

#ifndef QUEUETESTS_H
#define QUEUETESTS_H

#include <QtCore/QThread>

#include "AppendCharacterJob.h"

class LowPriorityAppendCharacterJob : public AppendCharacterJob
{
public:
    LowPriorityAppendCharacterJob(QChar character = QChar(), QString* stringref = 0);

    int priority() const;
};

class HighPriorityAppendCharacterJob : public AppendCharacterJob
{
public:
    HighPriorityAppendCharacterJob(QChar character = QChar(), QString* stringref = 0);

    int priority() const;
};

namespace ThreadWeaver {
    class Job;
    class JobCollection;
    class QObjectDecorator;
}

using ThreadWeaver::Job;

class SecondThreadThatQueues : public QThread
{
    Q_OBJECT

public:
    SecondThreadThatQueues();

protected:
    void run();
};

class QueueTests : public QObject
{
    Q_OBJECT

public:
    explicit QueueTests( QObject* parent = 0 );

public Q_SLOTS:
    // this slot (which is not a test) is part of
    // DeleteDoneJobsFromSequenceTest
    void deleteJob(ThreadWeaver::JobPointer);
    // this slot is part of DeleteCollectionOnDoneTest
    void deleteCollection(ThreadWeaver::JobPointer);

private:
    // this is also part of DeleteDoneJobsFromSequenceTest
    ThreadWeaver::QObjectDecorator* autoDeleteJob;
    // this is part of DeleteCollectionOnDoneTest
    ThreadWeaver::QObjectDecorator* autoDeleteCollection;

private Q_SLOTS:

    void initTestCase ();

    void SimpleQueuePrioritiesTest();
    void WeaverInitializationTest();
    void QueueFromSecondThreadTest();
    void DeleteDoneJobsFromSequenceTest();
    void DeleteCollectionOnDoneTest();
};

#endif

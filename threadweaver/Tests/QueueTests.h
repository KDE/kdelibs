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

namespace ThreadWeaver {
    class Job;
    class JobCollection;
};

using ThreadWeaver::Job;

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

public:
    explicit QueueTests( QObject* parent = 0 );

public slots:
    // this slot (which is not a test) is part of
    // DeleteDoneJobsFromSequenceTest
    void deleteJob( Job* );
    // this slot is part of DeleteCollectionOnDoneTest
    void deleteCollection( Job* );

private:
    // this is also part of DeleteDoneJobsFromSequenceTest
    AppendCharacterJob* autoDeleteJob;
    // this is part of DeleteCollectionOnDoneTest
    ThreadWeaver::JobCollection* autoDeleteCollection;

private slots:

    void initTestCase ();

    void SimpleQueuePrioritiesTest();
    void WeaverInitializationTest();
    void QueueFromSecondThreadTest();
    void DeleteDoneJobsFromSequenceTest();
    void DeleteCollectionOnDoneTest();
};

#endif

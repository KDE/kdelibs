#ifndef JOBTESTS_H
#define JOBTESTS_H

#include <QObject>

namespace ThreadWeaver{
    class Job;
}

using ThreadWeaver::Job;

class JobTests : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase ();

    // call finish() before leave a test to make sure the queue is empty

    void WeaverLazyThreadCreationTest();

    void SimpleJobTest();

    void SimpleJobCollectionTest();

    void EmptyJobCollectionTest();

    void ShortJobSequenceTest();

    void EmptyJobSequenceTest();

    void QueueAndDequeueSequenceTest();

    void RecursiveQueueAndDequeueSequenceTest();

    void QueueAndDequeueAllSequenceTest();

    void RecursiveQueueAndDequeueAllSequenceTest();

//     This test is not the most efficient, as the mutex locking takes most of
//     the execution time. Anyway, it will fail if the jobs are not executed
//     in the right order, and the order is randomized.
    void MassiveJobSequenceTest();

    void SimpleRecursiveSequencesTest();

    void SequenceOfSequencesTest();

    void QueueAndStopTest();

    void ResourceRestrictionPolicyBasicsTest ();

    void JobSignalsAreEmittedAsynchronouslyTest();

public slots: // slots used during tests that are not test cases
    void jobStarted( Job* );
    void jobDone( Job* );
};

#endif

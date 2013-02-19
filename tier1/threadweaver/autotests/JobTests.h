#ifndef JOBTESTS_H
#define JOBTESTS_H

#include <QtCore/QObject>

namespace ThreadWeaver{
    class Job;
}

using ThreadWeaver::Job;

class JobTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase ();
    void WeaverLazyThreadCreationTest();
    void SimpleJobTest();
    void SimpleJobCollectionTest();
    void EmptyJobCollectionTest();
    void CollectionQueueingTest();
    void ShortJobSequenceTest();
    void EmptyJobSequenceTest();
    void IncompleteCollectionTest();
    void EmitStartedOnFirstElementTest();
    void CollectionDependenciesTest();
    void QueueAndDequeueSequenceTest();
    void RecursiveSequenceTest();
    void RecursiveQueueAndDequeueSequenceTest();
    void QueueAndDequeueAllSequenceTest();
    void RecursiveQueueAndDequeueAllSequenceTest();
    void MassiveJobSequenceTest();
    void SimpleRecursiveSequencesTest();
    void SequenceOfSequencesTest();
    void QueueAndStopTest();
    void ResourceRestrictionPolicyBasicsTest ();
    void JobSignalsAreEmittedAsynchronouslyTest();
    void DequeueSuspendedSequence();

public Q_SLOTS: // slots used during tests that are not test cases
    void jobStarted( ThreadWeaver::Job* );
    void jobDone( ThreadWeaver::Job* );
};

#endif

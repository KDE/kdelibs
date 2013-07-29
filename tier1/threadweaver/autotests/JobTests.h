#ifndef JOBTESTS_H
#define JOBTESTS_H

#include <QtCore/QObject>
#include <Weaver/JobPointer.h>

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
    void QueueAndDequeueCollectionTest();
    void QueueAndDequeueSequenceTest();
    void RecursiveSequenceTest();
    void RecursiveQueueAndDequeueCollectionTest();
    void RecursiveQueueAndDequeueSequenceTest();
    void QueueAndDequeueAllCollectionTest();
    void QueueAndDequeueAllSequenceTest();
    void RecursiveQueueAndDequeueAllCollectionTest();
    void RecursiveQueueAndDequeueAllSequenceTest();
    void MassiveJobSequenceTest();
    void SimpleRecursiveSequencesTest();
    void SequenceOfSequencesTest();
    void QueueAndStopTest();
    void ResourceRestrictionPolicyBasicsTest ();
    void JobSignalsAreEmittedAsynchronouslyTest();
    void JobSignalsDeliveryTest();
    void JobPointerExecutionTest();
    void DequeueSuspendedSequenceTest(); //FIXME ...test?

public Q_SLOTS: // slots used during tests that are not test cases
    void jobStarted(ThreadWeaver::JobPointer);
    void jobDone(ThreadWeaver::JobPointer);

    void deliveryTestJobDone(ThreadWeaver::JobPointer);
};

#endif

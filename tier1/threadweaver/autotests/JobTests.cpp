#include "JobTests.h"

#include <cstdlib>

#include <QtCore/QMutex>
#include <QtTest/QtTest>
#include <QSignalSpy>

#include <JobSequence.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>
#include <JobCollection.h>
#include <ResourceRestrictionPolicy.h>
#include <DependencyPolicy.h>

#include "AppendCharacterJob.h"
#include "AppendCharacterAndVerifyJob.h"

QMutex s_GlobalMutex;

//Ensure that after the object is created, the weaver is idle and resumed.
//Upon desruction, ensure the weaver is idle and suspended.
class WaitForIdleAndFinished {
public:
    explicit WaitForIdleAndFinished(ThreadWeaver::Weaver* weaver)
        : weaver_(weaver)
    {
        Q_ASSERT(weaver);
        weaver_->finish();
        Q_ASSERT(weaver_->isIdle());
        weaver_->resume();
    }

    ~WaitForIdleAndFinished() {
        weaver_->resume();
        weaver_->dequeue();
        weaver_->finish();
        weaver_->suspend();
        Q_ASSERT(weaver_->isIdle());
    }
private:
    ThreadWeaver::Weaver* weaver_;
};

void JobTests::initTestCase ()
{
    ThreadWeaver::setDebugLevel(true, 1);
}

// call finish() before leave a test to make sure the queue is empty

void JobTests::WeaverLazyThreadCreationTest()
{
    ThreadWeaver::Weaver weaver;
    QString sequence;
    ThreadWeaver::JobPointer a(new AppendCharacterJob(QChar('a'), &sequence));

    WaitForIdleAndFinished w(&weaver);
    Q_ASSERT(weaver.isIdle());
    QCOMPARE (weaver.currentNumberOfThreads(), 0);
    weaver.enqueue(a);
    weaver.finish();
    QVERIFY(a->isFinished());
    QCOMPARE (weaver.currentNumberOfThreads(), 1);
    Q_ASSERT(weaver.isIdle());
}

void JobTests::SimpleJobTest() {
    QString sequence;
    ThreadWeaver::JobPointer job(new AppendCharacterJob( QChar('1'), &sequence));

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue(job);
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE ( sequence, QString( "1" ) );
}

void JobTests::SimpleJobCollectionTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> jobCollection(new ThreadWeaver::JobCollection());
    jobCollection->addJob(jobA);
    jobCollection->addJob(jobB);
    jobCollection->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue(jobCollection);
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();

    QVERIFY( sequence.length() == 3 );
    QVERIFY( sequence.count( 'a' ) == 1 );
    QVERIFY( sequence.count( 'b' ) == 1 );
    QVERIFY( sequence.count( 'c' ) == 1 );
}

void JobTests::EmptyJobCollectionTest() {
    QSharedPointer<ThreadWeaver::JobCollection> collection(new ThreadWeaver::JobCollection());

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    Q_ASSERT(ThreadWeaver::Weaver::instance()->isIdle());
    ThreadWeaver::Weaver::instance()->enqueue(collection);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection->isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::CollectionQueueingTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> jobCollection(new ThreadWeaver::JobCollection());
    jobCollection->addJob(jobA);
    jobCollection->addJob(jobB);
    jobCollection->addJob(jobC);

    ThreadWeaver::Weaver weaver;
    WaitForIdleAndFinished w(&weaver);
    weaver.suspend();
    weaver.enqueue(jobCollection);
    QCOMPARE(weaver.queueLength(), 1); //collection queues itself, and it's elements upon execution of self
    weaver.resume();
    weaver.finish();
    QCOMPARE(sequence.length(), 3);
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::ShortJobSequenceTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence(new ThreadWeaver::JobSequence());
    jobSequence->addJob(jobA);
    jobSequence->addJob(jobB);
    jobSequence->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueue(jobSequence);
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QLatin1String("abc"));
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::EmptyJobSequenceTest() {
    QSharedPointer<ThreadWeaver::JobSequence> sequence(new ThreadWeaver::JobSequence());

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    Q_ASSERT(ThreadWeaver::Weaver::instance()->isIdle());
    QSignalSpy doneSignalSpy(sequence.data(), SIGNAL(done(ThreadWeaver::JobPointer)));
    QCOMPARE(doneSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue(sequence);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(sequence->isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE(doneSignalSpy.count(), 1);
}

/** This test verifies that the done signal for a collection is only sent after all element of the collection have completed. */
void JobTests::IncompleteCollectionTest()
{
    QString result;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &result));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &result)); //jobB does not get added to the sequence and queued
    QSharedPointer<ThreadWeaver::JobCollection> collection(new ThreadWeaver::JobCollection());
    collection->addJob(jobA);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::DependencyPolicy::instance().addDependency(jobA.data(), jobB.data());
    QSignalSpy collectionDoneSignalSpy(collection.data(), SIGNAL(done(ThreadWeaver::JobPointer)));
    QSignalSpy jobADoneSignalSpy(jobA.data(), SIGNAL(done(ThreadWeaver::JobPointer)));
    QCOMPARE(collectionDoneSignalSpy.count(), 0);
    QCOMPARE(jobADoneSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue(collection);
    ThreadWeaver::Weaver::instance()->resume();
    QCoreApplication::processEvents();
    QCOMPARE(collectionDoneSignalSpy.count(), 0);
    QCOMPARE(jobADoneSignalSpy.count(), 0);
    ThreadWeaver::DependencyPolicy::instance().removeDependency(jobA.data(), jobB.data());
    ThreadWeaver::Weaver::instance()->finish();
    QCoreApplication::processEvents();
    QVERIFY(collection->isFinished());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE(collectionDoneSignalSpy.count(), 1);
    QCOMPARE(jobADoneSignalSpy.count(), 1);
}

/** This test verifies that started() is emitted for a collection at the time the first of any elements of the collection gets
 * executed. */
void JobTests::EmitStartedOnFirstElementTest()
{
    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    QString result;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &result));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &result));
    QSharedPointer<ThreadWeaver::JobCollection> collection (new ThreadWeaver::JobCollection());
    collection->addJob(jobA);
    collection->addJob(jobB);
    ThreadWeaver::Weaver::instance()->enqueue(collection);
    QSignalSpy collectionStartedSignalSpy(collection.data(), SIGNAL(started(ThreadWeaver::JobPointer)));
    QSignalSpy collectionDoneSignalSpy(collection.data(), SIGNAL(done(ThreadWeaver::JobPointer)));
    ThreadWeaver::Weaver::instance()->resume();
    QCoreApplication::processEvents();
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(collection->isFinished());
    QCOMPARE(collectionStartedSignalSpy.count(), 1);
    QCOMPARE(collectionDoneSignalSpy.count(), 1);
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

/* This test verifies that all elements of a collection are only executed after all dependencies for the collection
 * itself have been resolved.
 * Previous tests have already verified that collections without dependencies get executed right away. */
void JobTests::CollectionDependenciesTest()
{
    QString result;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &result));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &result));
    QSharedPointer<AppendCharacterJob> jobC(new AppendCharacterJob(QChar('c'), &result));
    QSharedPointer<ThreadWeaver::JobCollection> collection(new ThreadWeaver::JobCollection());
    QSignalSpy collectionStartedSignalSpy(collection.data(), SIGNAL(started(ThreadWeaver::JobPointer)));
    collection->addJob(jobA);
    collection->addJob(jobB);
    ThreadWeaver::DependencyPolicy::instance().addDependency(collection.data(), jobC.data());

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(collection);
    ThreadWeaver::Weaver::instance()->resume();
    QCoreApplication::processEvents();
    QTest::qWait(100);
    QCOMPARE(collectionStartedSignalSpy.count(), 0);
    ThreadWeaver::Weaver::instance()->enqueue(jobC);
    QCoreApplication::processEvents();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(collectionStartedSignalSpy.count(), 1);
    QVERIFY(collection->isFinished());
    QVERIFY(result.startsWith(jobC->character()));
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
}

void JobTests::QueueAndDequeueCollectionTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> collection(new ThreadWeaver::JobCollection());
    collection->addJob(jobA);
    collection->addJob(jobB);
    collection->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue(collection);
    ThreadWeaver::Weaver::instance()->dequeue(collection);
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}


void JobTests::QueueAndDequeueSequenceTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence(new ThreadWeaver::JobSequence());
    jobSequence->addJob(jobA);
    jobSequence->addJob(jobB);
    jobSequence->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue(jobSequence);
    ThreadWeaver::Weaver::instance()->dequeue(jobSequence);
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}

void JobTests::RecursiveSequenceTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    ThreadWeaver::JobPointer jobD(new AppendCharacterJob(QChar('d'), &sequence));
    ThreadWeaver::JobPointer jobE(new AppendCharacterJob(QChar('e'), &sequence));
    ThreadWeaver::JobPointer jobF(new AppendCharacterJob(QChar('f'), &sequence));
    ThreadWeaver::JobPointer jobG(new AppendCharacterJob(QChar('g'), &sequence));
    ThreadWeaver::JobPointer jobH(new AppendCharacterJob(QChar('h'), &sequence));
    ThreadWeaver::JobPointer jobI(new AppendCharacterJob(QChar('i'), &sequence));
    ThreadWeaver::JobPointer jobJ(new AppendCharacterJob(QChar('j'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence1(new ThreadWeaver::JobSequence());
    jobSequence1->setObjectName( "Sequ_1" );
    jobSequence1->addJob(jobA);
    jobSequence1->addJob(jobB);
    jobSequence1->addJob(jobC);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence2(new ThreadWeaver::JobSequence());
    jobSequence2->setObjectName( "Sequ_2" );
    jobSequence2->addJob(jobD);
    jobSequence2->addJob(jobE);
    jobSequence2->addJob(jobF);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence3(new ThreadWeaver::JobSequence());
    jobSequence3->setObjectName( "Sequ_3" );
    jobSequence3->addJob(jobG);
    jobSequence3->addJob(jobH);
    jobSequence3->addJob(jobI);
    jobSequence3->addJob(jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence4(new ThreadWeaver::JobSequence());
    jobSequence4->setObjectName( "Sequ_4" );
    jobSequence4->addJob(jobSequence1);
    jobSequence4->addJob(jobSequence2);
    jobSequence4->addJob(jobSequence3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(jobSequence4);
//    ThreadWeaver::DependencyPolicy::instance().dumpJobDependencies();
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QLatin1String("abcdefghij"));
}

void JobTests::RecursiveQueueAndDequeueCollectionTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    ThreadWeaver::JobPointer jobD(new AppendCharacterJob(QChar('d'), &sequence));
    ThreadWeaver::JobPointer jobE(new AppendCharacterJob(QChar('e'), &sequence));
    ThreadWeaver::JobPointer jobF(new AppendCharacterJob(QChar('f'), &sequence));
    ThreadWeaver::JobPointer jobG(new AppendCharacterJob(QChar('g'), &sequence));
    ThreadWeaver::JobPointer jobH(new AppendCharacterJob(QChar('h'), &sequence));
    ThreadWeaver::JobPointer jobI(new AppendCharacterJob(QChar('i'), &sequence));
    ThreadWeaver::JobPointer jobJ(new AppendCharacterJob(QChar('j'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> collection1(new ThreadWeaver::JobCollection());
    collection1->setObjectName("Coll_1");
    collection1->addJob(jobA);
    collection1->addJob(jobB);
    collection1->addJob(jobC);
    QSharedPointer<ThreadWeaver::JobCollection> collection2(new ThreadWeaver::JobCollection());
    collection2->setObjectName( "Coll_2" );
    collection2->addJob(jobD);
    collection2->addJob(jobE);
    collection2->addJob(jobF);
    QSharedPointer<ThreadWeaver::JobCollection> collection3(new ThreadWeaver::JobCollection());
    collection3->setObjectName( "Coll_3" );
    collection3->addJob(jobG);
    collection3->addJob(jobH);
    collection3->addJob(jobI);
    collection3->addJob(jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    QSharedPointer<ThreadWeaver::JobCollection> collection4(new ThreadWeaver::JobCollection());
    collection4->setObjectName( "Coll_4" );
    collection4->addJob(collection1);
    collection4->addJob(collection2);
    collection4->addJob(collection3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(collection4);
    ThreadWeaver::Weaver::instance()->dequeue(collection4);
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
}

void JobTests::RecursiveQueueAndDequeueSequenceTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    ThreadWeaver::JobPointer jobD(new AppendCharacterJob(QChar('d'), &sequence));
    ThreadWeaver::JobPointer jobE(new AppendCharacterJob(QChar('e'), &sequence));
    ThreadWeaver::JobPointer jobF(new AppendCharacterJob(QChar('f'), &sequence));
    ThreadWeaver::JobPointer jobG(new AppendCharacterJob(QChar('g'), &sequence));
    ThreadWeaver::JobPointer jobH(new AppendCharacterJob(QChar('h'), &sequence));
    ThreadWeaver::JobPointer jobI(new AppendCharacterJob(QChar('i'), &sequence));
    ThreadWeaver::JobPointer jobJ(new AppendCharacterJob(QChar('j'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence1(new ThreadWeaver::JobSequence());
    jobSequence1->setObjectName("Sequ_1");
    jobSequence1->addJob(jobA);
    jobSequence1->addJob(jobB);
    jobSequence1->addJob(jobC);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence2(new ThreadWeaver::JobSequence());
    jobSequence2->setObjectName("Sequ_2");
    jobSequence2->addJob(jobD);
    jobSequence2->addJob(jobE);
    jobSequence2->addJob(jobF);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence3(new ThreadWeaver::JobSequence());
    jobSequence3->setObjectName( "Sequ_3" );
    jobSequence3->addJob(jobG);
    jobSequence3->addJob(jobH);
    jobSequence3->addJob(jobI);
    jobSequence3->addJob(jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence4(new ThreadWeaver::JobSequence());
    jobSequence4->setObjectName( "Sequ_4" );
    jobSequence4->addJob(jobSequence1);
    jobSequence4->addJob(jobSequence2);
    jobSequence4->addJob(jobSequence3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(jobSequence4);
    ThreadWeaver::Weaver::instance()->dequeue(jobSequence4);
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
}

void JobTests::QueueAndDequeueAllCollectionTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> collection(new ThreadWeaver::JobCollection());
    collection->addJob(jobA);
    collection->addJob(jobB);
    collection->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->enqueue(collection);
    //collection cannot have been started, so only one job is queued at the moment:
    QCOMPARE(ThreadWeaver::Weaver::instance()->queueLength(), 1);
    ThreadWeaver::Weaver::instance()->dequeue();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}

void JobTests::QueueAndDequeueAllSequenceTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence(new ThreadWeaver::JobSequence());
    jobSequence->addJob(jobA);
    jobSequence->addJob(jobB);
    jobSequence->addJob(jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();

    ThreadWeaver::Weaver::instance()->enqueue(jobSequence);
    ThreadWeaver::Weaver::instance()->dequeue();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
}

void JobTests::RecursiveQueueAndDequeueAllCollectionTest()
{
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    ThreadWeaver::JobPointer jobD(new AppendCharacterJob(QChar('d'), &sequence));
    ThreadWeaver::JobPointer jobE(new AppendCharacterJob(QChar('e'), &sequence));
    ThreadWeaver::JobPointer jobF(new AppendCharacterJob(QChar('f'), &sequence));
    ThreadWeaver::JobPointer jobG(new AppendCharacterJob(QChar('g'), &sequence));
    ThreadWeaver::JobPointer jobH(new AppendCharacterJob(QChar('h'), &sequence));
    ThreadWeaver::JobPointer jobI(new AppendCharacterJob(QChar('i'), &sequence));
    ThreadWeaver::JobPointer jobJ(new AppendCharacterJob(QChar('j'), &sequence));
    QSharedPointer<ThreadWeaver::JobCollection> collection1(new ThreadWeaver::JobCollection());
    collection1->setObjectName("Coll_1");
    collection1->addJob(jobA);
    collection1->addJob(jobB);
    collection1->addJob(jobC);
    QSharedPointer<ThreadWeaver::JobCollection> collection2(new ThreadWeaver::JobCollection());
    collection2->setObjectName("Coll_2");
    collection2->addJob(jobD);
    collection2->addJob(jobE);
    collection2->addJob(jobF);
    QSharedPointer<ThreadWeaver::JobCollection> collection3(new ThreadWeaver::JobCollection());
    collection3->setObjectName("Coll_3");
    collection3->addJob(jobG);
    collection3->addJob(jobH);
    collection3->addJob(jobI);
    collection3->addJob(jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    QSharedPointer<ThreadWeaver::JobCollection> collection4(new ThreadWeaver::JobCollection());
    collection4->setObjectName("Coll_4");
    collection4->addJob(collection1);
    collection4->addJob(collection2);
    collection4->addJob(collection3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(collection4);
    ThreadWeaver::Weaver::instance()->dequeue ();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();

}

void JobTests::RecursiveQueueAndDequeueAllSequenceTest() {
    QString sequence;
    ThreadWeaver::JobPointer jobA(new AppendCharacterJob(QChar('a'), &sequence));
    ThreadWeaver::JobPointer jobB(new AppendCharacterJob(QChar('b'), &sequence));
    ThreadWeaver::JobPointer jobC(new AppendCharacterJob(QChar('c'), &sequence));
    ThreadWeaver::JobPointer jobD(new AppendCharacterJob(QChar('d'), &sequence));
    ThreadWeaver::JobPointer jobE(new AppendCharacterJob(QChar('e'), &sequence));
    ThreadWeaver::JobPointer jobF(new AppendCharacterJob(QChar('f'), &sequence));
    ThreadWeaver::JobPointer jobG(new AppendCharacterJob(QChar('g'), &sequence));
    ThreadWeaver::JobPointer jobH(new AppendCharacterJob(QChar('h'), &sequence));
    ThreadWeaver::JobPointer jobI(new AppendCharacterJob(QChar('i'), &sequence));
    ThreadWeaver::JobPointer jobJ(new AppendCharacterJob(QChar('j'), &sequence));
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence1(new ThreadWeaver::JobSequence());
    jobSequence1->setObjectName("Sequ_1");
    jobSequence1->addJob(jobA);
    jobSequence1->addJob(jobB);
    jobSequence1->addJob(jobC);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence2(new ThreadWeaver::JobSequence());
    jobSequence2->setObjectName("Sequ_2");
    jobSequence2->addJob(jobD);
    jobSequence2->addJob(jobE);
    jobSequence2->addJob(jobF);
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence3(new ThreadWeaver::JobSequence());
    jobSequence3->setObjectName("Sequ_3");
    jobSequence3->addJob(jobG);
    jobSequence3->addJob(jobH);
    jobSequence3->addJob(jobI);
    jobSequence3->addJob(jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    QSharedPointer<ThreadWeaver::JobSequence> jobSequence4(new ThreadWeaver::JobSequence());
    jobSequence4->setObjectName("Sequ_4");
    jobSequence4->addJob(jobSequence1);
    jobSequence4->addJob(jobSequence2);
    jobSequence4->addJob(jobSequence3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->suspend();
    ThreadWeaver::Weaver::instance()->enqueue(jobSequence4);
    ThreadWeaver::Weaver::instance()->dequeue ();
    QVERIFY(ThreadWeaver::Weaver::instance()->isEmpty());
    ThreadWeaver::Weaver::instance()->resume();
    ThreadWeaver::Weaver::instance()->finish();
}

//     This test is not the most efficient, as the mutex locking takes most of
//     the execution time. Anyway, it will fail if the jobs are not executed
//     in the right order, and the order is randomized.
void JobTests::MassiveJobSequenceTest() {
    const int NoOfChars = 1024;
    const char* Alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int SizeOfAlphabet = strlen( Alphabet );
    AppendCharacterAndVerifyJob jobs[NoOfChars];
    ThreadWeaver::JobSequence jobSequence(this);
    QString sequence;
    QString in;

    srand(1);
    in.reserve(NoOfChars);
    sequence.reserve (NoOfChars);
    for ( int i = 0; i<NoOfChars; ++i ) {
        const int position = static_cast<int> ( SizeOfAlphabet * ( ( 1.0 * rand() ) / RAND_MAX ) );
        Q_ASSERT ( 0 <= position && position < SizeOfAlphabet );
        QChar c( Alphabet[position] );
        in.append ( c );
    }

    for ( int i = 0; i<NoOfChars; ++i ) {
        jobs[i].setValues( in.at(i), &sequence, in );
        jobSequence.addNakedJob(&jobs[i]);
    }

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    ThreadWeaver::Weaver::instance()->enqueueJob(&jobSequence);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY(ThreadWeaver::Weaver::instance()->isIdle());
    QCOMPARE(sequence,in);
}

void JobTests::SimpleRecursiveSequencesTest() {
    QString sequence;
    AppendCharacterJob jobB(QChar('b'), &sequence, this);
    ThreadWeaver::JobSequence jobSequence1(this);
    jobSequence1.addNakedJob(&jobB);

    AppendCharacterJob jobC(QChar('c'), &sequence, this);
    AppendCharacterJob jobA(QChar('a'), &sequence, this);
    ThreadWeaver::JobSequence jobSequence2(this);
    jobSequence2.addNakedJob(&jobA);
    jobSequence2.addNakedJob(&jobSequence1);
    jobSequence2.addNakedJob(&jobC);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueueJob(&jobSequence2);
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QString("abc"));
}

void JobTests::SequenceOfSequencesTest() {
    QString sequence;
    AppendCharacterJob jobA ( QChar( 'a' ), &sequence, this );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence, this );
    AppendCharacterJob jobC ( QChar( 'c' ), &sequence, this );
    AppendCharacterJob jobD ( QChar( 'd' ), &sequence, this );
    AppendCharacterJob jobE ( QChar( 'e' ), &sequence, this );
    AppendCharacterJob jobF ( QChar( 'f' ), &sequence, this );
    AppendCharacterJob jobG ( QChar( 'g' ), &sequence, this );
    AppendCharacterJob jobH ( QChar( 'h' ), &sequence, this );
    AppendCharacterJob jobI ( QChar( 'i' ), &sequence, this );
    AppendCharacterJob jobJ ( QChar( 'j' ), &sequence, this );
    ThreadWeaver::JobSequence jobSequence1( this );
    jobSequence1.setObjectName( "Sequ_1" );
    jobSequence1.addNakedJob(&jobA);
    jobSequence1.addNakedJob(&jobB);
    jobSequence1.addNakedJob(&jobC);
    ThreadWeaver::JobSequence jobSequence2( this );
    jobSequence2.setObjectName( "Sequ_2" );
    jobSequence2.addNakedJob(&jobD);
    jobSequence2.addNakedJob(&jobE);
    jobSequence2.addNakedJob(&jobF);
    ThreadWeaver::JobSequence jobSequence3( this );
    jobSequence3.setObjectName( "Sequ_3" );
    jobSequence3.addNakedJob(&jobG);
    jobSequence3.addNakedJob(&jobH);
    jobSequence3.addNakedJob(&jobI);
    jobSequence3.addNakedJob(&jobJ);
    // sequence 4 will contain sequences 1, 2, and 3, in that order:
    ThreadWeaver::JobSequence jobSequence4( this );
    jobSequence4.setObjectName( "Sequ_4" );
    jobSequence4.addNakedJob(&jobSequence1);
    jobSequence4.addNakedJob(&jobSequence2);
    jobSequence4.addNakedJob(&jobSequence3);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueueJob(&jobSequence4);
    // ThreadWeaver::Job::DumpJobDependencies();
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence,QString("abcdefghij"));
}

void JobTests::QueueAndStopTest() {
    QString sequence;
    AppendCharacterJob a( 'a', &sequence );
    AppendCharacterJob b( 'b', &sequence );
    AppendCharacterJob c( 'c', &sequence );
    FailingAppendCharacterJob d( 'd', &sequence );
    AppendCharacterJob e( 'e', &sequence );
    AppendCharacterJob f( 'f', &sequence );
    AppendCharacterJob g( 'g', &sequence );
    ThreadWeaver::JobSequence jobSequence ( this );
    jobSequence.addNakedJob(&a);
    jobSequence.addNakedJob(&b);
    jobSequence.addNakedJob(&c);
    jobSequence.addNakedJob(&d);
    jobSequence.addNakedJob(&e);
    jobSequence.addNakedJob(&f);
    jobSequence.addNakedJob(&g);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueueJob(&jobSequence);
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE(sequence, QString("abcd"));
}

void JobTests::ResourceRestrictionPolicyBasicsTest () {
    // this test tests that with resource restrictions assigned, jobs
    // still get executed as expected
    QString sequence;
    ThreadWeaver::ResourceRestrictionPolicy restriction (2);
    AppendCharacterJob a( 'a', &sequence );
    AppendCharacterJob b( 'b', &sequence );
    AppendCharacterJob c( 'c', &sequence );
    AppendCharacterJob d( 'd', &sequence );
    AppendCharacterJob e( 'e', &sequence );
    AppendCharacterJob f( 'f', &sequence );
    AppendCharacterJob g( 'g', &sequence );
    ThreadWeaver::JobCollection collection;
    collection.addNakedJob( &a );
    a.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &b );
    b.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &c );
    c.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &d );
    d.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &e );
    e.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &f );
    f.assignQueuePolicy ( &restriction);
    collection.addNakedJob( &g );
    g.assignQueuePolicy ( &restriction);

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueueJob(&collection);
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
}

void JobTests::jobStarted( ThreadWeaver::JobPointer )
{
    // qDebug() << "jobStarted";
    QVERIFY( thread() == QThread::currentThread() );
}

void JobTests::jobDone( ThreadWeaver::JobPointer )
{
    // qDebug() << "jobDone";
    QVERIFY( thread() == QThread::currentThread() );
}

void JobTests::JobSignalsAreEmittedAsynchronouslyTest()
{
    char bits[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
    const int NumberOfBits = sizeof bits / sizeof bits[0];
    QString sequence;
    QList<Job*> jobs;
    ThreadWeaver::JobCollection collection;
    connect( &collection, SIGNAL(started(ThreadWeaver::JobPointer)),
             SLOT(jobStarted(ThreadWeaver::JobPointer)) );
    connect( &collection, SIGNAL(done(ThreadWeaver::JobPointer)),
             SLOT(jobDone(ThreadWeaver::JobPointer)) );
    for ( int counter = 0; counter < NumberOfBits; ++counter )
    {
        Job* job = new AppendCharacterJob( bits[counter], &sequence, this );

        connect ( job, SIGNAL(started(ThreadWeaver::JobPointer)),
                  SLOT(jobStarted(ThreadWeaver::JobPointer)) );
        connect ( job, SIGNAL(done(ThreadWeaver::JobPointer)),
                  SLOT(jobDone(ThreadWeaver::JobPointer)) );

        jobs.append( job );
        collection.addNakedJob(job);
    }

    WaitForIdleAndFinished w(ThreadWeaver::Weaver::instance());
    ThreadWeaver::Weaver::instance()->enqueueJob(&collection);
    QCoreApplication::processEvents();
    ThreadWeaver::Weaver::instance()->finish();
    QVERIFY( sequence.length() == NumberOfBits );
}

void JobTests::DequeueSuspendedSequence()
{
    return; //MARK_TEMPORARILY_DISABLED
    ThreadWeaver::Weaver weaver;
    weaver.suspend();
    weaver.enqueueJob(new ThreadWeaver::JobSequence(this) );
    weaver.dequeue();
    // don't crash
}

QTEST_MAIN ( JobTests )

#include "JobTests.moc"

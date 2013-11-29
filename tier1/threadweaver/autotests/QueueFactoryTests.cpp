#include <QString>
#include <QtTest>

#include <ThreadWeaver.h>
#include <QueueAPI.h>
#include <Queue.h>
#include <Weaver.h>
#include <WeaverImpl.h>
#include <IdDecorator.h>

using namespace ThreadWeaver;
QAtomicInt counter;

class CountingJobDecorator : public IdDecorator {
public:
    explicit CountingJobDecorator(const JobPointer& job)
        : IdDecorator(job.data(), false)
        , original_(job)
    {}

    void run(JobPointer self, Thread* thread) Q_DECL_OVERRIDE {
        counter.fetchAndAddRelease(1);
        IdDecorator::run(self, thread);
        counter.fetchAndAddAcquire(1);
    }

    JobPointer original_;
};

class JobCountingWeaver : public WeaverImpl {
    Q_OBJECT
public:
    explicit JobCountingWeaver(QObject* parent = 0) : WeaverImpl(parent) {}
    void enqueue(const JobPointer& job) Q_DECL_OVERRIDE {
        JobPointer decorated(new CountingJobDecorator(job));
        WeaverImpl::enqueue(decorated);
    }
};

class CountingGlobalQueueFactory : public Weaver::GlobalQueueFactory {
    Weaver* create(QObject* parent = 0) Q_DECL_OVERRIDE {
        return new Weaver(new JobCountingWeaver, parent);
    }
};

int argc = 0;

class QueueFactoryTests : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQueueFactory() {
        counter.storeRelease(0);
        QCoreApplication app(argc, (char**)0);
        Weaver queue(new JobCountingWeaver(this));
        queue.enqueue(make_job( [](){} )); // nop
        queue.finish();
        QCOMPARE(counter.loadAcquire(), 2);
    }

    void testGlobalQueueFactory() {
        Weaver::setGlobalQueueFactory(new CountingGlobalQueueFactory());
        QCoreApplication app(argc, (char**)0);
        counter.storeRelease(0);
        Weaver::instance()->enqueue(make_job( [](){} )); // nop
        Weaver::instance()->finish();
        QCOMPARE(counter.loadAcquire(), 2);
    }
};

QTEST_APPLESS_MAIN(QueueFactoryTests)

#include "QueueFactoryTests.moc"

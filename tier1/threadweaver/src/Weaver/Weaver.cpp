#include "Weaver.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>

#include "WeaverImpl.h"
#include "WeaverObserver.h"

using namespace ThreadWeaver;

namespace {
static Weaver::GlobalQueueFactory* globalQueueFactory;
}

class Weaver::Private
{
public:
    Private(Weaver* q, Queue *queue)
        : implementation(queue)
    {
        Q_ASSERT_X(qApp!=0, Q_FUNC_INFO, "Cannot create global ThreadWeaver instance before QApplication!");
        Q_ASSERT(queue);
        queue->setParent(q);
        q->connect(implementation, SIGNAL (finished()), SIGNAL (finished()));
        q->connect(implementation, SIGNAL (suspended()), SIGNAL (suspended()));
        q->connect(implementation, SIGNAL (jobDone(ThreadWeaver::JobPointer)), SIGNAL(jobDone(ThreadWeaver::JobPointer)));
    }

    Queue* implementation;
    void init(Queue *implementation);
};

/** @brief Construct a Weaver object. */
Weaver::Weaver(QObject* parent)
    : Queue(parent)
    , d(new Private(this, new WeaverImpl))
{
}

/** @brief Construct a Weaver object, specifying the Weaver implementation to use.
  * The Weaver instance will take ownership of the implementation object and delete it when destructed. */
Weaver::Weaver(Queue *implementation, QObject *parent)
    : Queue(parent)
    , d(new Private(this, implementation))
{
}

Weaver::~Weaver()
{
    if (d->implementation->state()->stateId()!=Destructed) {
        d->implementation->shutDown();
    }
    delete d->implementation;
    delete d;
}

QueueStream Weaver::stream()
{
    return QueueStream(this);
}

void Weaver::shutDown()
{
    d->implementation->shutDown();
}

/** @brief Set the factory object that will create the global queue.
 *
 * Once set, the global queue factory will be deleted when the global ThreadWeaver pool is deleted.
 * The factory object needs to be set before the global ThreadWeaver pool is instantiated. Call this
 * method before Q(Core)Application is constructed. */
void Weaver::setGlobalQueueFactory(Weaver::GlobalQueueFactory *factory)
{
    if (globalQueueFactory) {
        delete globalQueueFactory;
    }
    globalQueueFactory = factory;
}

const State* Weaver::state() const
{
    return d->implementation->state();
}

void Weaver::registerObserver ( WeaverObserver *ext )
{
    d->implementation->registerObserver ( ext );
}

namespace {

class StaticThreadWeaverInstanceGuard : public QObject {
    Q_OBJECT
public:
    explicit StaticThreadWeaverInstanceGuard(QAtomicPointer<Weaver>& instance, QCoreApplication* app)
        : QObject(app)
        , instance_(instance)
    {
        Q_ASSERT_X(app!=0, Q_FUNC_INFO, "Calling ThreadWeaver::Weaver::instance() requires a QCoreApplication!");
        QObject* impl = instance.load()->findChild<Queue*>();
        Q_ASSERT(impl);
        impl->setObjectName(tr("GlobalQueue"));
        qAddPostRoutine(shutDownGlobalQueue);
    }

    ~StaticThreadWeaverInstanceGuard() {
        instance_.fetchAndStoreOrdered(0);
        delete globalQueueFactory;
        globalQueueFactory = 0;
    }
private:
    static void shutDownGlobalQueue() {
        Weaver::instance()->shutDown();
        Q_ASSERT(Weaver::instance()->state()->stateId() == Destructed);
    }

    QAtomicPointer<Weaver>& instance_;
};

}

/** @brief The application-global Weaver instance.
 * This  instance will only be created if this method is actually called in the lifetime of the application.
 * The method will create the Weaver instance on first call. The Q(Core)Application object must exist at that time.
 * The instance will be deleted when Q(Core)Application is destructed. After that, the instance() method returns zero. */
Weaver* Weaver::instance()
{
    static QAtomicPointer<Weaver> s_instance(globalQueueFactory
                                             ? globalQueueFactory->create(qApp)
                                             : new Weaver(qApp));
    //Order is of importance here:
    //When s_instanceGuard is destructed (first, before s_instance), it sets the value of s_instance to zero. Next, qApp will delete
    //the object s_instance pointed to.
    static StaticThreadWeaverInstanceGuard* s_instanceGuard = new StaticThreadWeaverInstanceGuard(s_instance, qApp);
    Q_UNUSED(s_instanceGuard);
    Q_ASSERT_X(s_instance.load() == 0 || s_instance.load()->thread() == QCoreApplication::instance()->thread(),
               Q_FUNC_INFO,
               "The global ThreadWeaver queue needs to be instantiated (accessed first) from the main thread!");
    return s_instance.loadAcquire();
}

void Weaver::enqueue(const JobPointer &job)
{
    d->implementation->enqueue(job);
}

bool Weaver::dequeue(const JobPointer& job)
{
    return d->implementation->dequeue(job);
}

void Weaver::dequeue ()
{
    return d->implementation->dequeue();
}

void Weaver::finish ()
{
    return d->implementation->finish ();
}

void Weaver::suspend ()
{
    return d->implementation->suspend();
}

void Weaver::resume ()
{
    return d->implementation->resume();
}

bool Weaver::isEmpty() const
{
    return d->implementation->isEmpty();
}

bool Weaver::isIdle() const
{
    return d->implementation->isIdle();
}

int Weaver::queueLength() const
{
    return d->implementation->queueLength();
}

void Weaver::setMaximumNumberOfThreads( int cap )
{
    d->implementation->setMaximumNumberOfThreads( cap );
}

int Weaver::currentNumberOfThreads() const
{
    return d->implementation->currentNumberOfThreads();
}

int Weaver::maximumNumberOfThreads() const
{
    return d->implementation->maximumNumberOfThreads();
}

void Weaver::requestAbort()
{
    d->implementation->requestAbort();
}

void Weaver::reschedule()
{
    d->implementation->reschedule();
}

#include "Weaver.moc"

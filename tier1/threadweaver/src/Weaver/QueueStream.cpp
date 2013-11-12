#include <QVector>

#include "Weaver.h"
#include "QueueStream.h"

namespace ThreadWeaver {

class QueueStream::Private {
public:
    Private(Weaver* w) : weaver(w) {}
    Weaver* weaver;
    QVector<JobPointer> jobs;
};

QueueStream::QueueStream(Weaver *queue)
    : d(new Private(queue))
{
    Q_ASSERT(queue);
}

QueueStream::~QueueStream()
{
    if (d->jobs.isEmpty()) {
        return;
    }
    Q_ASSERT(d->weaver);
    //FIXME: this should be one atomic operation:
    Q_FOREACH(const JobPointer& job, d->jobs) {
        d->weaver->enqueue(job);
    }
    delete d;
}


void ThreadWeaver::QueueStream::add(const ThreadWeaver::JobPointer &job)
{
    d->jobs.append(job);
}

/** @brief Return a stream the enqueues jobs in the ThreadWeaver global queue. */
QueueStream queue()
{
    return QueueStream(Weaver::instance());
}

/** @brief Return a stream the enqueues jobs in the specified queue. */
QueueStream queue(Weaver *weaver)
{
    return QueueStream(weaver);
}

}



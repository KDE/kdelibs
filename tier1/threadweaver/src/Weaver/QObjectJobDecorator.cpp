#include <QObjectJobDecorator.h>
#include <JobCollection.h>
#include <JobSequence.h>
#include <ManagedJobPointer.h>

namespace ThreadWeaver {

QObjectJobDecorator::QObjectJobDecorator(JobInterface *decoratee, QObject *parent)
    : QObject(parent)
    , IdDecorator(decoratee)
{
}

void QObjectJobDecorator::defaultBegin(JobPointer self, Thread *thread)
{
    Q_ASSERT(job());
    Q_EMIT started(self);
    job()->defaultBegin(self, thread);
}

void QObjectJobDecorator::defaultEnd(JobPointer self, Thread *thread)
{
    Q_ASSERT(job());
    job()->defaultEnd(self, thread);
    if (!self->success()) {
        Q_EMIT failed(self);
    }
    Q_EMIT done(self);
}

}

#include "moc_QObjectJobDecorator.cpp"

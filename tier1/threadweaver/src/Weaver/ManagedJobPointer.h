#ifndef MANAGEDJOBPOINTER_H
#define MANAGEDJOBPOINTER_H

#include <QSharedPointer>
#include <JobInterface.h>

namespace ThreadWeaver {

inline void doNotDeleteJob(JobInterface*) {}

template<typename T>
class ManagedJobPointer : public QSharedPointer<T>
{
public:
    ManagedJobPointer(T *job)
        : QSharedPointer<T>(job, doNotDeleteJob) {}
};

}

#endif // MANAGEDJOBPOINTER_H

#ifndef THREADWEAVER_QUEUEING_H
#define THREADWEAVER_QUEUEING_H

#include "JobPointer.h"
#include "ManagedJobPointer.h"
#include "JobInterface.h"
#include "JobCollection.h"
#include "Lambda.h"
#include "QObjectDecorator.h"
#include "Weaver.h"

namespace ThreadWeaver {

// make a job that calls a functor, anything that responds to operator()
template<typename T>
QSharedPointer<Lambda<T> > make_job(T t) {
    QSharedPointer<Lambda<T> > ret(new Lambda<T>(t));
    return ret;
}

// make a job pointer holding a pointer to a Job(Interface)
template<typename T>
inline QSharedPointer<T> make_job(T* job) {
    JobInterface* test = static_cast<JobInterface*>(job); Q_UNUSED(test);
    return QSharedPointer<T>(job);
}

// make a job pointer holding anything resembling JobInterface
inline JobPointer make_job_raw(JobInterface* job) {
    return ManagedJobPointer<JobInterface>(job);
}

// enqueue any functor type to the specified queue:
template<typename T>
JobPointer enqueue(Weaver* weaver, T t) {
    JobPointer ret = make_job(t);
    weaver->enqueue(ret);
    return ret;
}

template<typename T>
QSharedPointer<T>  enqueue(Weaver* weaver, T* t) {
    JobInterface* test = static_cast<JobInterface*>(t); Q_UNUSED(test);
    QSharedPointer<T> ret(make_job(t));
    weaver->enqueue(ret);
    return ret;
}

// specialise for JobPointer:
template<>
inline JobPointer enqueue<JobPointer>(Weaver* weaver, JobPointer job) {
    weaver->enqueue(job);
    return job;
}

// convenience overload: enqueue the functor to the global queue:
template<typename T>
JobPointer enqueue(T t) {
    return enqueue(Weaver::instance(), t);
}

// enqueue a raw pointer with no memory management
inline JobPointer enqueue_raw(Weaver* weaver, JobInterface* job) {
    return enqueue(weaver, make_job_raw(job));
}

// overload to enqueue to the global pool
inline JobPointer enqueue_raw(JobInterface* job) {
    return enqueue(Weaver::instance(), make_job_raw(job));
}

}

#endif // THREADWEAVER_QUEUEING_H

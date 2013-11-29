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

// make a job that calls a functor, anything that responds to operator[]
template<typename T>
JobPointer make_job(T t) {
    JobPointer ret(new Lambda<T>(t));
    return ret;
}

// make a job pointer holding a pointer to a Job(Interface)
template<typename T>
inline JobPointer make_job(T* job) {
    return JobPointer(static_cast<JobInterface*>(job));
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
JobPointer enqueue(Weaver* weaver, T* t) {
    JobPointer ret(make_job(static_cast<JobInterface*>(t)));
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

// create a QObjectDecorator decorating the job
inline QObjectDecorator* decorate_q(JobInterface* job) {
    return new QObjectDecorator(job);
}

}

#endif // THREADWEAVER_QUEUEING_H

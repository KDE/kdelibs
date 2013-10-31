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
namespace Queueing {

// make a job that calls a functor, anything that responds to operator[]
template<typename T>
JobPointer make_job(T t) {
    JobPointer ret(new Lambda<T>(t));
    return ret;
}

// make a job pointer holding a pointer to a Job(Interface)
template<>
inline JobPointer make_job<JobInterface*>(JobInterface* job) {
    return JobPointer(job);
}

// enqueue any functor type to the specified queue:
template<typename T>
JobPointer enqueue(Weaver* weaver, T t) {
    JobPointer ret = make_job(t);
    weaver->enqueue(ret);
    return ret;
}

// specialise for QObjectDecorator:
template<>
inline JobPointer enqueue<QObjectDecorator*>(Weaver* weaver, QObjectDecorator* q) {
    JobPointer ret(q);
    weaver->enqueue(ret);
    return ret;
}

// specialise for JobPointer:
template<>
inline JobPointer enqueue<JobPointer>(Weaver* weaver, JobPointer job) {
    weaver->enqueue(job);
    return job;
}

//// specialise for JobInterface:
//template<>
//JobPointer enqueue<JobInterface*>(Weaver* weaver, JobInterface* job) {
//    return enqueue(weaver, make_job(job));
//}

//// specialise for Collection:
//template<>
//JobPointer enqueue<JobCollection*>(Weaver* weaver, JobCollection* job) {
//    return enqueue(weaver, make_job(job));
//}

//// specialise for Sequence:
//template<>
//JobPointer enqueue<JobSequence*>(Weaver* weaver, JobSequence* job) {
//    return enqueue(weaver, make_job(job));
//}

// convenience overload: enqueue the functor to the global queue:
template<typename T>
JobPointer enqueue(T t) {
    return enqueue(Weaver::instance(), t);
}

// enqueue a raw pointer with no memory management
template<typename T>
JobPointer enqueue_raw(Weaver* weaver, T* t) {
    ManagedJobPointer<T> ret(t);
    weaver->enqueue(ret);
    return ret;
}

// create a QObjectDecorator decorating the job
inline QObjectDecorator* decorate_q(JobInterface* job) {
    return new QObjectDecorator(job);
}

}
}

inline ThreadWeaver::JobCollection& operator<<(ThreadWeaver::JobCollection& collection, ThreadWeaver::JobInterface* job) {
    collection.addJob(ThreadWeaver::Queueing::make_job(job));
    return collection;
}

inline ThreadWeaver::JobCollection& operator<<(ThreadWeaver::JobCollection& collection, const ThreadWeaver::JobPointer& job) {
    collection.addJob(job);
    return collection;
}

#endif // THREADWEAVER_QUEUEING_H

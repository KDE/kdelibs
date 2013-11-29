#ifndef QUEUESTREAM_H
#define QUEUESTREAM_H

#include "JobPointer.h"
#include "JobInterface.h"
#include "Queueing.h"
#include "threadweaver_export.h"

namespace ThreadWeaver {

/** @brief QueueStream implements a stream based API to access ThreadWeaver queues. */
class THREADWEAVER_EXPORT QueueStream {
public:
    explicit QueueStream(Weaver* queue);
    ~QueueStream();
    void add(const JobPointer& job);

    inline QueueStream& operator<<(const JobPointer& job) { add(job); return *this; }
    inline QueueStream& operator<<(JobInterface* job) { add(make_job(job)); return *this; }
    inline QueueStream& operator<<(Job& job) { add(make_job_raw(&job)); return *this; }

private:
    class Private;
    Private * const d;
};

QueueStream THREADWEAVER_EXPORT queue();
QueueStream THREADWEAVER_EXPORT queue(Weaver* weaver);

}

#endif // QUEUESTREAM_H

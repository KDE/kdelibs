#ifndef THREADWEAVER_JOBPOINTER_H
#define THREADWEAVER_JOBPOINTER_H

#include <QSharedPointer>

namespace ThreadWeaver {

class Job;

typedef QSharedPointer<Job> JobPointer;

}

#include <Weaver/Job.h>

Q_DECLARE_METATYPE(ThreadWeaver::JobPointer)

#endif // THREADWEAVER_JOBPOINTER_H

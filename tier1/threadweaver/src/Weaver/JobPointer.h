#ifndef JOBPOINTER_H
#define JOBPOINTER_H

#include <QSharedPointer>

#include <Weaver/Job.h>

namespace ThreadWeaver {

typedef QSharedPointer<Job> JobPointer;

}

Q_DECLARE_METATYPE(ThreadWeaver::JobPointer)

#endif // JOBPOINTER_H

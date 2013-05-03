#ifndef JOBPOINTER_H
#define JOBPOINTER_H

#include <QSharedPointer>

#include <Weaver/Job.h>

namespace ThreadWeaver {

typedef QSharedPointer<Job> JobPointer;

}

#endif // JOBPOINTER_H

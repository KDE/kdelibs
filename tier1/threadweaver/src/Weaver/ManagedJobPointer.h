#ifndef MANAGEDJOBPOINTER_H
#define MANAGEDJOBPOINTER_H

#include <QSharedPointer>
#include <JobInterface.h>

namespace ThreadWeaver {

class ManagedJobPointer : public QSharedPointer<JobInterface>
{
public:
    ManagedJobPointer(JobInterface *job);
};

}

#endif // MANAGEDJOBPOINTER_H

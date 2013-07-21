#ifndef MANAGEDJOBPOINTER_H
#define MANAGEDJOBPOINTER_H

#include <QSharedPointer>
#include <Weaver/Job.h>

namespace ThreadWeaver {

class ManagedJobPointer : public QSharedPointer<Job>
{
public:
    ManagedJobPointer(Job *job);
};

};

#endif // MANAGEDJOBPOINTER_H

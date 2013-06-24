#include "ManagedJobPointer.h"

namespace ThreadWeaver {

void doNotDeleteJob(Job*) {}

ManagedJobPointer::ManagedJobPointer(Job* job)
    : QSharedPointer<Job>(job,doNotDeleteJob)
{
}

}

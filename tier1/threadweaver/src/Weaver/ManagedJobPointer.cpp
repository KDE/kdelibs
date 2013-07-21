#include "ManagedJobPointer.h"

namespace ThreadWeaver {

void doNotDeleteJob(JobInterface*) {}

ManagedJobPointer::ManagedJobPointer(JobInterface *job)
    : QSharedPointer<JobInterface>(job,doNotDeleteJob)
{
}

}

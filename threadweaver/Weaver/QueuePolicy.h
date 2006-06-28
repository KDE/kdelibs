#ifndef QUEUEPOLICY_H
#define QUEUEPOLICY_H

namespace ThreadWeaver {

/** QueuePolicy is an interface for customizations of the queueing behaviour of sets of jobs.

A job can have a queueing policy assigned. In that case, the job is only
executed when the method canRun() of the assigned policy returns true. For
every call to canRun() that returns true, it is guaranteed that the method
free() or the method release() is called. Calling free() means the job has
been executed, while calling release() means the job was not executed for
external reasons, and will be retried lateron.

As an example, dependencies can be implemented using a QueuePolicy:
canRun() returns true when the job has no unresolved dependencies. free()
and release() are empty.

A job can have multiple queue policies assigned, and will only be executed
if all of them return true from canRun() within the same execution attempt.
Jobs only keep a reference to the QueuePolicy. Therefore, the same object
can be assigned to multiple jobs and this way control the way all those
jobs are executed.
*/

    class Job;

    class QueuePolicy
    {
    public:
        virtual ~QueuePolicy() {}

        virtual bool canRun( Job* ) = 0;

        virtual void free( Job* ) = 0;

        virtual void release( Job* ) = 0;
    };

}

#endif

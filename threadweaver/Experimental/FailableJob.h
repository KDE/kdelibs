#ifndef FAILABLEJOB_H
#define FAILABLEJOB_H

/** FailableJob is a Job that only resolves it's dependencies if it
    successfully finished.

    The generic Job class has no notion of whether the operation has been
    successful or not. To add this capability, FailableJob implementations
    need to implement the success() method. success() is supposed to return
    true if the operation wa successful, false otherwise.
    If it is true, the object will resolve it's dependencies. Otherwise, it
    will emit the failed() signal. Jobs that depend on this object will then
    not be executed.

    The developer has to make sure that jobs that become invalidated by the
    failure are dequeued or otherwise unwound.

    Note: Any Job object that is deleted <b>will</b> resolve it's
    dependencies. This is necessary to avoid references to non-existant
    objects. Therefore, all cleanup operations (like dequeuing of dependant
    jobs) need to be finished before the Job that failed is deleted.

    FailableJob's usefulness has been suggested at the 2005 akademy conference.
*/

#include "Job.h"

namespace ThreadWeaver {

    class FailableJob : public Job
    {
        Q_OBJECT
    public:
        /** Standard constructor. */
        FailableJob ( QObject *parent = 0 );
        /** Return whether the Job finished successfully or not.
         */
        virtual bool success () const = 0;
    protected:
        /** Overload the execute method.

            It will emit failed() for non-zero return values after run().
         */
        virtual void execute ( Thread* );
    signals:
        /** The run() method has finished with a non-zero return
            value. Dependencies will not be resolved. */
        void failed ( Job* );
    private:
        /** Overload the resolveDependencies() method.

            The method is adapted to act depending on the return code.
         */
        void resolveDependencies();
    };

}

#endif

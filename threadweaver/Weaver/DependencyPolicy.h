#ifndef DEPENDENCYPOLICY_H
#define DEPENDENCYPOLICY_H

#include "QueuePolicy.h"

namespace ThreadWeaver {

    class DependencyPolicy : public QueuePolicy
    {
    public:
        /** Add jobA as a dependency of jobB.
            jobA will only be executed after jobB has been successfully processed.
            @param jobA the depending job
            @param jobB the job jobA depends on
        */
        void addDependency( Job* jobA,  Job* jobB );

        /** Remove dependency.

            The dependency of jobA on jobB is removed. If no dependencies are
            left for jobA, canRun will return true.

            Returns false if the given object is not dependency of this job.

	    This function is inefficient, and should be used only to abort
	    execution of a job.

	    @param jobA the depending job
	    @param jobB the job jobA depends on
	    @return true if dependency existed, false otherwise
        */
        bool removeDependency( Job* jobA, Job* jobB );

        /** Resolve all dependencies.
            This method is called after the Job has been finished, or
            when it is deleted without being executed (performed by the
            destructor).
            The method will remove all entries stating that another Job
            depends on this one.
        */
        void resolveDependencies ( Job* );

        /** Retrieve a list of dependencies of this job. */
        QList<Job*> getDependencies( Job* ) const;

        static DependencyPolicy& instance();

        bool canRun( Job* );

        void free( Job* );

        void release( Job* );

        void destructed( Job* );

    protected:
        /** Query whether the job has an unresolved dependency.
            In case it does, the policy will return false from canRun().
        */
        bool hasUnresolvedDependencies( Job* );

        DependencyPolicy();

    private:
        class Private;
        Private* d;

    public:
        /** This method should be useful for debugging purposes. */
        void dumpJobDependencies();
    };

}

#endif

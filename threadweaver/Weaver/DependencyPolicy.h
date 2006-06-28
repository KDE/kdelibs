#ifndef DEPENDENCYPOLICY_H
#define DEPENDENCYPOLICY_H

#include "QueuePolicy.h"

namespace ThreadWeaver {

    class DependencyPolicy : public QueuePolicy
    {
    public:
        static void addDependency( Job* dependency,  Job* dependant );
        static void resolveDependencies ( Job* );

        static DependencyPolicy& instance();

        bool canRun( Job* );

        void free( Job* );

        void release( Job* );

    protected:
        bool hasUnresolvedDependencies( Job* );

        DependencyPolicy() {}
    };

}

#endif

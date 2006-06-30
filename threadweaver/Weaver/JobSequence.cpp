#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include "JobSequence.h"
#include "DependencyPolicy.h"

namespace ThreadWeaver {

    JobSequence::JobSequence ( QObject *parent )
        : JobCollection ( parent )
    {
    }

    void JobSequence::aboutToBeQueued ( WeaverInterface *weaver )
    {
        int i;

        if ( jobListLength() > 1 )
        {
            // set up the dependencies:
            for ( i = 0; i < jobListLength() -1 ; ++i )
            {
                P_ASSERT ( jobAt( i ) != 0 );
                P_ASSERT ( jobAt( i+1 ) != 0 );
                DependencyPolicy::instance().addDependency ( jobAt( i ), jobAt( i+1 ) );
            }
        }

        JobCollection::aboutToBeQueued( weaver );
    }

    void JobSequence::jobFailed( Job* job)
    {
        stop( job );
    }

}


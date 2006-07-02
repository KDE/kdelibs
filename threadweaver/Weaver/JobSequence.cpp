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
      if ( jobListLength() > 1 )
        {
            // set up the dependencies:
            for ( int i = 1; i < jobListLength(); ++i )
            {
	      Job* jobA = jobAt(i);
	      Job* jobB = jobAt(i-1);
	      P_ASSERT ( jobA != 0 );
	      P_ASSERT ( jobB != 0 );
	      DependencyPolicy::instance().addDependency ( jobA, jobB );
            }
        }

        JobCollection::aboutToBeQueued( weaver );
    }

    void JobSequence::internalJobDone( Job* job)
    {
      if ( ! job->success() )
	{
	  stop( job );
	}
      JobCollection::internalJobDone(job);
    }

}


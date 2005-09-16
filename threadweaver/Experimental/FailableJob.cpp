#include <QObject>

#include "FailableJob.h"

namespace ThreadWeaver {

    FailableJob::FailableJob ( QObject *parent )
        : Job ( parent )
    {
    }

    void FailableJob::execute ( Thread *t )
    {
        Job::execute ( t );
        if ( ! success() )
        {
            emit ( failed ( this ) );
        }
    }

    void FailableJob::resolveDependencies ()
    {
        if ( success() )
        {
            Job::resolveDependencies();
        } // otherwise we do not
    }


}

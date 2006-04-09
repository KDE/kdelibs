#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include <QList>
#include <QPointer>

#include "JobCollection.h"

namespace ThreadWeaver {

    /* QPointers are used internally to be able to dequeue jobs at destruction
       time. The owner of the jobs could have deleted them in the meantime.
       We use a class instead of a typedef to be able to forward-declare the
       class in the declaration.
    */
    class JobCollection::JobList : public QList < QPointer <Job> > {};

    JobCollection::JobCollection ( QObject *parent )
        : Job ( parent )
        , m_elements ( new JobList() )
        , m_queued ( false )
        , m_weaver ( 0 )
    {
    }

    JobCollection::~JobCollection()
    {
        // dequeue all remaining jobs:
        if ( m_weaver )
        {
            for ( int i = 1; i < m_elements->size(); ++i )
            {
                if ( m_elements->at( i ) ) // ... a QPointer
                {
                    if ( ! m_elements->at( i )->isFinished() )
                    {
                        m_weaver->dequeue ( m_elements->at( i ) );
                    }
                }
            }
        }

        delete m_elements;
    }

    void JobCollection::addJob ( Job *j )
    {
        P_ASSERT ( m_queued == false );

        m_elements->append ( QPointer<Job> ( j ) );
    }

    void JobCollection::stop( Job *job )
    {
        Q_UNUSED( job );
        P_ASSERT ( m_queued == true ); // should only be stopped once started,
                                       // maybe a bit strict...
        // job has failed, so we dequeue everything after job:
        // find job in m_elements:
        for ( int index = 1; index < m_elements->size(); ++index )
        {   // no job should have been deleted while it is queued:
            P_ASSERT( m_elements->at(  index ) );
            if ( ! m_elements->at( index )->isFinished() )
            {
                m_weaver->dequeue ( m_elements->at( index ) );
            }
        }
        m_weaver->dequeue( this );
    }

    void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
    {
        int i;

        m_weaver = weaver;

        if ( m_elements->size() > 0 )
        {
            // set up the dependencies:
            for ( i = 1; i < m_elements->size(); ++i )
            {
                P_ASSERT ( m_elements->at( i ) != 0 );
                addDependency( m_elements->at( i ) );
                m_weaver->enqueue( m_elements->at( i ) );
            }
        }
        m_queued = true;
    }

    void JobCollection::execute ( Thread *t )
    {
        if ( ! m_elements->isEmpty() )
        {   // this is a hack (but a good one): instead of queueing (this), we
            // execute the last job, to avoid to have (this) wait for an
            // available thread (the last operation does not get queued in
            // aboutToBeQueued() )
            P_ASSERT ( m_elements->at( 0 ) != 0 );

            m_elements->at( 0 )->execute ( t );
        }
        Job::execute ( t ); // run() is empty
    }

}

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

    class JobCollection::DummyJob : public Job
    {
    public:
        DummyJob ()
            : Job ()
        {}

        void run() {}
    };

    JobCollection::JobCollection ( QObject *parent )
        : Job ( parent )
        , m_elements ( new JobList() )
        , m_queued ( false )
        , m_weaver ( 0 )
        , m_dummy ( 0 )
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

        m_elements->prepend ( QPointer<Job> ( j ) );
    }

    void JobCollection::stop( Job *job )
    {
        Q_UNUSED( job );
        P_ASSERT ( m_queued == true ); // should only be stopped once started,
                                       // maybe a bit strict...

        // dequeue everything:
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
        Q_ASSERT ( m_queued == false ); // never queue twice

        int i;

        m_weaver = weaver;

        if ( hasUnresolvedDependencies() )
        {
            m_dummy = new DummyJob ();
        }

        if ( m_elements->size() > 0 )
        {
            // set up the dependencies:
            for ( i = 1; i < m_elements->size(); ++i )
            {
                Job* job = m_elements->at( i );
                P_ASSERT ( job != 0 );

                addDependency( job );

                if ( m_dummy ) // we have unresolved dependencies at queueing time
                {
                    job->addDependency( m_dummy );
                }

                m_weaver->enqueue( job );
            }

            m_elements->at( 0 )->aboutToBeQueued( weaver );
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

    Job* JobCollection::jobAt( int i )
    {
        return m_elements->at( i );
    }

    const int JobCollection::jobListLength()
    {
        return m_elements->size();
    }

    bool JobCollection::hasUnresolvedDependencies()
    {
        bool unresolved = Job::hasUnresolvedDependencies();

        if ( ! unresolved )
        {   // this will delete the dependencies of all jobs on m_dummy:
            // delete 0; is ok:
            delete m_dummy; m_dummy = 0;
        }

        return unresolved;
    }

}


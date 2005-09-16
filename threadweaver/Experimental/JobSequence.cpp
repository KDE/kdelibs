#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include "JobSequence.h"

namespace ThreadWeaver {

    JobSequence::JobSequence ( QObject *parent )
        : Job ( parent ),
          m_queued ( false ),
          m_weaver ( 0 )
    {
    }

    JobSequence::~JobSequence()
    {
        // dequeue all remaining jobs:
        if ( m_weaver )
        {
            for ( int i = 0; i < m_elements.size(); ++i )
            {
                if ( ! m_elements[i]->isFinished() )
                {
                    m_weaver->dequeue ( m_elements[i] );
                }
            }
        }
    }

    void JobSequence::append ( Job *j )
    {
        P_ASSERT ( m_queued == false );

        m_elements.append ( j );
    }

    void JobSequence::stop( Job *job )
    {
        P_ASSERT ( m_queued == true );
        // job has failed, so we dequeue everything after job:
        // find job in m_elements:
        int index = m_elements.indexOf ( job );
        P_ASSERT ( index != -1 && m_weaver != 0 );
        // dequeue all jobs after it:
        if ( index != -1 && index < m_elements.size() - 1 )
        {
            for ( int i = index; i < m_elements.size(); ++i )
            {
                m_weaver->dequeue ( m_elements.at ( i ) );
            }
        }
    }

    void JobSequence::aboutToBeQueued ( WeaverInterface *weaver )
    {
        int i;

        m_weaver = weaver;
        if ( m_elements.size() > 1 )
        {
            // set up the dependencies:
            for ( i = 1; i < m_elements.size() - 1; ++i )
            {
                m_elements[i]->addDependency ( m_elements[i-1] );
            }
            addDependency ( m_elements[i-1] );
        }

        // queue the sequence:
        if ( m_elements.size () > 1 )
        {
            weaver->enqueue ( m_elements.mid(0,  m_elements.size() -1 ) );
        }
        m_queued = true;
    }

    void JobSequence::execute ( Thread *t )
    {
        if ( ! m_elements.isEmpty() )
        {   // this is a hack (but a good one): instead of queueing (this), we
            // execute the last job, to avoid to have (this) wait for an
            // available thread (the last operation does not get queued in
            // aboutToBeQueued() )
            int pos = m_elements.size() - 1;
            m_elements[pos]->execute ( t );
        }
        Job::execute ( t ); // run() is empty
    }

}

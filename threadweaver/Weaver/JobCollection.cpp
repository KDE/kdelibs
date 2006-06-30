#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include <QList>
#include <QObject>
#include <QPointer>

#include "JobCollection.h"

using namespace ThreadWeaver;

/* QPointers are used internally to be able to dequeue jobs at destruction
   time. The owner of the jobs could have deleted them in the meantime.
   We use a class instead of a typedef to be able to forward-declare the
   class in the declaration.
*/
class ThreadWeaver::JobCollectionJobRunner : public Job
{
  Q_OBJECT

public:
  JobCollectionJobRunner ( JobCollection* collection, Job* payload, QObject* parent )
    : Job( parent )
    , m_payload( payload )
    , m_collection( collection )
  {
    Q_ASSERT ( payload ); // will not accept zero jobs
    if ( ! m_payload->objectName().isEmpty() )
      {   // this is most useful for debugging...
	setObjectName ( tr( "JobRunner executing " ) + m_payload->objectName() );
      }
  }

  bool canBeExecuted()
  {   // the JobCollectionJobRunner object never have any dependencies:
    return m_payload->canBeExecuted();
  }

  Job* payload ()
  {
    return m_payload;
  }

  void aboutToBeQueued ( WeaverInterface *weaver )
  {
    m_payload->aboutToBeQueued( weaver );
  }

  void aboutToBeDequeued ( WeaverInterface *weaver )
  {
    m_payload->aboutToBeDequeued( weaver );
  }

  void execute ( Thread *t )
  {
    if ( m_payload )
      {
	m_payload->execute ( t );
	m_collection->internalJobDone ( m_payload);
      } else {
	debug ( 1, "JobCollection: job in collection has been deleted." );
      }
    Job::execute ( t );
  }

private:
  void run () {}

  QPointer<Job> m_payload;
  JobCollection* m_collection;
};

class JobCollection::JobList : public QList <JobCollectionJobRunner*> {};

JobCollection::JobCollection ( QObject *parent )
  : Job ( parent )
  , m_elements ( new JobList() )
  , m_weaver ( 0 )
  , m_jobCounter (0)
{
}

JobCollection::~JobCollection()
{   // dequeue all remaining jobs:
  dequeueElements();  
  delete m_elements;
  // QObject cleanup takes care of the job runners
}

void JobCollection::addJob ( Job *j )
{
  P_ASSERT ( m_weaver == 0 );
  m_elements->append ( new JobCollectionJobRunner( this, j, this ) );
}

void JobCollection::stop( Job *job )
{   // this only works if there is an event queue executed by the main
    // thread, and it is not blocked:
  Q_UNUSED( job );

  if ( m_weaver != 0 )
    {
      debug( 4, "JobCollection::stop: dequeueing %p.\n", this);
      m_weaver->dequeue( this );
    }
}

void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
{
  Q_ASSERT ( m_weaver == 0 ); // never queue twice
  m_weaver = weaver;
   
  if ( m_elements->size() > 0 )
    {
      m_elements->at( 0 )->aboutToBeQueued( weaver );
    }
}

void JobCollection::aboutToBeDequeued( WeaverInterface* weaver )
{
  Q_ASSERT ( m_weaver != 0 ); // must have been queued first

  if ( m_weaver )
    {
      dequeueElements();

      m_elements->at( 0 )->aboutToBeDequeued( weaver );
    }

  m_weaver = 0;
}


void JobCollection::execute ( Thread *t )
{
  Q_ASSERT ( m_weaver != 0);
  
  // FIXME make sure this is async:
  emit (started (this));

  if ( ! m_elements->isEmpty() )
    {  
      // set job counter:
      m_jobCounter = m_elements->size();

      // queue elements:
      for (int index = 1; index < m_elements->size(); ++index)
	{
	  m_weaver->enqueue (m_elements->at(index));
	}
      
      // this is a hack (but a good one): instead of queueing (this), we
      // execute the last job, to avoid to have (this) wait for an
      // available thread (the last operation does not get queued in
      // aboutToBeQueued() )
      // NOTE: this also calls internalJobDone()
      m_elements->at( 0 )->execute ( t );
    }

  freeQueuePolicyResources();
  // do not emit done, done is emitted when the last job called internalJobDone()
}

Job* JobCollection::jobAt( int i )
{
  qDebug("unecessary" );
  return m_elements->at( i )->payload();
}

const int JobCollection::jobListLength()
{
  qDebug("unecessary" );
  return m_elements->size();
}

bool JobCollection::canBeExecuted()
{
  bool inheritedCanRun;

  if ( m_elements->size() > 0 )
    {
      inheritedCanRun = m_elements->at( 0 )->canBeExecuted();
    } else {
      inheritedCanRun = false;
    }
  return Job::canBeExecuted() || inheritedCanRun;
}

void JobCollection::internalJobDone ( Job* job )
{
  Q_UNUSED (job);
  --m_jobCounter;
  if (m_jobCounter == 0)
    {
      emit done(this);
      if (! success())
	{
	  emit failed(this);
	}
    }
}

void JobCollection::dequeueElements()
{   // dequeue everything:
  if ( m_weaver != 0 )
    for ( int index = 1; index < m_elements->size(); ++index )
      {
	if ( m_elements->at( index ) && ! m_elements->at( index )->isFinished() ) // ... a QPointer
	  {
	    debug( 4, "JobCollection::dequeueElements: dequeueing %p.\n", m_elements->at( index ) );
	    m_weaver->dequeue ( m_elements->at( index ) );
	  } else {
	    debug( 4, "JobCollection::dequeueElements: not dequeueing %p, already finished.\n",
		   m_elements->at( index ) );
	    // this returns false if the job was not in the queue, which we assume:
	    Q_ASSERT ( ! m_weaver->dequeue ( m_elements->at( index ) ) );
	  }
      }
}

#include "JobCollection.moc"

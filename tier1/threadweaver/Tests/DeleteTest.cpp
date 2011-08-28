#include "DeleteTest.h"

#include <QtCore/QChar>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtTest/QtTest>

#include <Job.h>
#include <State.h>
#include <QueuePolicy.h>
#include <JobSequence.h>
#include <JobCollection.h>
#include <DebuggingAids.h>
#include <WeaverObserver.h>
#include <DependencyPolicy.h>
#include <ResourceRestrictionPolicy.h>

#include <ThreadWeaver.h>
#include <Thread.h>

#include "AppendCharacterJob.h"

int main(int argc, char **argv)
{
  DeleteTest dt(argc, argv);
  return dt.exec();
}

DeleteTest::DeleteTest(int argc, char **argv)
  : QCoreApplication(argc, argv)
{
  ThreadWeaver::setDebugLevel ( true,  1 );

  ThreadWeaver::Weaver::instance()->setMaximumNumberOfThreads(4);

  m_finishCount = 100;

  for (int i = 0; i < 100; ++i) {
    ThreadWeaver::JobSequence* jobSeq = new ThreadWeaver::JobSequence( this );
    connect ( jobSeq, SIGNAL( done( ThreadWeaver::Job* ) ),
              SLOT( deleteSequence( ThreadWeaver::Job* ) ) );

    jobSeq->addJob( new BusyJob );
    jobSeq->addJob( new BusyJob );

    ThreadWeaver::Weaver::instance()->enqueue( jobSeq );
  }

  ThreadWeaver::Weaver::instance()->resume();
}

void DeleteTest::deleteSequence(ThreadWeaver::Job* job)
{
  Q_ASSERT(job);
  delete job;

  QMutexLocker lock(&m_finishMutex);
  --m_finishCount;
  if (m_finishCount == 0)
    exit(0);
}

QMutex s_GlobalMutex;

#include "DeleteTest.moc"

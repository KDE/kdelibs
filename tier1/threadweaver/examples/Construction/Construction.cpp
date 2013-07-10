/* -*- C++ -*-

   This file implements the basic Construction test. It tests job queueing and
   queue handling in simple applications without event loops.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: Construction.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <Job.h>
#include <Thread.h>
#include <ThreadWeaver.h>
#include <DebuggingAids.h>

const int Multiplier = 20; // [0..9]* Multiplier sleep time for each job
const int NoOfThreads = qMax(2 * QThread::idealThreadCount(), 4);
const int NoOfJobs = NoOfThreads * 4; // number of jobs for every thread

using namespace ThreadWeaver;

class WaitJob : public Job
{
    Q_OBJECT
public:
    WaitJob(QObject* parent = 0)
        : Job(parent)
        , m_wait(0) {
        m_wait = Multiplier * (1 + s_count++ % 10);
    }
    ~WaitJob() {}
protected:
    void run () {
        debug(0, "WaitJob::run: doing it - sleeping %i milliseconds.\n", m_wait);
        thread()->msleep( m_wait );
        debug(0, "WaitJob::run: done.\n");
    }
private:
    int m_wait;
    static int s_count;
};

int WaitJob::s_count;

int main ( int argc,  char** argv )
{
    QCoreApplication app(argc, argv);
    ThreadWeaver::setDebugLevel(true, 4);

    ThreadWeaver::Weaver weaver;
    weaver.setMaximumNumberOfThreads( NoOfThreads );
    WaitJob dummies[NoOfJobs];

    // ----- enqueue the jobs:
    for ( int count  = 0; count < NoOfJobs; ++count ) {
        weaver.enqueueRaw(&dummies[count]);
    }

    weaver.finish();
}

#include "Construction.moc"

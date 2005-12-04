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

extern "C" {
#include <unistd.h>
}

#include <QCoreApplication>
#include <ThreadWeaver.h>
#include <Thread.h>
#include <DebuggingAids.h>
#include <QtDebug>

using namespace ThreadWeaver;

class DummyJob : public Job
{
public:
    DummyJob(QObject* parent = 0, int msec = 100)
        : Job ( parent ),
          m_wait ( msec )
        {
        }

    ~DummyJob() {}
protected:
    void run ()
        {
            debug (0, "DummyJob::run: doing it - sleeping %i milliseconds.\n",
                   m_wait );
            thread()->msleep( m_wait );
            debug (0, "DummyJob::run: done.\n" );
        }

    int m_wait;
};


int main ( int argc,  char** argv )
{
    const int NoOfThreads = 4 ;
    const int NoOfJobs = NoOfThreads * 4  ; // number of jobs for every thread
    const int Multiplier = 250; // [0..9]* Multiplier sleep time for each job
    ThreadWeaver::setDebugLevel ( true, 4);
    QCoreApplication app ( argc,  argv );
    ThreadWeaver::Weaver weaver ( 0, NoOfThreads );
    DummyJob *dummies[ NoOfJobs];

    // ----- create a number of dummy jobs:
    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        dummies[count] = new DummyJob (0, Multiplier * (count % 10) );
    }

    // ----- enqueue the jobs:
    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        weaver.enqueue( dummies[count] );
    }

    weaver.finish();

    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        delete dummies[count];
    }

    qDebug() << "main() exits here, the Weaver is destructed when it goes out"
             << " of scope " << endl
             << "... let's see if that works:" << endl;
    return 0;
}

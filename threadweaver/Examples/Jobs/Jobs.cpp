/* -*- C++ -*-

   This file implements the Jobs example.
   This example shows how to make a multithreaded GUI application.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: Jobs.cpp 30 2005-08-16 16:16:04Z mirko $
*/

extern "C" {
#include <stdlib.h>
}

#include <QWidget>
#include <QProgressBar>
#include <QApplication>
#include <QCheckBox>

#include <Jobs.h>
#include <Thread.h>
#include <DebuggingAids.h>

const int NoOfJobs = 100;

DummyJob::DummyJob (QObject* parent)
    : Job ( parent )
{
}

void DummyJob::run ()
{
    // the job will take (... sleep) a random number of msec between 1 and
    // 1000:
    thread()->msleep( 1+(int) ( 1000.0*rand () / (RAND_MAX+1.0) ) );
}

Jobs::Jobs ( QWidget *parent )
    : QWidget ( parent ),
      m_quit (false),
      weaver( new Weaver( this ) ),
      m_log ( 0 )
{
    ui.setupUi( this );

    connect ( weaver, SIGNAL ( finished() ), SLOT (slotStopped () ) );
    connect ( weaver, SIGNAL ( jobDone(Job*) ), SLOT (slotJobDone (Job*) ) );
    connect ( weaver, SIGNAL ( suspended() ), SLOT (slotStopped () ) );

    // create the jobs:
    for (int count = 0; count < NoOfJobs; ++count)
    {
        m_jobs.append (new DummyJob (this) );
    }

    connect ( ui.pbStart,  SIGNAL ( clicked() ),  SLOT ( slotStart() ) );
    connect ( ui.pbStop,  SIGNAL ( clicked() ),  SLOT ( slotStop() ) );
    connect ( ui.pbQuit,  SIGNAL ( clicked () ),  SLOT ( slotQuit() ) );
    connect ( ui.cbLog, SIGNAL (stateChanged ( int )),
	      SLOT (slotLogStateChanged ( int )));
    setState (Initial);
    ui.threadGrid->attach ( weaver );
}

Jobs::~Jobs()
{
    delete weaver;
}

void Jobs::slotLogStateChanged (int s)
{
    switch ((Qt::CheckState) s)
    {
	case Qt::Unchecked:
	    if (m_log)
	    {
		delete m_log;
		m_log = 0;
	    }
	    break;
	case Qt::Checked:
            if ( m_log == 0 )
            {
                m_log = new WeaverObserverTest ( this );
                weaver->registerObserver ( m_log );
            }
	case Qt::PartiallyChecked:
	default:
	    break;
    };
}

void Jobs::setState (State s)
{
    m_state = s;
    switch (m_state)
    {
	case ShuttingDown:
	    // case Disable:  (same)
	    // case Stopping: (same)
	    ui.pbStart->setEnabled ( false );
	    ui.pbStop->setEnabled ( false );
	    ui.pbQuit->setEnabled ( false );
	    break;
	case Started:
	    ui.pbStart->setEnabled ( false );
	    ui.pbStop->setEnabled ( true );
	    ui.pbQuit->setEnabled ( true );
	    break;
	case Initial:
	    // case Stopped: (same)
	default:
	    ui.pbStart->setEnabled ( true );
	    ui.pbStop->setEnabled ( false );
	    ui.pbQuit->setEnabled ( true );
	    ui.pbProgress->reset();
	    ui.lcdNumJobsRem->display (0);
	    break;
    };
}

Jobs::State Jobs::state ()
{
    return m_state;
}

void Jobs::slotStart()
{
    setState (Disable);
    ui.lcdNumJobsRem->display ( NoOfJobs );
    ui.pbProgress->setRange (0, NoOfJobs);
    weaver->enqueue (m_jobs);
    setState (Started);
}

void Jobs::slotStop ()
{
    weaver->suspend ();
}

void Jobs::slotStopped ()
{
    setState (Disable);
    weaver->dequeue();
    weaver->resume ();
    if (m_quit == true)
    {
	setState (ShuttingDown);
        close ();
    } else {
	setState (Stopped);
    }
}

void Jobs::slotQuit ()
{
    setState (Disable);
    m_quit = true;
    slotStop();
}

void Jobs::slotJobDone (Job*)
{
    ui.lcdNumJobsRem->display ( weaver->queueLength());

    ui.pbProgress->setValue ( NoOfJobs - weaver->queueLength() );
}

Weaver *weaver;

int main ( int argc, char ** argv)
{
    QApplication app ( argc, argv );
    ThreadWeaver::setDebugLevel( true, 1);
    Jobs jobs;

    jobs.show();

    return app.exec();
}

extern "C" {
#include <stdlib.h>
}

#include <qapplication.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qstring.h>
#include <qgroupbox.h>

#include <weaverlogger.h>
#include <weavervisualizer.h>

#include "gui.h"

DummyJob::DummyJob (QObject* parent, const char* name)
    :Job (parent, name)
{
}

void DummyJob::run ()
{
    // the job will take (... sleep) a random number of msec between 1 and
    // 1000:
    thread()->msleep( 1+(int) ( 1000.0*rand () / (RAND_MAX+1.0) ) );
}

GUI::GUI ( QWidget *parent, const char *name)
    : GUIBase ( parent, name ),
      weaver (this, 0, 8),
      m_quit (false)
{
    WeaverThreadLogger *logger = new WeaverThreadLogger (this);
    logger->attach (&weaver);
    WeaverThreadVisualizer *vis = new WeaverThreadVisualizer (this);
    vis->attach (&weaver);

    connect ( &weaver, SIGNAL ( finished() ), SLOT (slotStopped () ) );
    connect ( &weaver, SIGNAL ( jobDone(Job*) ), SLOT (slotJobDone (Job*) ) );
    connect ( &weaver, SIGNAL ( suspended() ), SLOT (slotStopped () ) );

    // create the jobs:
    for (int count = 0; count < NoOfJobs; ++count)
    {
        jobs.append (new DummyJob (this) );
    }

    // create the status leds
//    QHBoxLayout *layout = new QHBoxLayout (frameThrStatus);
//     for (int count = 0; count < weaver.threads(); ++count)
//     {
//         layout->add (new ThreadStatusLED (frameThrStatus ) );
//     }
}

GUI::~GUI()
{
}

void GUI::slotStart()
{
    pbStart->setEnabled ( false );

    tlNumJobsRem->setText ( QString().setNum (NoOfJobs) );

    weaver.enqueue (jobs);

    pbProgress->setProgress (0, NoOfJobs);
    pbStop->setEnabled ( true );
}

void GUI::slotStop ()
{
    weaver.suspend (true);
}

void GUI::slotStopped ()
{
    pbStop->setEnabled ( false );

    // in case we got stopped before all jobs where done:
    weaver.dequeue ();
    // no finish() needed, since we did receive "Suspended"
    weaver.suspend (false);

    pbProgress->setProgress (0, NoOfJobs);

    if (m_quit == true)
    {
        close ();
    } else {
        pbStart->setEnabled ( true );
    }
}

void GUI::slotQuit ()
{
    m_quit = true;

    weaver.suspend (true);
}

void GUI::slotJobDone (Job*)
{
    tlNumJobsRem->setText ( QString().setNum (weaver.queueLength() ) );

    pbProgress->setProgress ( NoOfJobs - weaver.queueLength() );
}

int main ( int argc, char ** argv)
{
    QApplication app ( argc, argv );
    GUI gui;


    app.setMainWidget (&gui);

    gui.show();

    app.exec();

    return 0;
}

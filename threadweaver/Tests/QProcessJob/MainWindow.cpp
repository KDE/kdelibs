#include "MainWindow.h"
#include "QProcessJob.h"

#include <ThreadWeaver.h>

using namespace ThreadWeaver;

MainWindow::MainWindow ( QWidget * parent )
    : QMainWindow ( parent )
{
    ui.setupUi ( this );
    connect ( Weaver::instance(),  SIGNAL ( jobDone ( Job* ) ),
              SLOT ( update ( Job* ) ) );
}

void MainWindow::on_pushButtonStart_clicked ()
{
    ui.pushButtonStart->setEnabled( false );
    Weaver::instance()->enqueue ( new QProcessJob ( this ) );
}

void MainWindow::on_pushButtonQuit_clicked ()
{
    ui.pushButtonStart->setEnabled( false );
    ui.pushButtonQuit->setEnabled( false );
    Weaver::instance()->finish();
    QApplication::instance()->quit();
}

void MainWindow::update( Job *j )
{
    qDebug ( "MainWindow::update: job finished." );
    QProcessJob* job = dynamic_cast<QProcessJob*> ( j );

    if ( job )
    {
        ui.labelTime->setText ( job->result() );
    } else {
        qDebug ( "I do not know anything about this kind of jobs." );
    }
    ui.pushButtonStart->setEnabled( true );
    ui.pushButtonQuit->setEnabled( true );
}






#include <qtimer.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kstatusbar.h>
#include <kmenubar.h>

#include "kmainwindowtest.h"

MainWindow::MainWindow()
{
    QTimer::singleShot( 2*1000, this, SLOT( showMessage() ) );

    setCentralWidget( new QLabel( "foo", this ) );

    menuBar()->insertItem( "hi" );
}

void MainWindow::showMessage()
{
    statusBar()->show();
    statusBar()->message( "test" );
}

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "kmainwindowtest" );

    MainWindow mw;

    mw.show();

    return app.exec();
}

#include "kmainwindowtest.moc"

/* vim: et sw=4 ts=4
 */

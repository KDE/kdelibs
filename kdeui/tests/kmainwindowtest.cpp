#include <qtimer.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
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
    KCmdLineArgs::init( argc, argv, "kmainwindowtest", "KMainWindowTest", "kmainwindow test app", "1.0" );
    KApplication app;

    MainWindow mw;

    mw.show();

    return app.exec();
}

#include "kmainwindowtest.moc"

/* vim: et sw=4 ts=4
 */

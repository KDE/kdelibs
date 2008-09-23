#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "MainWindow.h"

int main(int argc, char* argv[])
{
    KAboutData aboutData( "kformulapp", 0,
                          ki18n("Sample KFormula application"), "0.1",
                          ki18n("Application showing a KFormula widget for debugging purposes"),
                          KAboutData::License_GPL,
                          ki18n("Copyright 2008 Alfredo Beaumont <alfredo.beaumont@gmail.com>") );
    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication app;

    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}

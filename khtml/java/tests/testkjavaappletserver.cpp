#include <QDebug>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <qstring.h>
#include <stdio.h>
#include <unistd.h>

#include "java/kjavaappletserver.h"
#include "java/kjavaapplet.h"
#include "java/kjavaappletcontext.h"
#include "java/kjavaappletwidget.h"

static KCmdLineOptions options[] =
{
    { "+kdelibspath", "path to kdelibs directory", 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "testKJASSever", "testKJASServer", "test program", "0.0" );

    KCmdLineArgs::addCmdLineOptions( options );
    //KCmdLineArgs::addStdCmdLineOptions();

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QByteArray path;
#if 0
    path = args->getOption("kdelibspath");
    if (path.isEmpty())
    {
      kdWarning() << "you need to specify a path to your kdelibs source dir, see \"--help\"" << endl;
      return -1;
    }
#else
    #ifdef __GNUC__
    #warning better adjust this :)
    #endif
    path = "/home/danimo/src/kde/trunk/KDE/kdelibs/";
#endif
    QString testpath = "file://" + path + "/kdelibs/khtml/test/";

    KJavaAppletContext *context = new KJavaAppletContext;
    KJavaAppletWidget *a = new KJavaAppletWidget;
    a->applet()->setAppletContext(context);

    a->show();

//    c->registerApplet(a->applet());

    a->applet()->setBaseURL( testpath );
    a->applet()->setAppletName( "Lake" );
    a->applet()->setAppletClass( "lake.class" );
    a->applet()->setParameter( "image", "konqi.gif" );

    a->showApplet();
    a->applet()->start();

    app.exec();
}

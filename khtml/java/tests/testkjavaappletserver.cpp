#include <QtCore/QDebug>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <QtCore/QString>
#include <stdio.h>
#include <unistd.h>

#include "java/kjavaappletserver.h"
#include "java/kjavaapplet.h"
#include "java/kjavaappletcontext.h"
#include "java/kjavaappletwidget.h"

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "testKJASSever", 0, ki18n("testKJASServer"), "0.0", ki18n("test program"));

    KCmdLineOptions options;
    options.add("+kdelibspath", ki18n("path to kdelibs directory"));

    KCmdLineArgs::addCmdLineOptions( options );
    //KCmdLineArgs::addStdCmdLineOptions();

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QByteArray path;
#if 0
    path = args->getOption("kdelibspath");
    if (path.isEmpty())
    {
      kWarning() << "you need to specify a path to your kdelibs source dir, see \"--help\"";
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

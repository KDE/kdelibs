#include <kapp.h>
#include <kcmdlineargs.h>
#include <kjavaappletserver.h>
#include <kjavaapplet.h>
#include <kjavaappletwidget.h>
#include <qstring.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "testKJASSever", "test program", "0.0" );
    KApplication app;

    QString path_to_kdelibs( argv[1] );
    
    KJavaAppletContext* context = new KJavaAppletContext();
    KJavaAppletWidget *applet = new KJavaAppletWidget( context );

    applet->show();

    applet->setBaseURL( "file:" + path_to_kdelibs + "/kdelibs/khtml/test/" );
    applet->setAppletName( "Lake" );
    applet->setAppletClass( "lake.class" );
    applet->setParameter( "image", "konqi.gif" );

    applet->create();
    applet->showApplet();
    applet->start();

    app.exec();
}

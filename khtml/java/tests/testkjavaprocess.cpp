#include <kapp.h>
#include <kjavaprocess.h>
#include <kcmdlineargs.h>
#include <kstddirs.h>

#include <qstring.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
    KCmdLineArgs::init( argc, argv, "testKJASProcess", "test Program", "0.0" );
    KApplication app;

    QString kjas_classpath = locate( "data", "kjava/kjava-classes.zip" );
    QCString classpath = getenv( "CLASSPATH" );

    if( classpath.isEmpty() )
    {
      classpath += "CLASSPATH=";
      classpath += kjas_classpath.local8Bit();
    }
    else
    {
      classpath += ":";
      classpath += kjas_classpath.local8Bit();
    }

    putenv( qstrdup(classpath) );


    KJavaProcess *kjp = new KJavaProcess();
    kjp->setMainClass( "org.kde.kjas.server.Main" );
    kjp->startJava();

    QString s;

    s = "createContext!test\n";
    kjp->send( s );

    s = "createApplet!test!lake_applet!Lake!lake.class!file:/build/kde-src/kdelibs/khtml/test/java.html!file:/build/kde-src/kdelibs/khtml/test/!null!240!630\n";
    kjp->send( s );

    s = "setParameter!test!lake_applet!image!konqi.gif\n";
    kjp->send( s );

    s = "showApplet!test!lake_applet!The Test Applet\n";
    kjp->send( s );


    app.exec();
}

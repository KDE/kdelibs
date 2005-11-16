#include "KIDLTest.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <dcopclient.h>

KIDLTest::KIDLTest( const DCOPCString& id )
    : DCOPObject( id )
{
}

QString KIDLTest::hello( const QString& name )
{
    qDebug("Du heter %s", name.latin1() );
    qDebug("Ha det %s", name.latin1() );
	
    return QString("Jeg heter KIDLTest");
}

int main( int argc, char** argv )
{
    KAboutData about("kidlservertest", "kidlservertest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app(false);

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "kidlservertest", false );

    qDebug("Server process started...");

    (void) new KIDLTest( "Hello" );

    qDebug("Server listening ...");

    return app.exec();
}

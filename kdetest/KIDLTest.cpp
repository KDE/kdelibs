#include "KIDLTest.h"

#include <kapp.h>
#include <dcopclient.h>

KIDLTest::KIDLTest( const QCString& id )
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
    KApplication app( argc, argv, "kidlservertest" );

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "kidlservertest" );

    qDebug("Server process started...");

    KIDLTest* t = new KIDLTest( "Hello" );

    qDebug("Server listening ...");

    return app.exec();
}

#include "KIDLTest.h"

#include <kapp.h>

QString KIDLTest::hello( const QString& name )
{
    qDebug("Du heter %s", name.latin1() );
    qDebug("Ha det %s", name.latin1() );
	
    return QString("Jeg heter KIDLTest");
}

int main( int argc, char** argv )
{
    KApplication app( argc, argv, "kidlservertest" );

    qDebug("Server process started...");

    app.dcopClient();
    KIDLTest* t = new KIDLTest( "Hello" );
    
    qDebug("Server listening ...");
    
    return app.exec();
}

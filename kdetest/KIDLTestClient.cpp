#include <kapp.h>
#include <dcopclient.h>

#include "KIDLTest_stub.h"

int main( int argc, char** argv )
{
    KApplication app( argc, argv, "KIDLTestClient" );

    kapp->dcopClient()->attach();
    // kapp->dcopClient()->registerAs( "kidlclienttest" );

    KIDLTest_stub* t = new KIDLTest_stub( "kidlservertest", "Hello" );

    QString ret = t->hello("Torben");
    qDebug("Server says: %s", ret.latin1() );
}

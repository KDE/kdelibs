#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <dcopclient.h>

#include "KIDLTest_stub.h"

int main( int argc, char** argv )
{
    KAboutData about("KIDLTestClient", "KIDLTestClient", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app(false,false);


    KApplication::dcopClient()->attach();
    // KApplication::dcopClient()->registerAs( "kidlclienttest" );

    KIDLTest_stub* t = new KIDLTest_stub( "kidlservertest", "Hello" );

    QString ret = t->hello("Torben");
    qDebug("Server says: %s", ret.latin1() );
}

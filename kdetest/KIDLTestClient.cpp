#include <kapp.h>

#include "KIDLTest_stub.h"

int main( int argc, char** argv )
{
    KApplication app( argc, argv );
    
    KIDLTest_stub* t = new KIDLTest_stub( "kidlservertest", "Hello" );
    
    QString ret = t->hello("Torben");
    qDebug("Server says: %s", ret.latin1() );
}

#include <kapp.h>
#include <klocale.h>
#include "kcharselect.h"

int main (int argc,char **argv)
{
    KApplication app( argc, argv, "kcharselecttest" );

    KCharSelect selector( 0, "char selector" );
    selector.resize( selector.sizeHint() );
    selector.show();
    selector.setCaption( "KCharSelect Test" );

    app.setMainWidget( &selector );
    return app.exec();
}

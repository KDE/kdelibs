#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>
#include "kcharselect.h"

int main (int argc,char **argv)
{
    KAboutData about("kcharselecttest", "kcharselecttest", "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    KCharSelect selector( 0, "char selector" );
    selector.resize( selector.sizeHint() );
    selector.show();
    selector.setWindowTitle( "KCharSelect Test" );

    return app.exec();
}

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>
#include "kcharselect.h"

int main (int argc,char **argv)
{
    KAboutData about("kcharselecttest", 0, ki18n("kcharselecttest"), "version");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    KCharSelect selector( 0 );
    selector.resize( selector.sizeHint() );
    selector.show();
    selector.setWindowTitle( "KCharSelect Test" );

    return app.exec();
}

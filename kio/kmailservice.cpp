#include <kapp.h>
#include <klocale.h>
#include <kcmdlineargs.h>

static const KCmdLineOptions options[] =
{
    { "+url", 0, 0 },
    { 0, 0, 0 }
};

int main( int argc, char **argv )
{
    KLocale::setMainCatalogue("kdelibs");
    KCmdLineArgs::init( argc, argv, "kmailservice", I18N_NOOP("mail service"), "unknown" );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() != 1 )
        return 1;

    a.invokeMailer(KURL(args->arg(0)));

    return 0;
}

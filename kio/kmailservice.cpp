#include <kapp.h>
#include <kcmdlineargs.h>

static const KCmdLineOptions options[] =
{
    { "+url", 0, 0 },
    { 0, 0, 0 }
};

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kmailservice", "mail service", "unknown" );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() != 1 )
        return 1;

    a.invokeMailer(KURL(args->arg(0)));

    return 0;
}

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

    KURL u = args->arg( 0 );

    QString addr = u.path();
    QString subj;

    if (u.query().left(9) == "?subject=")
    {
       subj = KURL::decode_string( u.query().mid(9) );
    }

    a.invokeMailer( addr, subj );
    return 0;
}

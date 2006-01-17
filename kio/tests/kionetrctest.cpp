#include <kapplication.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "authinfo.h"

void output( const KUrl& u )
{
    kdDebug() << "Looking up auto login for: " << u.url() << endl;
    KIO::NetRC::AutoLogin l;
    bool result = KIO::NetRC::self()->lookup( u, l, true );
    if ( !result )
    {
       kdDebug() << "Either no .netrc and/or .kionetrc file was "
                    "found or there was problem when attempting to "
                    "read from them!  Please make sure either or both "
                    "of the above files exist and have the correct "
                    "permission, i.e. a regular file owned by you with "
                    "with a read/write permission (0600)" << endl;
       return;
    }

    kdDebug() << "Type: " << l.type << endl
              << "Machine: " << l.machine << endl
              << "Login: " << l.login << endl
              << "Password: " << l.password << endl;

    QMap<QString,QStringList>::ConstIterator it = l.macdef.begin();
    for ( ; it != l.macdef.end(); ++it )
    {
        kdDebug() << "Macro: " << it.key() << "= "
                  << it.data().join("   ") << endl;
    }
}

int main(int argc, char **argv)
{
    const char *version = "0.5";
    const char *description = "Unit test for .netrc and kionetrc parser.";
    KCmdLineOptions options[] =
    {
        { "+command", "[url1,url2 ,...]", 0 },
        KCmdLineLastOption
    };

    KCmdLineArgs::init( argc, argv, "kionetrctest", "KIO-netrc-test", description, version );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int count = args->count();
    KApplication app;

    if ( !count )
        args->usage();
    else
    {
        KUrl u;
        for( int i=0 ; i < count; i++ )
        {
            u = args->arg(i);
            if ( !u.isValid() )
            {
              kdDebug() << u.url() << " is invalid! Ignoring..." << endl;
              continue;
            }
            output( u );
        }
    }
    args->clear();
    return 0;
}

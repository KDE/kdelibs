#include <kapplication.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "authinfo.h"

void output( const QUrl& u )
{
    kDebug() << "Looking up auto login for: " << u;
    KIO::NetRC::AutoLogin l;
    bool result = KIO::NetRC::self()->lookup( u, l, true );
    if ( !result )
    {
       kDebug() << "Either no .netrc and/or .kionetrc file was "
                    "found or there was problem when attempting to "
                    "read from them!  Please make sure either or both "
                    "of the above files exist and have the correct "
                    "permission, i.e. a regular file owned by you with "
                    "with a read/write permission (0600)" << endl;
       return;
    }

    kDebug() << "Type: " << l.type << endl
              << "Machine: " << l.machine << endl
              << "Login: " << l.login << endl
              << "Password: " << l.password << endl;

    QMap<QString,QStringList>::ConstIterator it = l.macdef.constBegin();
    for ( ; it != l.macdef.constEnd(); ++it )
    {
        kDebug() << "Macro: " << it.key() << "= "
                  << it.value().join("   ") << endl;
    }
}

int main(int argc, char **argv)
{
    const char *version = "0.5";
    KCmdLineOptions options;
    options.add("+command", qi18n("[url1,url2 ,...]"));

    KCmdLineArgs::init( argc, argv, "kionetrctest", 0, qi18n("KIO-netrc-test"), version, qi18n("Unit test for .netrc and kionetrc parser."));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    int count = args->count();
    KApplication app;

    if ( !count )
        args->usage();
    else
    {
        QUrl u;
        for( int i=0 ; i < count; i++ )
        {
            u = QUrl::fromUserInput(args->arg(i));
            if (!u.isValid()) {
              kDebug() << u << "is invalid! Ignoring...";
              continue;
            }
            output( u );
        }
    }
    args->clear();
    return 0;
}

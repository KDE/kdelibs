
#include <iostream>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kurifilter.h"

void filter( const char* u, QStringList list = QStringList() )
{
    QString a = QString::fromLatin1( u );
    kdDebug() << "***********************************\nFiltering: " << a << endl;
    if (KURIFilter::self()->filterURI(a, list))
        kdDebug() << "After filtering: " << a << "\n***********************************\n" << endl;
    else
        kdDebug() << "No filtering required\n***********************************\n" << endl;
}

int main(int argc, char **argv) {
    KAboutData aboutData("kurifiltertest", "KURIFilter Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    // URI that should require no filtering
    filter( "http://www.kde.org" );

    // ShortURI tests
    filter( "linuxtoday.com" );
    filter( "LINUXTODAY.COM" );
    filter( "kde.org" );
    filter( "mosfet.org" );
    filter( "~/.kde", "ShortURIFilter" );

    // SMB share test with a specific filter chosen
    filter( "\\\\THUNDER\\", "ShortURIFilter" );
    filter( "smb://", "ShortURIFilter" );
    filter( "smb://THUNDER\\WORKGROUP", "ShortURIFilter" );
    filter( "smb:/THUNDER/WORKGROUP", "ShortURIFilter" );   
    filter( "smb:///", "ShortURIFilter" ); // use specific filter.
    filter( "smb:", "ShortURIFilter" ); // use specific filter.
    filter( "smb:/", "ShortURIFilter" ); // use specific filter.
   
    // Executable tests
    filter( "kppp", "ShortURIFilter" );
   
    // IKWS test
    filter( "KDE" );
    filter( "GNOME" );
   
    // ENVIRONMENT variable
    filter( "$KDEDIR/kdelibs/kio" );
    filter( "$HOME/.kde/share" );
    filter( "$HOME/$KDEDIR/kdebase/kconotrol/ebrowsing" );
    filter( "$1/$2/$3" );  // can be used as bogus or valid test
    filter( "$$$$" ); // worst case scenarios.
    filter( "$QTDIR", "ShortURIFilter" ); //use specific filter. 
    filter( "$KDEDIR", "ShortURIFilter" ); //use specific filter.
   
    return 0;
}


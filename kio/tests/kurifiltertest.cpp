
#include <iostream>

#include <kaboutdata.h>
#include <kapp.h>
#include <kcmdlineargs.h>

#include "kurifilter.h"

void filter( const char* u, QStringList list = QStringList() )
{
    QString a = QString::fromLatin1( u );
    cout << "***********************************\nFiltering: " << a << endl;
    if (KURIFilter::self()->filterURI(a, list))
        cout << "After filtering: " << a << "\n***********************************\n" << endl;
    else
        cout << "No filtering required\n***********************************\n" << endl;
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
    filter( "kde.org" );
    filter( "mosfet.org" );

    // SMB share test with a specific filter chosen
    filter( "\\\\THUNDER\\", "ShortURIFilter" );
    filter( "smb:///", "ShortURIFilter" ); // use specific filter.
    filter( "smb:", "ShortURIFilter" ); // use specific filter.
    filter( "smb:/", "ShortURIFilter" ); // use specific filter.

    // IKWS test
    filter( "KDE" );
    filter( "GNOME" );
    return 0;
}


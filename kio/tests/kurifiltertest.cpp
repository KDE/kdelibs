
#include <iostream>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kurifilter.h"

void filter( const char* u, QStringList list = QStringList(), const char * expectedResult = 0 )
{
    QString a = QString::fromLatin1( u );
    KURIFilterData * m_filterData = new KURIFilterData;
    m_filterData->setData( a );
    kdDebug() << "Filtering: " << a << endl;
    if (KURIFilter::self()->filterURI(*m_filterData, list))
    {
        switch( m_filterData->uriType() )
            {
                case KURIFilterData::LOCAL_FILE:
                case KURIFilterData::LOCAL_DIR:
                case KURIFilterData::NET_PROTOCOL:
                case KURIFilterData::HELP:
                    kdDebug() << "Here, minicli would run KRun on " << m_filterData->uri().url() << endl;
                    break;
                case KURIFilterData::EXECUTABLE:
                case KURIFilterData::SHELL:
                    kdDebug() << "Executable. cmd=" << m_filterData->uri().url() << endl;
                    break;
                case KURIFilterData::ERROR:
                    kdDebug() << "Error" << endl;
                    break;
                default:
                    kdDebug() << "Not found" << endl;
        }
    }
    else
        kdDebug() << "No filtering required" << endl;
    kdDebug() << "-----" << endl;
    if ( expectedResult )
        if ( m_filterData->uri().url() != QString::fromLatin1( expectedResult ) )
            kdWarning() << " Got " << m_filterData->uri().url() << " expected " << expectedResult << endl;
    delete m_filterData;
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
    filter( "kppp", "ShortURIFilter", "kppp" );
    filter( "xemacs", QStringList(), "xemacs" );

    // IKWS test
    filter( "KDE" );
    filter( "GNOME" );

    // ENVIRONMENT variable
    filter( "$KDEDIR/kdelibs/kio" ); // note: this dir doesn't exist...
    filter( "$KDEDIR/include" );
    filter( "$HOME/.kde/share" );
    filter( "$HOME/$KDEDIR/kdebase/kcontrol/ebrowsing" );
    filter( "$1/$2/$3" );  // can be used as bogus or valid test
    filter( "$$$$" ); // worst case scenarios.
    filter( "$QTDIR", "ShortURIFilter" ); //use specific filter.
    filter( "$KDEDIR", "ShortURIFilter" ); //use specific filter.

    return 0;
}


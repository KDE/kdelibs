
#include <iostream>
#include <stdlib.h>

#include <kaboutdata.h>
#include <kapp.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include "kurifilter.h"
#include <qdir.h>

void filter( const char* u, const char * expectedResult = 0, QStringList list = QStringList() )
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
            kdFatal() << " Got " << m_filterData->uri().url() << " expected " << expectedResult << endl;
    delete m_filterData;
}

int main(int argc, char **argv) {
    KAboutData aboutData("kurifiltertest", "KURIFilter Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QStringList minicliFilters;
    minicliFilters << "kshorturifilter" << "kurisearchfilter";

    // URI that should require no filtering
    filter( "http://www.kde.org", "http://www.kde.org" );

    // ShortURI tests
    filter( "linuxtoday.com", "http://linuxtoday.com" );
    filter( "LINUXTODAY.COM", "http://LINUXTODAY.COM" );
    filter( "kde.org", "http://kde.org" );
    filter( "mosfet.org", "http://mosfet.org" );
    filter( "/", "file:/" );
    filter( "/", "file:/", "kshorturifilter" );
    filter( "~/.kde", QCString("file:")+QDir::homeDirPath().local8Bit()+"/.kde", "kshorturifilter" );

    // SMB share test with a specific filter chosen
    filter( "\\\\THUNDER\\", "smb:/THUNDER/", "kshorturifilter" );
    filter( "smb://", "smb:/", "kshorturifilter" );
    filter( "smb://THUNDER\\WORKGROUP", "smb:/THUNDER%5CWORKGROUP", "kshorturifilter" );
    filter( "smb:/THUNDER/WORKGROUP", "smb:/THUNDER/WORKGROUP", "kshorturifilter" );
    filter( "smb:///", "smb:/", "kshorturifilter" ); // use specific filter.
    filter( "smb:", "smb:/", "kshorturifilter" ); // use specific filter.
    filter( "smb:/", "smb:/", "kshorturifilter" ); // use specific filter.

    // IKWS test
    filter( "KDE", "http://navigation.realnames.com/resolver.dll?realname=KDE&charset=utf-8&providerid=180" );
    filter( "GNOME", "http://navigation.realnames.com/resolver.dll?realname=GNOME&charset=utf-8&providerid=180" );

    // Executable tests
    filter( "kppp", "kppp", minicliFilters );
    filter( "xemacs", "xemacs", minicliFilters );

    // No IKWS in minicli
    filter( "KDE", "KDE", minicliFilters );
    filter( "I/dont/exist", "I/dont/exist", minicliFilters );

    // ENVIRONMENT variable
    filter( "$KDEDIR/kdelibs/kio" ); // note: this dir doesn't exist...

    // and this currently launches realnames, which is wrong IMHO. Maybe
    // we should prevent realnames from happening when there is a '/' ?
    //
    // Now "Realnames" will should only get invoked if and only if
    // $KDEDIR has not already been set.  Otherwise, the shortURI filter
    // plugin will consume it, evenif the environment variable does not
    // exist. (DA)
    filter( "$KDEDIR/include" );
    filter( "$HOME/.kde/share" );
    filter( "$HOME/$KDEDIR/kdebase/kcontrol/ebrowsing" );
    filter( "$1/$2/$3" );  // can be used as bogus or valid test
    filter( "$$$$" ); // worst case scenarios.

    // Replaced the match testing with a 0 since
    // the shortURI filter will return the string
    // itself if the requested environment variable
    // is not already set.
    filter( "$QTDIR", 0, "kshorturifilter" ); //use specific filter.
    filter( "$KDEDIR", 0, "kshorturifilter" ); //use specific filter.

    // Search Engine tests
    filter( "gg:foo bar" );
    filter( "ya:foo bar was here" );

    return 0;
}


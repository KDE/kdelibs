
#include <iostream>
#include <stdlib.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>

#include "kurifilter.h"
#include <qdir.h>

static const char * const s_uritypes[] = { "NET_PROTOCOL", "LOCAL_FILE", "LOCAL_DIR", "EXECUTABLE", "HELP", "SHELL", "BLOCKED", "ERROR", "UNKNOWN" };
#define NO_FILTERING -2

void filter( const char* u, const char * expectedResult = 0, int expectedUriType = -1, QStringList list = QStringList(), const char * abs_path = 0 )
{
    QString a = QString::fromUtf8( u );
    KURIFilterData * m_filterData = new KURIFilterData;
    m_filterData->setData( a );
    if( abs_path ) {
        m_filterData->setAbsolutePath( QString::fromLatin1( abs_path ) );
        kdDebug() << "Filtering: " << a << " with abs_path=" << abs_path << endl;
   }  else
        kdDebug() << "Filtering: " << a << endl;
    QString cmd;
    if (KURIFilter::self()->filterURI(*m_filterData, list))
    {
        // From minicli
        if ( m_filterData->uri().isLocalFile() && !m_filterData->uri().hasRef() )
            cmd = m_filterData->uri().path();
        else
            cmd = m_filterData->uri().url();
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
                    if( m_filterData->hasArgsAndOptions() )
                        cmd += m_filterData->argsAndOptions();
                    kdDebug() << "Executable - minicli would run cmd=" << cmd << endl;
                    break;
                case KURIFilterData::ERROR:
                    kdDebug() << "Error" << endl;
                    break;
                default:
                    kdDebug() << "Not found" << endl;
        }
        if ( expectedUriType != -1 && expectedUriType != m_filterData->uriType() )
        {
            kdError() << " Got uri type " << s_uritypes[m_filterData->uriType()] << " expected " << s_uritypes[expectedUriType] << endl;
            ::exit(1);
        }
        if ( expectedResult )
            if ( cmd != QString::fromLatin1( expectedResult ) )
            {
                kdError() << " Got " << cmd << " expected " << expectedResult << endl;
                ::exit(1);
            }
    }
    else
    {
        kdDebug() << "No filtering required" << endl;
        if ( expectedUriType != NO_FILTERING )
        {
            kdError() << " Expected uri type " << s_uritypes[expectedUriType] << endl;
            ::exit(1);
        }
    }
    delete m_filterData;
    kdDebug() << "-----" << endl;
}

int main(int argc, char **argv) {
    KAboutData aboutData("kurifiltertest", "KURIFilter Test",
                        "1.0");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QStringList minicliFilters;
    minicliFilters << "kshorturifilter" << "kurisearchfilter" << "localdomainurifilter";

    // URI that should require no filtering
    filter( "http://www.kde.org", "http://www.kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "http://www.kde.org/developer//index.html", "http://www.kde.org/developer//index.html", KURIFilterData::NET_PROTOCOL );
        // URL with reference
    filter( "http://www.kde.org/index.html#q8", "http://www.kde.org/index.html#q8", KURIFilterData::NET_PROTOCOL );
        // local file with reference
    filter( "file:/etc/passwd#q8", "file:/etc/passwd#q8", KURIFilterData::LOCAL_FILE );
        // hostnames are lowercased by KURL
    filter( "http://www.myDomain.commyPort/ViewObjectRes//Default:name=hello",
            "http://www.mydomain.commyport/ViewObjectRes//Default:name=hello", KURIFilterData::NET_PROTOCOL);
    filter( "ftp://ftp.kde.org", "ftp://ftp.kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "ftp://username@ftp.kde.org:500", "ftp://username@ftp.kde.org:500", KURIFilterData::NET_PROTOCOL );

    // ShortURI tests
    filter( "linuxtoday.com", "http://linuxtoday.com", KURIFilterData::NET_PROTOCOL );
    filter( "LINUXTODAY.COM", "http://linuxtoday.com", KURIFilterData::NET_PROTOCOL );
    filter( "kde.org", "http://kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "mosfet.org", "http://mosfet.org", KURIFilterData::NET_PROTOCOL );
    filter( "/", "/", KURIFilterData::LOCAL_DIR );
    filter( "/", "/", KURIFilterData::LOCAL_DIR, "kshorturifilter" );
    filter( "~/.kderc", QDir::homeDirPath().local8Bit()+"/.kderc", KURIFilterData::LOCAL_FILE, "kshorturifilter" );
    filter( "~", QDir::homeDirPath().local8Bit(), KURIFilterData::LOCAL_DIR, "kshorturifilter", "/tmp" );
    filter( "~foobar", 0, KURIFilterData::ERROR, "kshorturifilter" );

    // SMB share test with a specific filter chosen
    // #### Those fail - I see no specific code for them in kshorturifilter !?
#if 0
    filter( "\\\\THUNDER\\", "smb:/THUNDER/", "kshorturifilter" );
    filter( "smb://", "smb:/", "kshorturifilter" );
    filter( "smb://THUNDER\\WORKGROUP", "smb:/THUNDER%5CWORKGROUP", "kshorturifilter" );
    filter( "smb:/THUNDER/WORKGROUP", "smb:/THUNDER/WORKGROUP", "kshorturifilter" );
    filter( "smb:///", "smb:/", "kshorturifilter" ); // use specific filter.
    filter( "smb:", "smb:/", "kshorturifilter" ); // use specific filter.
    filter( "smb:/", "smb:/", "kshorturifilter" ); // use specific filter.
#endif

    // IKWS test
    filter( "KDE", "http://www.google.com/search?q=KDE&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "FTP", "http://www.google.com/search?q=FTP&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    // Typing 'ls' in konq's location bar should go to google for ls too. But the short-uri filter catches it first :}
    // Solutions: a kurifilter flag for "I accept executables", or splitting the executable lookup out of kshorturifilter
    // (and omitting that filter from e.g. Konqueror)
    //filter( "ls", "http://www.google.com/search?q=ls&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );

    // Executable tests - No IKWS in minicli
    filter( "kppp", "kppp", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "xemacs", "xemacs", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "KDE", "KDE", NO_FILTERING, minicliFilters );
    filter( "I/dont/exist", "I/dont/exist", NO_FILTERING, minicliFilters );
    filter( "/I/dont/exist", 0, KURIFilterData::ERROR, minicliFilters );
    filter( "/I/dont/exist#a", 0, KURIFilterData::ERROR, minicliFilters );
    filter( "kppp --help", "kppp --help", KURIFilterData::EXECUTABLE, minicliFilters ); // the args are in argsAndOptions()
    filter( "/usr/bin/gs", "/usr/bin/gs", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "/usr/bin/gs -q -option arg1", "/usr/bin/gs -q -option arg1", KURIFilterData::EXECUTABLE, minicliFilters ); // the args are in argsAndOptions()

    // ENVIRONMENT variable
    setenv( "SOMEVAR", "/somevar", 0 );
    setenv( "ETC", "/etc", 0 );
    QCString qtdir=getenv("QTDIR");
    filter( "$SOMEVAR/kdelibs/kio", 0, KURIFilterData::ERROR ); // note: this dir doesn't exist...
    filter( "$ETC/passwd", "/etc/passwd", KURIFilterData::LOCAL_FILE );
    filter( "$QTDIR/doc/html/functions.html#s", QCString("file:")+qtdir+"/doc/html/functions.html#s", KURIFilterData::LOCAL_FILE );
    filter( "http://www.kde.org/$USER", "http://www.kde.org/$USER", KURIFilterData::NET_PROTOCOL ); // no expansion

    QCString home = getenv("HOME");
    filter( "$HOME/.kde/share", home+"/.kde/share", KURIFilterData::LOCAL_DIR );
    KStandardDirs::makeDir( "/tmp/a+plus" );
    filter( "/tmp/a+plus", "/tmp/a+plus", KURIFilterData::LOCAL_DIR );
    // BR 27788 - note that you need this dir to exist for this test to work
    filter( "$HOME/.kde/share/apps/kword/templates/Text oriented", home+"/.kde/share/apps/kword/templates/Text oriented", KURIFilterData::LOCAL_DIR );
    filter( "$HOME/$KDEDIR/kdebase/kcontrol/ebrowsing", 0, KURIFilterData::ERROR );
    filter( "$1/$2/$3", "http://www.google.com/search?q=$1/$2/$3&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );  // can be used as bogus or valid test. Currently triggers default search, i.e. google
    filter( "$$$$", "http://www.google.com/search?q=$$$$&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL ); // worst case scenarios.

    // Replaced the match testing with a 0 since
    // the shortURI filter will return the string
    // itself if the requested environment variable
    // is not already set.
    filter( "$QTDIR", 0, KURIFilterData::LOCAL_DIR, "kshorturifilter" ); //use specific filter.
    filter( "$KDEDIR", 0, KURIFilterData::LOCAL_DIR, "kshorturifilter" ); //use specific filter.

    // Search Engine tests
    filter( "gg:foo bar", "http://www.google.com/search?q=foo%20bar&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    // BR ...
    filter( "gg:C++", "http://www.google.com/search?q=C%2B%2B&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "ya:foo bar was here", 0, -1 ); // this triggers default search, i.e. google
    filter( "gg:www.kde.org", "http://www.google.com/search?q=www.kde.org&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "av:+rock +sample", "http://www.altavista.com/cgi-bin/query?pg=q&kl=XX&stype=stext&q=%2Brock%20%2Bsample", KURIFilterData::NET_PROTOCOL );
    filter( "gg:é" /*eaccent in utf8*/, "http://www.google.com/search?q=%C3%A9&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "gg:прйвет" /* greetings in russian utf-8*/, "http://www.google.com/search?q=%D0%BF%D1%80%D0%B9%D0%B2%D0%B5%D1%82&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );

    // Absolute Path tests for kshorturifilter
    filter( "./", home+"/.kde/share", KURIFilterData::LOCAL_DIR, "kshorturifilter", home+"/.kde/share/" ); // cleanDirPath removes the trailing slash
    filter( "../", home+"/.kde", KURIFilterData::LOCAL_DIR, "kshorturifilter", home+"/.kde/share" );
    filter( "apps", home+"/.kde/share/apps", KURIFilterData::LOCAL_DIR, "kshorturifilter", home+"/.kde/share" );
    // This test is against the specification of setAbsolutePath !!
    // It says a _PATH_ must be used!
    // kshorturifilter had code for this, but it broke, since it couldn't check that the resulting URL existed. Disabled.
    //filter( "../../index.html", "http://www.kde.org/index.html", KURIFilterData::NET_PROTOCOL, "kshorturifilter", "http://www.kde.org/tes1/tes2/" );

    // User defined entries...
    filter( "localhost", "http://localhost", KURIFilterData::NET_PROTOCOL );

    kdDebug() << "All tests done." << endl;
    return 0;
}


/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <config.h>
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
    
    if( abs_path ) 
    {
        m_filterData->setAbsolutePath( QString::fromLatin1( abs_path ) );
        kdDebug() << "Filtering: " << a << " with abs_path=" << abs_path << endl;
    }  
    else
        kdDebug() << "Filtering: " << a << endl;
    
    if (KURIFilter::self()->filterURI(*m_filterData, list))
    {
        // Copied from minicli...
        QString cmd;
        KURL uri = m_filterData->uri();
        
        if ( uri.isLocalFile() && !uri.hasRef() && uri.query().isEmpty() )
            cmd = uri.path();
        else
            cmd = uri.url();
        
        switch( m_filterData->uriType() )
        {
            case KURIFilterData::LOCAL_FILE:
            case KURIFilterData::LOCAL_DIR:
            case KURIFilterData::HELP:
                kdDebug() << "*** Result: Local Resource =>  '" 
                          << m_filterData->uri().url() << "'" << endl;
                break;
            case KURIFilterData::NET_PROTOCOL:
                kdDebug() << "*** Result: Network Resource => '"
                          << m_filterData->uri().url() << "'" << endl;
                break;
            case KURIFilterData::SHELL:                    
            case KURIFilterData::EXECUTABLE:
                if( m_filterData->hasArgsAndOptions() )
                    cmd += m_filterData->argsAndOptions();
                kdDebug() << "*** Result: Executable/Shell => '" << cmd << "'"<< endl;
                break;
            case KURIFilterData::ERROR:
                kdDebug() << "*** Result: Encountered error. See reason below." << endl;
                break;
            default:
                kdDebug() << "*** Result: Unknown or invalid resource." << endl;
        }
        
        if ( expectedUriType != -1 && expectedUriType != m_filterData->uriType() )
        {
            kdError() << " Got URI type " << s_uritypes[m_filterData->uriType()] 
                      << " expected " << s_uritypes[expectedUriType] << endl;
            ::exit(1);
        }
        
        if ( expectedResult )
        {
            if ( cmd != QString::fromLatin1( expectedResult ) )
            {
                kdError() << " Got " << cmd << " expected " << expectedResult << endl;
                ::exit(1);
            }
        }
    }
    else
    {        
        if ( expectedUriType == NO_FILTERING )
            kdDebug() << "*** No filtering required." << endl;
        else  
        {
            kdDebug() << "*** Could not be filtered." << endl;
            if( expectedUriType != m_filterData->uriType() )
            {
                kdError() << " Got URI type " << s_uritypes[m_filterData->uriType()]
                          << " expected " << s_uritypes[expectedUriType] << endl;
                ::exit(1);
            }
        }
    }
    
    delete m_filterData;
    kdDebug() << "-----" << endl;
}

static const char appName[] = "kurifiltertest";
static const char programName[] = I18N_NOOP("kurifiltertest");
static const char description[] = I18N_NOOP("Unit test for the URI filter plugin framework.");
static const char version[] = "1.5";

static const KCmdLineOptions options[] =
{
   { "s", I18N_NOOP("Use space as keyword delimeter for web shortcuts."), 0},
   KCmdLineLastOption
};

int main(int argc, char **argv) 
{
    KAboutData aboutData(appName, programName, version, description);
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );
    
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
        // local file with query (can be used by javascript)
    filter( "file:/etc/passwd?foo=bar", "file:/etc/passwd?foo=bar", KURIFilterData::LOCAL_FILE );
        // local file with ? in the name (#58990)
    QFile tmpFile( "/tmp/kurifiltertest?foo" ); // Yeah, I know, security risk blah blah. This is a test prog!
    
    if ( tmpFile.open( IO_ReadWrite ) ) 
    {
        QCString fname = QFile::encodeName( tmpFile.name() );
        filter(fname, fname, KURIFilterData::LOCAL_FILE);
        tmpFile.close();
        tmpFile.remove();
    }
    else
        kdDebug() << "Couldn't create " << tmpFile.name() << ", skipping test" << endl;
        // hostnames are lowercased by KURL
    filter( "http://www.myDomain.commyPort/ViewObjectRes//Default:name=hello",
            "http://www.mydomain.commyport/ViewObjectRes//Default:name=hello", KURIFilterData::NET_PROTOCOL);
    filter( "ftp://ftp.kde.org", "ftp://ftp.kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "ftp://username@ftp.kde.org:500", "ftp://username@ftp.kde.org:500", KURIFilterData::NET_PROTOCOL );

    // ShortURI/LocalDomain filter tests. NOTE: any of these tests can fail
    // if you have specified your own patterns in kshorturifilterrc. For 
    // examples, see $KDEDIR/share/config/kshorturifilterrc .
    filter( "linuxtoday.com", "http://linuxtoday.com", KURIFilterData::NET_PROTOCOL );
    filter( "LINUXTODAY.COM", "http://linuxtoday.com", KURIFilterData::NET_PROTOCOL );
    filter( "kde.org", "http://kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "ftp.kde.org", "ftp://ftp.kde.org", KURIFilterData::NET_PROTOCOL );
    filter( "ftp.kde.org:21", "ftp://ftp.kde.org:21", KURIFilterData::NET_PROTOCOL );
    filter( "cr.yp.to", "http://cr.yp.to", KURIFilterData::NET_PROTOCOL );
    filter( "user@192.168.1.0:3128", "http://user@192.168.1.0:3128", KURIFilterData::NET_PROTOCOL );
    filter( "127.0.0.1", "http://127.0.0.1", KURIFilterData::NET_PROTOCOL );
    filter( "127.0.0.1:3128", "http://127.0.0.1:3128", KURIFilterData::NET_PROTOCOL );
    filter( "foo@bar.com", "mailto:foo@bar.com", KURIFilterData::NET_PROTOCOL );
    filter( "www.123.foo", "http://www.123.foo", KURIFilterData::NET_PROTOCOL );
    filter( "user@www.123.foo:3128", "http://user@www.123.foo:3128", KURIFilterData::NET_PROTOCOL );
    
    // Exotic IPv4 address formats...
    filter( "127.1", "http://127.1", KURIFilterData::NET_PROTOCOL );
    filter( "127.0.1", "http://127.0.1", KURIFilterData::NET_PROTOCOL );
        
    // Local domain filter territory - If you uncomment this test, make sure
    // you adjust this based on the localhost entry in /etc/hosts.
    // filter( "localhost:3128", "http://localhost.localdomain:3128", KURIFilterData::NET_PROTOCOL );
    filter( "localhost", "http://localhost", KURIFilterData::NET_PROTOCOL );

    filter( "/", "/", KURIFilterData::LOCAL_DIR );
    filter( "/", "/", KURIFilterData::LOCAL_DIR, "kshorturifilter" );
    filter( "~/.kderc", QDir::homeDirPath().local8Bit()+"/.kderc", KURIFilterData::LOCAL_FILE, "kshorturifilter" );
    filter( "~", QDir::homeDirPath().local8Bit(), KURIFilterData::LOCAL_DIR, "kshorturifilter", "/tmp" );
    filter( "~foobar", 0, KURIFilterData::ERROR, "kshorturifilter" );
    filter( "user@host.domain", "mailto:user@host.domain", KURIFilterData::NET_PROTOCOL ); // new in KDE-3.2

    
    // Should not be filtered at all. All valid protocols of this form will be ignored.
    filter( "smb:" , "smb:", KURIFilterData::UNKNOWN );
    filter( "ftp:" , "ftp:", KURIFilterData::UNKNOWN );
    filter( "http:" , "http:", KURIFilterData::UNKNOWN );
    
    /* 
     Automatic searching tests. NOTE: If the Default search engine is set to 'None',
     this stuff will fail as the status returned will then be KURIFilterData::UNKNOWN.
    */
    filter( "gg:", 0 , KURIFilterData::NET_PROTOCOL );
    filter( "KDE", 0 , KURIFilterData::NET_PROTOCOL );
    filter( "FTP", 0 , KURIFilterData::NET_PROTOCOL );

    // If your default search engine is set to 'Google', you can uncomment the test below.
    filter( "gg:", "http://www.google.com/search?q=gg%3A&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "KDE", "http://www.google.com/search?q=KDE&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( "FTP", "http://www.google.com/search?q=FTP&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );    
    

    // Typing 'ls' or any other valid unix command in konq's location bar should result in 
    // a search using the default search engine unless that is set to 'None' in which
    // case you should end up with an error message.
    //filter( "ls", "http://www.google.com/search?q=ls&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );

    // Executable tests - No IKWS in minicli
    filter( "kfmclient", "kfmclient", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "xwininfo", "xwininfo", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "KDE", "KDE", NO_FILTERING, minicliFilters );
    filter( "I/dont/exist", "I/dont/exist", NO_FILTERING, minicliFilters );
    filter( "/I/dont/exist", 0, KURIFilterData::ERROR, minicliFilters );
    filter( "/I/dont/exist#a", 0, KURIFilterData::ERROR, minicliFilters );
    filter( "kfmclient --help", "kfmclient --help", KURIFilterData::EXECUTABLE, minicliFilters ); // the args are in argsAndOptions()
    filter( "/usr/bin/gs", "/usr/bin/gs", KURIFilterData::EXECUTABLE, minicliFilters );
    filter( "/usr/bin/gs -q -option arg1", "/usr/bin/gs -q -option arg1", KURIFilterData::EXECUTABLE, minicliFilters ); // the args are in argsAndOptions()

    // ENVIRONMENT variable
    setenv( "SOMEVAR", "/somevar", 0 );
    setenv( "ETC", "/etc", 0 );
    
    QCString qtdir=getenv("QTDIR");    
    QCString home = getenv("HOME");
    QCString kdehome = getenv("KDEHOME");

    filter( "$SOMEVAR/kdelibs/kio", 0, KURIFilterData::ERROR ); // note: this dir doesn't exist...
    filter( "$ETC/passwd", "/etc/passwd", KURIFilterData::LOCAL_FILE );
    filter( "$QTDIR/doc/html/functions.html#s", QCString("file:")+qtdir+"/doc/html/functions.html#s", KURIFilterData::LOCAL_FILE );
    filter( "http://www.kde.org/$USER", "http://www.kde.org/$USER", KURIFilterData::NET_PROTOCOL ); // no expansion
    
    // Assume the default (~/.kde) if 
    if (kdehome.isEmpty())
    {
      kdehome += "$HOME/.kde";
      setenv("KDEHOME", kdehome.data(), 0);
    }
      
    filter( "$KDEHOME/share", kdehome+"/share", KURIFilterData::LOCAL_DIR );
    KStandardDirs::makeDir( "/tmp/a+plus" );
    filter( "/tmp/a+plus", "/tmp/a+plus", KURIFilterData::LOCAL_DIR );
    
    // BR 27788 - note that you need this dir to exist for this test to work. 
    //filter( "$KDEHOME/share/apps/kword/templates/Text oriented", kdehome+"/share/apps/kword/templates/Text oriented", KURIFilterData::LOCAL_DIR );
    
    filter( "$HOME/$KDEDIR/kdebase/kcontrol/ebrowsing", 0, KURIFilterData::ERROR );
    filter( "$1/$2/$3", "http://www.google.com/search?q=$1/$2/$3&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );  // can be used as bogus or valid test. Currently triggers default search, i.e. google
    filter( "$$$$", "http://www.google.com/search?q=$$$$&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL ); // worst case scenarios.

    // Replaced the match testing with a 0 since
    // the shortURI filter will return the string
    // itself if the requested environment variable
    // is not already set.
    filter( "$QTDIR", 0, KURIFilterData::LOCAL_DIR, "kshorturifilter" ); //use specific filter.
    filter( "$HOME", 0, KURIFilterData::LOCAL_DIR, "kshorturifilter" ); //use specific filter.

    // Search Engine tests    
    char delimiter = KCmdLineArgs::parsedArgs()->isSet("s") ? ' ' : ':';
    
    QCString sc;          
    filter( sc.sprintf("gg%cfoo bar",delimiter), "http://www.google.com/search?q=foo%20bar&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( sc.sprintf("bug%c55798", delimiter), "http://bugs.kde.org/show_bug.cgi?id=55798", KURIFilterData::NET_PROTOCOL );

    filter( sc.sprintf("gg%cC++", delimiter), "http://www.google.com/search?q=C%2B%2B&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( sc.sprintf("ya%cfoo bar was here", delimiter), 0, -1 ); // this triggers default search, i.e. google
    filter( sc.sprintf("gg%cwww.kde.org", delimiter), "http://www.google.com/search?q=www.kde.org&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( sc.sprintf("av%c+rock +sample", delimiter), "http://www.altavista.com/cgi-bin/query?pg=q&kl=XX&stype=stext&q=%2Brock%20%2Bsample", KURIFilterData::NET_PROTOCOL );
    filter( sc.sprintf("gg%cé", delimiter) /*eaccent in utf8*/, "http://www.google.com/search?q=%C3%A9&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );
    filter( sc.sprintf("gg%cпрйвет", delimiter) /* greetings in russian utf-8*/, "http://www.google.com/search?q=%D0%BF%D1%80%D0%B9%D0%B2%D0%B5%D1%82&ie=UTF-8&oe=UTF-8", KURIFilterData::NET_PROTOCOL );

    // Absolute Path tests for kshorturifilter
    filter( "./", kdehome+"/share", KURIFilterData::LOCAL_DIR, "kshorturifilter", kdehome+"/share/" ); // cleanDirPath removes the trailing slash
    filter( "../", kdehome, KURIFilterData::LOCAL_DIR, "kshorturifilter", kdehome+"/share" );
    filter( "apps", kdehome+"/share/apps", KURIFilterData::LOCAL_DIR, "kshorturifilter", kdehome+"/share" );
    
    kdDebug() << "All tests done. Go home..." << endl;
    return 0;
}

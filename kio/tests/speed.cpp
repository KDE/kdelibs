#include <kdebug.h>
#include <kapplication.h>
#include <time.h>
#include "speed.h"
#include <kio/job.h>
#include <kcmdlineargs.h>
#include <qdir.h>
#include <kio/global.h>

using namespace KIO;

SpeedTest::SpeedTest( const KURL & url )
    : QObject(0, "speed")
{
    Job *job = listRecursive( url );
    //Job *job = del( KURL("file:" + QDir::currentDirPath()) ); DANGEROUS !
    connect(job, SIGNAL( result( KIO::Job*)),
	    SLOT( finished( KIO::Job* ) ));
    /*connect(job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
	    SLOT( entries( KIO::Job*, const KIO::UDSEntryList&)));
    */
}

void SpeedTest::entries(KIO::Job*, const UDSEntryList& list) {

    UDSEntryListConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
      UDSEntry::ConstIterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
              kdDebug() << ( *it2 ).m_str << endl;
        }
    }
}


void SpeedTest::finished(Job*) {
    kdDebug() << "job finished" << endl;
    kapp->quit();
}

static KCmdLineOptions options[] =
{
  { "+[URL]", "the URL to list", 0 },
  { 0, 0, 0 }
};

int main(int argc, char **argv) {

    KCmdLineArgs::init( argc, argv, "speedapp", "A KIO::listRecursive testing tool", "0.0" );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KURL url;
    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = "file:" + QDir::currentDirPath();
    args->clear();

    kdDebug() << url.url() << " is probably " << (KIO::probably_slow_mounted(url.path()) ? "slow" : "normal") << " mounted\n";
    QString mp = KIO::findDeviceMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

}

#include "speed.moc"

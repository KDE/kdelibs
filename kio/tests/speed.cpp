#include <kurl.h>
#include <kdebug.h>
#include <kapp.h>
#include <time.h>
#include "speed.h"
#include <kio/job.h>
#include <qdir.h>

using namespace KIO;

SpeedTest::SpeedTest()
    : QObject(0, "speed")
{
    // Job *job = listRecursive( KURL("file:" + QDir::currentDirPath()) );
    Job *job = del( KURL("file:" + QDir::currentDirPath()) );
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
}

int main(int argc, char **argv) {

    KApplication app(argc, argv, "speedapp");

    SpeedTest test;
    app.exec();

}

#include "speed.moc"

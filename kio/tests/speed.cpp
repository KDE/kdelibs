#include <kurl.h>
#include <kdebug.h>
#include <kapp.h>
#include <time.h>
#include "speed.h"
#include <kio_job.h>
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

    UDSEntryListIterator it(list);
    for (; it.current(); ++it) {
        UDSEntry::ConstIterator it2 = it.current()->begin();
        for( ; it2 != it.current()->end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
		debug( "%s", debugString(( *it2 ).m_str));
        }
    }
}


void SpeedTest::finished(Job*) {
    debug("job finished");
}

int main(int argc, char **argv) {

    KApplication app(argc, argv, "speedapp");

    SpeedTest test;
    app.exec();

}

#include "speed.moc"

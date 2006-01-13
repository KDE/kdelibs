#include "qasyncpixmap.h"

#include <kio/job.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qfile.h>

QAsyncPixmap::QAsyncPixmap(QString url)
: QPixmap()
{
    // XXX ???
    //KTempFile
    m_dest = KGlobal::dirs()->saveLocation("tmp") + KApplication::randomString(10) + ".png";

    KIO::FileCopyJob *job = KIO::file_copy(url, m_dest, -1, true, false, false);
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotDownload(KIO::Job*)));
}

void QAsyncPixmap::slotDownload(KIO::Job *job)
{
    kdDebug() << "DOWNLOAD" << endl;
    if(job->error())
    {
        // XXX ???
        return;
    }
    bool ret = load(m_dest);
//    QFile::remove(m_dest);
    kdDebug() << "DOWNLOADed to " << m_dest << endl;
    kdDebug() << "ret = " << ret << endl;

    emit signalLoaded(this);
}

#include "qasyncpixmap.moc"

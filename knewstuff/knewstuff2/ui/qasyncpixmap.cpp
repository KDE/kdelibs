#include "qasyncpixmap.h"

#include <kio/job.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <krandom.h>
#include <kdebug.h>

#include <qfile.h>

QAsyncPixmap::QAsyncPixmap(QString url)
: QPixmap()
{
    if(!url.isEmpty())
    {
        // XXX ???
         //KTempFile
        m_dest = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".png";

        KIO::FileCopyJob *job = KIO::file_copy(url, m_dest, -1, true, false, false);
        connect(job, SIGNAL(result(KJob*)), SLOT(slotDownload(KJob*)));
    }
}

void QAsyncPixmap::slotDownload(KJob *job)
{
    kDebug(550) << "DOWNLOAD" << endl;
    if(job->error())
    {
        // XXX ???
        return;
    }
    bool ret = load(m_dest);
//    QFile::remove(m_dest);
    kDebug(550) << "DOWNLOADed to " << m_dest << endl;
    kDebug(550) << "ret = " << ret << endl;

    emit signalLoaded(this);
}

#include "qasyncpixmap.moc"

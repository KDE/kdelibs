#include "kjavadownloader.h"
#include "kjavaappletserver.h"

#include <kurl.h>
#include <kio/job.h>

class KJavaDownloaderPrivate
{
friend class KJavaDownloader;
public:
    ~KJavaDownloaderPrivate()
    {
        delete url;
        delete job;
    }
private:
    QString           loaderID;
    KURL*             url;
    QByteArray        file;
    KIO::TransferJob* job;
};


KJavaDownloader::KJavaDownloader( QString& ID, QString& url )
{
    d = new KJavaDownloaderPrivate;

    d->loaderID = ID;
    d->url = new KURL( url );

    d->job = KIO::get( url, false, false );
    connect( d->job,  SIGNAL(data( KIO::Job*, const QByteArray& )),
             this,    SLOT(slotData( KIO::Job*, const QByteArray& )) );
    connect( d->job,  SIGNAL(finished()),
             this,    SLOT(finished()) );
}

KJavaDownloader::~KJavaDownloader()
{
    delete d;
}

void KJavaDownloader::slotData( KIO::Job*, const QByteArray& qb )
{
    int cur_size = d->file.size();
    int qb_size = qb.size();
    d->file.resize( cur_size + qb_size );

    memcpy( d->file.data() + cur_size, qb.data(), qb_size );
}

void KJavaDownloader::slotFinished()
{
    if( d->job->error() )
    {
    }
    else
    {
        KJavaAppletServer* server = KJavaAppletServer::allocateJavaServer();
        server->sendURLData( d->loaderID, d->url->url(), d->file );
        KJavaAppletServer::freeJavaServer();
    }

    delete this;
}


#ifndef KJAVADOWNLOADER_H
#define KJAVADOWNLOADER_H

#include <qobject.h>
#include <kio/jobclasses.h>

class KJavaDownloaderPrivate;

class KJavaDownloader : public QObject
{
Q_OBJECT

public:
	KJavaDownloader( QString& ID, QString& url );
	~KJavaDownloader();

protected slots:
    void slotData( KIO::Job*, const QByteArray& );
    void slotFinished();

private:
    KJavaDownloaderPrivate* d;

};

#endif

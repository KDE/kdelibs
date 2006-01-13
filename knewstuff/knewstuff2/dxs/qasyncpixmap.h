#ifndef QASYNC_PIXMAP_H
#define QASYNC_PIXMAP_H

#include <qpixmap.h>
#include <qobject.h>

namespace KIO
{
    class Job;
};

class QAsyncPixmap : public QObject, public QPixmap
{
    Q_OBJECT
    public:
        QAsyncPixmap(QString url);

    signals:
        void signalLoaded(QPixmap *pix);

    private slots:
        void slotDownload(KIO::Job *job);

    private:
        QString m_dest;
};

#endif


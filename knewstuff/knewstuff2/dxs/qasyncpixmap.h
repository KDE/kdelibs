#ifndef QASYNC_PIXMAP_H
#define QASYNC_PIXMAP_H

#include <qpixmap.h>
#include <qobject.h>

class KJob;

class QAsyncPixmap : public QObject, public QPixmap
{
    Q_OBJECT
    public:
        QAsyncPixmap(QString url);

    signals:
        void signalLoaded(QPixmap *pix);

    private slots:
        void slotDownload(KJob *job);

    private:
        QString m_dest;
};

#endif


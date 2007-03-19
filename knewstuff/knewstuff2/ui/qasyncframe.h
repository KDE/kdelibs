#ifndef QASYNC_FRAME_H
#define QASYNC_FRAME_H

#include <qframe.h>

class QPixmap;

class QAsyncFrame : public QFrame
{
    Q_OBJECT
    public:
        QAsyncFrame(QWidget *parent = 0);

    private slots:
        void slotLoaded(QPixmap *pix);
};

#endif


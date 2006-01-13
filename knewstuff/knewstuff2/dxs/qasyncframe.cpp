#include "qasyncframe.h"

#include <qpixmap.h>

QAsyncFrame::QAsyncFrame(QWidget *parent)
: QFrame(parent)
{
}

void QAsyncFrame::slotLoaded(QPixmap *pix)
{
    setBackgroundPixmap(*pix);
}

#include "qasyncframe.moc"

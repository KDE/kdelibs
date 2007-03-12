#include "qasyncframe.h"

#include <qpixmap.h>

QAsyncFrame::QAsyncFrame(QWidget *parent)
: QFrame(parent)
{
}

void QAsyncFrame::slotLoaded(QPixmap *pix)
{
    QPalette palette;
    palette.setBrush(backgroundRole(), QBrush(*pix));
    setPalette(palette);
}

#include "qasyncframe.moc"

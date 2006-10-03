#!/bin/bash

QTDIR=/home/kde4/kdesvn/qt-copy
HEADERS="qbitarray.h   qbytearray.h  qcursor.h      qicon.h
         qline.h       qpalette.h    qpoint.h       qregexp.h
         qsizepolicy.h qurl.h        qbitmap.h      qchar.h
         qdatetime.h   qimage.h      qlocale.h      qpen.h
         qpolygon.h    qregion.h     qstring.h      qbrush.h
         qcolor.h      qfont.h       qkeysequence.h qmatrix.h
         qpixmap.h     qrect.h       qsize.h        qtextformat.h
         qmatrix.h"

for header in $HEADERS
do
    cp $QTDIR/include/Qt/$header ./input
done



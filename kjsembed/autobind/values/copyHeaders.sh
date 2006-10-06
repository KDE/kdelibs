#!/bin/bash

HEADERDIR=$QTDIR
HEADERS="qbitarray.h   qbytearray.h  qcursor.h      qicon.h
         qline.h       qpalette.h    qpoint.h       qregexp.h
         qsizepolicy.h qurl.h        qbitmap.h      qchar.h
         qdatetime.h   qimage.h      qlocale.h      qpen.h
         qpolygon.h    qregion.h     qstring.h      qbrush.h
         qcolor.h      qfont.h       qkeysequence.h qmatrix.h
         qpixmap.h     qrect.h       qsize.h        qtextformat.h
         qmatrix.h     qnamespace.h  qglobal.h      qrgb.h"

for header in $HEADERS
do
    cp -Lv $HEADERDIR/include/Qt/$header ./input
done



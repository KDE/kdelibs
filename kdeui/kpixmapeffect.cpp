#include "kpixmapeffect.h"
#include <qimage.h>
#include <dither.h>
#include <qcolor.h>

void KPixmapEffect::pyramidGradient(KPixmap &pixmap, const QColor &ca,
                                    const QColor &cb, int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca;

    float rfd, gfd, bfd;
    QImage image(pixmap.width(), pixmap.height(), 32);
    unsigned char xtable[pixmap.width()][3], ytable[pixmap.height()][3];
    register unsigned int x, y;

    rca = ca.red();
    gca = ca.green();
    bca = ca.blue();

    rDiff = cb.red() - ca.red();
    gDiff = cb.green() - ca.green();
    bDiff = cb.blue() - ca.blue();

    rfd = (float)rDiff/pixmap.width(); // CT: no need to stick w or h in
    gfd = (float)gDiff/pixmap.width(); // a var, they are inline (mosfet)
    bfd = (float)bDiff/pixmap.width();

    int rSign = rDiff>0? 1: -1;
    int gSign = gDiff>0? 1: -1;
    int bSign = bDiff>0? 1: -1;

    float rd = -(float)rDiff/2;
    float gd = -(float)gDiff/2;
    float bd = -(float)bDiff/2;

    for (x = 0; x < pixmap.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
        xtable[x][0] = (unsigned char) abs((int)rd);
        xtable[x][1] = (unsigned char) abs((int)gd);
        xtable[x][2] = (unsigned char) abs((int)bd);
    }

    rfd = (float)rDiff/pixmap.height();
    gfd = (float)gDiff/pixmap.height();
    bfd = (float)bDiff/pixmap.height();

    rd = -(float)rDiff/2;
    gd = -(float)gDiff/2;
    bd = -(float)bDiff/2;

    for (y = 0; y < pixmap.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
        ytable[y][0] = (unsigned char) abs((int)rd);
        ytable[y][1] = (unsigned char) abs((int)gd);
        ytable[y][2] = (unsigned char) abs((int)bd);
    }

    for (y = 0; y < pixmap.height(); y++) {
        unsigned int *scanline = (unsigned int *)image.scanLine(y);
        for (x = 0; x < pixmap.width(); x++) {
            scanline[x] = qRgb(rca + rSign * (xtable[x][0] + ytable[y][0]),
                               gca + gSign * (xtable[x][1] + ytable[y][1]),
                               bca + bSign * (xtable[x][2] + ytable[y][2]));
        }
    }
    if(pixmap.depth() <= 16 ) {
        if( pixmap.depth() == 16 )
            ncols = 32;
        if ( ncols < 2 || ncols > 256 )
            ncols = 3;
        QColor *dPal = new QColor[ncols];
        for (int i=0; i<ncols; i++) {
            dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
                             gca + gDiff * i / ( ncols - 1 ),
                             bca + bDiff * i / ( ncols - 1 ) );
        }
        kFSDither dither(dPal, ncols);
        image = dither.dither(image);
        pixmap.convertFromImage(image);
        delete [] dPal;
    }
    else
        pixmap.convertFromImage(image);
}

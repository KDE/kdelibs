#include "kpixmapeffect.h"
#include <qimage.h>
#include <dither.h>
#include <qcolor.h>

void KPixmapEffect::advancedGradient(KPixmap &pixmap, const QColor &ca,
                                     const QColor &cb, GradientType eff,
                                     int ncols)
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
    
    float rd = (float)rDiff/2;
    float gd = (float)gDiff/2;
    float bd = (float)bDiff/2;
    
    for (x = 0; x < pixmap.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd) {
      xtable[x][0] = (unsigned char) abs((int)rd);
      xtable[x][1] = (unsigned char) abs((int)gd);
      xtable[x][2] = (unsigned char) abs((int)bd);
    }
    
    rfd = (float)rDiff/pixmap.height();
    gfd = (float)gDiff/pixmap.height();
    bfd = (float)bDiff/pixmap.height();
    
    rd = (float)rDiff/2;
    gd = (float)gDiff/2;
    bd = (float)bDiff/2;
    
    for (y = 0; y < pixmap.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd) {
      if ( eff == PyramidGradient)
	{
	  ytable[y][0] = (unsigned char) abs((int)rd);
	  ytable[y][1] = (unsigned char) abs((int)gd);
	  ytable[y][2] = (unsigned char) abs((int)bd);
	}
    
      else if ( eff == RectangleGradient )
	{
	  ytable[y][0] = (unsigned char) (rDiff/2-abs((int)rd));
	  ytable[y][1] = (unsigned char) (gDiff/2-abs((int)gd));
	  ytable[y][2] = (unsigned char) (bDiff/2-abs((int)bd));
	}
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

/* This builds a 256 byte unsigned char lookup table with all
 * the possible percent values prior to applying the effect, then uses
 * integer math for the pixels. For any image larger than 9x9 this will be
 * less expensive than doing a float operation on the 3 color components of
 * each pixel. (mosfet)
 */
void KPixmapEffect::intensity(QImage &image, float percent, bool brighten)
{
    int i, tmp, r, g, b;
    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char segTbl[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
        image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
        (unsigned int *)image.colorTable();

    if(brighten){ // keep overflow check out of loops
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
            g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
            b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
    else{
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
            g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
            b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
}

void KPixmapEffect::channelIntensity(QImage &image, float percent,
                                     RGBComponent channel,
                                     bool brighten)
{
    int i, tmp, c;
    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char segTbl[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
        image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
        (unsigned int *)image.colorTable();

    if(brighten){ // keep overflow check out of loops
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        if(channel == Red){ // and here ;-)
            for(i=0; i < pixels; ++i){
                c = qRed(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgb(c, qGreen(data[i]), qBlue(data[i]));
            }
        }
        if(channel == Green){
            for(i=0; i < pixels; ++i){
                c = qGreen(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgb(qRed(data[i]), c, qBlue(data[i]));
            }
        }
        else{
            for(i=0; i < pixels; ++i){
                c = qBlue(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgb(qRed(data[i]), qGreen(data[i]), c);
            }
        }

    }
    else{
        if(channel == Red){
            for(i=0; i < pixels; ++i){
                c = qRed(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgb(c, qGreen(data[i]), qBlue(data[i]));
            }
        }
        if(channel == Green){
            for(i=0; i < pixels; ++i){
                c = qGreen(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgb(qRed(data[i]), c, qBlue(data[i]));
            }
        }
        else{
            for(i=0; i < pixels; ++i){
                c = qBlue(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgb(qRed(data[i]), qGreen(data[i]), c);
            }
        }
    }
}


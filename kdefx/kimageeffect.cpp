/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>
	      (C) 2000       Josef Weidendorfer <weidendo@in.tum.de>

*/

// $Id$

#include <math.h>

#include <qimage.h>
#include <stdlib.h>
#include <iostream.h>

#include "kimageeffect.h"

//======================================================================
//
// Gradient effects
//
//======================================================================

QImage KImageEffect::gradient(const QSize &size, const QColor &ca,
	const QColor &cb, GradientType eff, int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image(size, 32);

    if (size.width() == 0 || size.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::gradient: invalid image" << endl;
#endif
      return image;
    }

    register int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient || eff == HorizontalGradient ){

        uint *p;
        uint rgb;

        register int rl = rca << 16;
        register int gl = gca << 16;
        register int bl = bca << 16;

        if( eff == VerticalGradient ) {

            int rcdelta = ((1<<16) / size.height()) * rDiff;
            int gcdelta = ((1<<16) / size.height()) * gDiff;
            int bcdelta = ((1<<16) / size.height()) * bDiff;

            for ( y = 0; y < size.height(); y++ ) {
                p = (uint *) image.scanLine(y);

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                rgb = qRgb( (rl>>16), (gl>>16), (bl>>16) );

                for( x = 0; x < size.width(); x++ ) {
                    *p = rgb;
                    p++;
                }
            }

        }
        else {                  // must be HorizontalGradient

            unsigned int *o_src = (unsigned int *)image.scanLine(0);
            unsigned int *src = o_src;

            int rcdelta = ((1<<16) / size.width()) * rDiff;
            int gcdelta = ((1<<16) / size.width()) * gDiff;
            int bcdelta = ((1<<16) / size.width()) * bDiff;

            for( x = 0; x < size.width(); x++) {

                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                *src++ = qRgb( (rl>>16), (gl>>16), (bl>>16));
            }

            src = o_src;

            // Believe it or not, manually copying in a for loop is faster
            // than calling memcpy for each scanline (on the order of ms...).
            // I think this is due to the function call overhead (mosfet).

            for (y = 1; y < size.height(); ++y) {

                p = (unsigned int *)image.scanLine(y);
                src = o_src;
                for(x=0; x < size.width(); ++x)
                    *p++ = *src++;
            }
        }
    }

    else {

        float rfd, gfd, bfd;
        float rd = rca, gd = gca, bd = bca;

        unsigned char *xtable[3];
        unsigned char *ytable[3];

        unsigned int w = size.width(), h = size.height();
        xtable[0] = new unsigned char[w];
        xtable[1] = new unsigned char[w];
        xtable[2] = new unsigned char[w];
        ytable[0] = new unsigned char[h];
        ytable[1] = new unsigned char[h];
        ytable[2] = new unsigned char[h];
        w*=2, h*=2;

        if ( eff == DiagonalGradient || eff == CrossDiagonalGradient) {
            // Diagonal dgradient code inspired by BlackBox (mosfet)
            // BlackBox dgradient is (C) Brad Hughes, <bhughes@tcac.net> and
            // Mike Cole <mike@mydot.com>.

            rfd = (float)rDiff/w;
            gfd = (float)gDiff/w;
            bfd = (float)bDiff/w;

            int dir;
            for (x = 0; x < size.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
                dir = eff == DiagonalGradient? x : size.width() - x - 1;
                xtable[0][dir] = (unsigned char) rd;
                xtable[1][dir] = (unsigned char) gd;
                xtable[2][dir] = (unsigned char) bd;
            }
            rfd = (float)rDiff/h;
            gfd = (float)gDiff/h;
            bfd = (float)bDiff/h;
            rd = gd = bd = 0;
            for (y = 0; y < size.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
                ytable[0][y] = (unsigned char) rd;
                ytable[1][y] = (unsigned char) gd;
                ytable[2][y] = (unsigned char) bd;
            }

            for (y = 0; y < size.height(); y++) {
                unsigned int *scanline = (unsigned int *)image.scanLine(y);
                for (x = 0; x < size.width(); x++) {
                    scanline[x] = qRgb(xtable[0][x] + ytable[0][y],
                                       xtable[1][x] + ytable[1][y],
                                       xtable[2][x] + ytable[2][y]);
                }
            }
        }

        else if (eff == RectangleGradient ||
                 eff == PyramidGradient ||
                 eff == PipeCrossGradient ||
                 eff == EllipticGradient)
        {
            int rSign = rDiff>0? 1: -1;
            int gSign = gDiff>0? 1: -1;
            int bSign = bDiff>0? 1: -1;

            rfd = (float)rDiff / size.width();
            gfd = (float)gDiff / size.width();
            bfd = (float)bDiff / size.width();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (x = 0; x < size.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                xtable[0][x] = (unsigned char) abs((int)rd);
                xtable[1][x] = (unsigned char) abs((int)gd);
                xtable[2][x] = (unsigned char) abs((int)bd);
            }

            rfd = (float)rDiff/size.height();
            gfd = (float)gDiff/size.height();
            bfd = (float)bDiff/size.height();

            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;

            for (y = 0; y < size.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd)
            {
                ytable[0][y] = (unsigned char) abs((int)rd);
                ytable[1][y] = (unsigned char) abs((int)gd);
                ytable[2][y] = (unsigned char) abs((int)bd);
            }
            unsigned int rgb;
            int h = (size.height()+1)>>1;
            for (y = 0; y < h; y++) {
                unsigned int *sl1 = (unsigned int *)image.scanLine(y);
                unsigned int *sl2 = (unsigned int *)image.scanLine(QMAX(size.height()-y-1, y));

                int w = (size.width()+1)>>1;
                int x2 = size.width()-1;

                for (x = 0; x < w; x++, x2--) {
		    rgb = 0;
                    if (eff == PyramidGradient) {
                        rgb = qRgb(rcb-rSign*(xtable[0][x]+ytable[0][y]),
                                   gcb-gSign*(xtable[1][x]+ytable[1][y]),
                                   bcb-bSign*(xtable[2][x]+ytable[2][y]));
                    }
                    if (eff == RectangleGradient) {
                        rgb = qRgb(rcb - rSign *
                                   QMAX(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   QMAX(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   QMAX(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == PipeCrossGradient) {
                        rgb = qRgb(rcb - rSign *
                                   QMIN(xtable[0][x], ytable[0][y]) * 2,
                                   gcb - gSign *
                                   QMIN(xtable[1][x], ytable[1][y]) * 2,
                                   bcb - bSign *
                                   QMIN(xtable[2][x], ytable[2][y]) * 2);
                    }
                    if (eff == EllipticGradient) {
                        rgb = qRgb(rcb - rSign *
                                   (int)sqrt((xtable[0][x]*xtable[0][x] +
                                              ytable[0][y]*ytable[0][y])*2.0),
                                   gcb - gSign *
                                   (int)sqrt((xtable[1][x]*xtable[1][x] +
                                              ytable[1][y]*ytable[1][y])*2.0),
                                   bcb - bSign *
                                   (int)sqrt((xtable[2][x]*xtable[2][x] +
                                              ytable[2][y]*ytable[2][y])*2.0));
                    }

                    sl1[x] = sl2[x] = rgb;
                    sl1[x2] = sl2[x2] = rgb;
                }
            }
        }

        delete [] xtable[0];
        delete [] xtable[1];
        delete [] xtable[2];
        delete [] ytable[0];
        delete [] ytable[1];
        delete [] ytable[2];
    }

    // dither if necessary
    if (ncols && (QPixmap::defaultDepth() < 15 )) {
	if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	QColor *dPal = new QColor[ncols];
	for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	}
    dither(image, dPal, ncols);
	delete [] dPal;
    }

    return image;
}


// -----------------------------------------------------------------------------

//CT this was (before Dirk A. Mueller's speedup changes)
//   merely the same code as in the above method, but it's supposedly
//   way less performant since it introduces a lot of supplementary tests
//   and simple math operations for the calculus of the balance.
//      (surprizingly, it isn't less performant, in the contrary :-)
//   Yes, I could have merged them, but then the excellent performance of
//   the balanced code would suffer with no other gain than a mere
//   source code and byte code size economy.

QImage KImageEffect::unbalancedGradient(const QSize &size, const QColor &ca,
	const QColor &cb, GradientType eff, int xfactor, int yfactor,
	int ncols)
{
    int dir; // general parameter used for direction switches

    bool _xanti = false , _yanti = false;

    if (xfactor < 0) _xanti = true; // negative on X direction
    if (yfactor < 0) _yanti = true; // negative on Y direction

    xfactor = abs(xfactor);
    yfactor = abs(yfactor);

    if (!xfactor) xfactor = 1;
    if (!yfactor) yfactor = 1;

    if (xfactor > 200 ) xfactor = 200;
    if (yfactor > 200 ) yfactor = 200;


    //    float xbal = xfactor/5000.;
    //    float ybal = yfactor/5000.;
    float xbal = xfactor/30./size.width();
    float ybal = yfactor/30./size.height();
    float rat;

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image(size, 32);

    if (size.width() == 0 || size.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::unbalancedGradient : invalid image\n";
#endif
      return image;
    }

    register int x, y;
    unsigned int *scanline;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient || eff == HorizontalGradient){
        QColor cRow;

        uint *p;
        uint rgbRow;

	if( eff == VerticalGradient) {
	  for ( y = 0; y < size.height(); y++ ) {
	    dir = _yanti ? y : size.height() - 1 - y;
            p = (uint *) image.scanLine(dir);
            rat =  1 - exp( - (float)y  * ybal );

            cRow.setRgb( rcb - (int) ( rDiff * rat ),
                         gcb - (int) ( gDiff * rat ),
                         bcb - (int) ( bDiff * rat ) );

            rgbRow = cRow.rgb();

            for( x = 0; x < size.width(); x++ ) {
	      *p = rgbRow;
	      p++;
            }
	  }
	}
	else {

	  unsigned int *src = (unsigned int *)image.scanLine(0);
	  for(x = 0; x < size.width(); x++ )
	    {
	      dir = _xanti ? x : size.width() - 1 - x;
	      rat = 1 - exp( - (float)x  * xbal );

	      src[dir] = qRgb(rcb - (int) ( rDiff * rat ),
			    gcb - (int) ( gDiff * rat ),
			    bcb - (int) ( bDiff * rat ));
	    }

	  // Believe it or not, manually copying in a for loop is faster
	  // than calling memcpy for each scanline (on the order of ms...).
	  // I think this is due to the function call overhead (mosfet).

	  for(y = 1; y < size.height(); ++y)
	    {
	      scanline = (unsigned int *)image.scanLine(y);
	      for(x=0; x < size.width(); ++x)
		scanline[x] = src[x];
	    }
	}
    }

    else {
      int w=size.width(), h=size.height();

      unsigned char *xtable[3];
      unsigned char *ytable[3];
      xtable[0] = new unsigned char[w];
      xtable[1] = new unsigned char[w];
      xtable[2] = new unsigned char[w];
      ytable[0] = new unsigned char[h];
      ytable[1] = new unsigned char[h];
      ytable[2] = new unsigned char[h];

      if ( eff == DiagonalGradient || eff == CrossDiagonalGradient)
	{
	  for (x = 0; x < w; x++) {
              dir = _xanti ? x : w - 1 - x;
              rat = 1 - exp( - (float)x * xbal );

              xtable[0][dir] = (unsigned char) ( rDiff/2 * rat );
              xtable[1][dir] = (unsigned char) ( gDiff/2 * rat );
              xtable[2][dir] = (unsigned char) ( bDiff/2 * rat );
          }

	  for (y = 0; y < h; y++) {
              dir = _yanti ? y : h - 1 - y;
              rat =  1 - exp( - (float)y  * ybal );

              ytable[0][dir] = (unsigned char) ( rDiff/2 * rat );
              ytable[1][dir] = (unsigned char) ( gDiff/2 * rat );
              ytable[2][dir] = (unsigned char) ( bDiff/2 * rat );
          }

	  for (y = 0; y < h; y++) {
              unsigned int *scanline = (unsigned int *)image.scanLine(y);
              for (x = 0; x < w; x++) {
                  scanline[x] = qRgb(rcb - (xtable[0][x] + ytable[0][y]),
                                     gcb - (xtable[1][x] + ytable[1][y]),
                                     bcb - (xtable[2][x] + ytable[2][y]));
              }
          }
        }

      else if (eff == RectangleGradient ||
               eff == PyramidGradient ||
               eff == PipeCrossGradient ||
               eff == EllipticGradient)
      {
          int rSign = rDiff>0? 1: -1;
          int gSign = gDiff>0? 1: -1;
          int bSign = bDiff>0? 1: -1;

          for (x = 0; x < w; x++)
	    {
                dir = _xanti ? x : w - 1 - x;
                rat =  1 - exp( - (float)x * xbal );

                xtable[0][dir] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
                xtable[1][dir] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
                xtable[2][dir] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
            }

          for (y = 0; y < h; y++)
          {
              dir = _yanti ? y : h - 1 - y;

              rat =  1 - exp( - (float)y * ybal );

              ytable[0][dir] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
              ytable[1][dir] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
              ytable[2][dir] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
          }

          for (y = 0; y < h; y++) {
              unsigned int *scanline = (unsigned int *)image.scanLine(y);
              for (x = 0; x < w; x++) {
                  if (eff == PyramidGradient)
                  {
                      scanline[x] = qRgb(rcb-rSign*(xtable[0][x]+ytable[0][y]),
                                         gcb-gSign*(xtable[1][x]+ytable[1][y]),
                                         bcb-bSign*(xtable[2][x]+ytable[2][y]));
                  }
                  if (eff == RectangleGradient)
                  {
                      scanline[x] = qRgb(rcb - rSign *
                                         QMAX(xtable[0][x], ytable[0][y]) * 2,
                                         gcb - gSign *
                                         QMAX(xtable[1][x], ytable[1][y]) * 2,
                                         bcb - bSign *
                                         QMAX(xtable[2][x], ytable[2][y]) * 2);
                  }
                  if (eff == PipeCrossGradient)
                  {
                      scanline[x] = qRgb(rcb - rSign *
                                         QMIN(xtable[0][x], ytable[0][y]) * 2,
                                         gcb - gSign *
                                         QMIN(xtable[1][x], ytable[1][y]) * 2,
                                         bcb - bSign *
                                         QMIN(xtable[2][x], ytable[2][y]) * 2);
                  }
                  if (eff == EllipticGradient)
                  {
                      scanline[x] = qRgb(rcb - rSign *
                                         (int)sqrt((xtable[0][x]*xtable[0][x] +
                                                    ytable[0][y]*ytable[0][y])*2.0),
                                         gcb - gSign *
                                         (int)sqrt((xtable[1][x]*xtable[1][x] +
                                                    ytable[1][y]*ytable[1][y])*2.0),
                                         bcb - bSign *
                                         (int)sqrt((xtable[2][x]*xtable[2][x] +
                                                    ytable[2][y]*ytable[2][y])*2.0));
                  }
              }
          }
      }

      if (ncols && (QPixmap::defaultDepth() < 15 )) {
          if ( ncols < 2 || ncols > 256 )
              ncols = 3;
          QColor *dPal = new QColor[ncols];
          for (int i=0; i<ncols; i++) {
              dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
                               gca + gDiff * i / ( ncols - 1 ),
                               bca + bDiff * i / ( ncols - 1 ) );
          }
          dither(image, dPal, ncols);
          delete [] dPal;
      }

      delete [] xtable[0];
      delete [] xtable[1];
      delete [] xtable[2];
      delete [] ytable[0];
      delete [] ytable[1];
      delete [] ytable[2];

    }

    return image;
}


//======================================================================
//
// Intensity effects
//
//======================================================================


/* This builds a 256 byte unsigned char lookup table with all
 * the possible percent values prior to applying the effect, then uses
 * integer math for the pixels. For any image larger than 9x9 this will be
 * less expensive than doing a float operation on the 3 color components of
 * each pixel. (mosfet)
 */

QImage& KImageEffect::intensity(QImage &image, float percent)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::intensity : invalid image\n";
#endif
      return image;
    }

    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char *segTbl = new unsigned char[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
        image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
        (unsigned int *)image.colorTable();

    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

    if(brighten){ // keep overflow check out of loops
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        for(int i=0; i < pixels; ++i){
            int r = qRed(data[i]);
            int g = qGreen(data[i]);
            int b = qBlue(data[i]);
            int a = qAlpha(data[i]);
            r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
            g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
            b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
            data[i] = qRgba(r, g, b,a);
        }
    }
    else{
        for(int i=0; i < pixels; ++i){
            int r = qRed(data[i]);
            int g = qGreen(data[i]);
            int b = qBlue(data[i]);
            int a = qAlpha(data[i]);
            r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
            g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
            b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
            data[i] = qRgba(r, g, b, a);
        }
    }
    delete [] segTbl;

    return image;
}

QImage& KImageEffect::channelIntensity(QImage &image, float percent,
                                       RGBComponent channel)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::channelIntensity : invalid image\n";
#endif
      return image;
    }

    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char *segTbl = new unsigned char[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
        image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
        (unsigned int *)image.colorTable();
    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

    if(brighten){ // keep overflow check out of loops
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(int i=0; i < segColors; ++i){
            int tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        if(channel == Red){ // and here ;-)
            for(int i=0; i < pixels; ++i){
                int c = qRed(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgba(c, qGreen(data[i]), qBlue(data[i]), qAlpha(data[i]));
            }
        }
        if(channel == Green){
            for(int i=0; i < pixels; ++i){
                int c = qGreen(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgba(qRed(data[i]), c, qBlue(data[i]), qAlpha(data[i]));
            }
        }
        else{
            for(int i=0; i < pixels; ++i){
                int c = qBlue(data[i]);
                c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
                data[i] = qRgba(qRed(data[i]), qGreen(data[i]), c, qAlpha(data[i]));
            }
        }

    }
    else{
        if(channel == Red){
            for(int i=0; i < pixels; ++i){
                int c = qRed(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgba(c, qGreen(data[i]), qBlue(data[i]), qAlpha(data[i]));
            }
        }
        if(channel == Green){
            for(int i=0; i < pixels; ++i){
                int c = qGreen(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgba(qRed(data[i]), c, qBlue(data[i]), qAlpha(data[i]));
            }
        }
        else{
            for(int i=0; i < pixels; ++i){
                int c = qBlue(data[i]);
                c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
                data[i] = qRgba(qRed(data[i]), qGreen(data[i]), c, qAlpha(data[i]));
            }
        }
    }
    delete [] segTbl;

    return image;
}

// Modulate an image with an RBG channel of another image
//
QImage& KImageEffect::modulate(QImage &image, QImage &modImage, bool reverse,
	ModulationType type, int factor, RGBComponent channel)
{
    if (image.width() == 0 || image.height() == 0 ||
        modImage.width() == 0 || modImage.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::modulate : invalid image\n";
#endif
      return image;
    }

    int r, g, b, h, s, v, a;
    QColor clr;
    int mod=0;
    unsigned int x1, x2, y1, y2;
    register int x, y;

    // for image, we handle only depth 32
    if (image.depth()<32) image = image.convertDepth(32);

    // for modImage, we handle depth 8 and 32
    if (modImage.depth()<8) modImage = modImage.convertDepth(8);

    unsigned int *colorTable2 = (modImage.depth()==8) ?
				 modImage.colorTable():0;
    unsigned int *data1, *data2;
    unsigned char *data2b;
    unsigned int color1, color2;

    x1 = image.width();    y1 = image.height();
    x2 = modImage.width(); y2 = modImage.height();

    for (y = 0; y < (int)y1; y++) {
        data1 =  (unsigned int *) image.scanLine(y);
	data2 =  (unsigned int *) modImage.scanLine( y%y2 );
	data2b = (unsigned char *) modImage.scanLine( y%y2 );

	x=0;
	while(x < (int)x1) {
	  color2 = (colorTable2) ? colorTable2[*data2b] : *data2;
	  if (reverse) {
	      color1 = color2;
	      color2 = *data1;
	  }
	  else
	      color1 = *data1;

	  if (type == Intensity || type == Contrast) {
              r = qRed(color1);
	      g = qGreen(color1);
	      b = qBlue(color1);
	      if (channel != All) {
      	        mod = (channel == Red) ? qRed(color2) :
		    (channel == Green) ? qGreen(color2) :
	    	    (channel == Blue) ? qBlue(color2) :
		    (channel == Gray) ? qGray(color2) : 0;
	        mod = mod*factor/50;
	      }

	      if (type == Intensity) {
	        if (channel == All) {
	          r += r * factor/50 * qRed(color2)/256;
	          g += g * factor/50 * qGreen(color2)/256;
	          b += b * factor/50 * qBlue(color2)/256;
	        }
	        else {
	          r += r * mod/256;
	          g += g * mod/256;
	          b += b * mod/256;
	        }
	      }
	      else { // Contrast
	        if (channel == All) {
		  r += (r-128) * factor/50 * qRed(color2)/128;
	          g += (g-128) * factor/50 * qGreen(color2)/128;
	          b += (b-128) * factor/50 * qBlue(color2)/128;
	        }
	        else {
	          r += (r-128) * mod/128;
	          g += (g-128) * mod/128;
	          b += (b-128) * mod/128;
	        }
	      }

	      if (r<0) r=0; if (r>255) r=255;
	      if (g<0) g=0; if (g>255) g=255;
	      if (b<0) b=0; if (b>255) b=255;
	      a = qAlpha(*data1);
	      *data1 = qRgba(r, g, b, a);
	  }
	  else if (type == Saturation || type == HueShift) {
	      clr.setRgb(color1);
	      clr.hsv(&h, &s, &v);
      	      mod = (channel == Red) ? qRed(color2) :
		    (channel == Green) ? qGreen(color2) :
	    	    (channel == Blue) ? qBlue(color2) :
		    (channel == Gray) ? qGray(color2) : 0;
	      mod = mod*factor/50;

	      if (type == Saturation) {
		  s -= s * mod/256;
		  if (s<0) s=0; if (s>255) s=255;
	      }
	      else { // HueShift
	        h += mod;
		while(h<0) h+=360;
		h %= 360;
	      }

	      clr.setHsv(h, s, v);
	      a = qAlpha(*data1);
	      *data1 = clr.rgb() | ((uint)(a & 0xff) << 24);
	  }
	  data1++; data2++; data2b++; x++;
	  if ( (x%x2) ==0) { data2 -= x2; data2b -= x2; }
        }
    }
    return image;
}



//======================================================================
//
// Blend effects
//
//======================================================================

QImage& KImageEffect::blend(QImage &image, float initial_intensity,
                            const QColor &bgnd, GradientType eff,
                            bool anti_dir)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      cerr << "WARNING: KImageEffect::blend : invalid image\n";
#endif
      return image;
    }

    int r_bgnd = bgnd.red(), g_bgnd = bgnd.green(), b_bgnd = bgnd.blue();
    int r, g, b;
    int ind;

    unsigned int xi, xf, yi, yf;
    unsigned int a;

    // check the boundaries of the initial intesity param
    float unaffected = 1;
    if (initial_intensity >  1) initial_intensity =  1;
    if (initial_intensity < -1) initial_intensity = -1;
    if (initial_intensity < 0) {
        unaffected = 1. + initial_intensity;
        initial_intensity = 0;
    }


    float intensity = initial_intensity;
    float var = 1. - initial_intensity;

    if (anti_dir) {
        initial_intensity = intensity = 1.;
        var = -var;
    }

    register int x, y;

    unsigned int *data =  (unsigned int *)image.bits();

    if( eff == VerticalGradient || eff == HorizontalGradient ) {

        // set the image domain to apply the effect to
        xi = 0, xf = image.width();
        yi = 0, yf = image.height();
        if (eff == VerticalGradient) {
            if (anti_dir) yf = (int)(image.height() * unaffected);
            else yi = (int)(image.height() * (1 - unaffected));
        }
        else {
            if (anti_dir) xf = (int)(image.width() * unaffected);
            else xi = (int)(image.height() * (1 - unaffected));
        }

        var /= (eff == VerticalGradient?yf-yi:xf-xi);

        for (y = yi; y < (int)yf; y++) {
            intensity = eff == VerticalGradient? intensity + var :
                initial_intensity;
            for (x = xi; x < (int)xf ; x++) {
                if (eff == HorizontalGradient) intensity += var;
                ind = x + image.width()  * y ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);
            }
        }
    }
    else if (eff == DiagonalGradient  || eff == CrossDiagonalGradient) {
        float xvar = var / 2 / image.width();  // / unaffected;
        float yvar = var / 2 / image.height(); // / unaffected;
        float tmp;

        for (x = 0; x < image.width() ; x++) {
            tmp =  xvar * (eff == DiagonalGradient? x : image.width()-x-1);
            for (y = 0; y < image.height() ; y++) {
                intensity = initial_intensity + tmp + yvar * y;
                ind = x + image.width()  * y ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);
            }
        }
    }

    else if (eff == RectangleGradient || eff == EllipticGradient) {
        float xvar;
        float yvar;

        for (x = 0; x < image.width() / 2 + image.width() % 2; x++) {
            xvar = var / image.width()  * (image.width() - x*2/unaffected-1);
            for (y = 0; y < image.height() / 2 + image.height() % 2; y++) {
                yvar = var / image.height()   * (image.height() - y*2/unaffected -1);

                if (eff == RectangleGradient)
                    intensity = initial_intensity + QMAX(xvar, yvar);
                else
                    intensity = initial_intensity + sqrt(xvar * xvar + yvar * yvar);
                if (intensity > 1) intensity = 1;
                if (intensity < 0) intensity = 0;

                //NW
                ind = x + image.width()  * y ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);

                //NE
                ind = image.width() - x - 1 + image.width()  * y ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);
            }
        }

        //CT  loop is doubled because of stupid central row/column issue.
        //    other solution?
        for (x = 0; x < image.width() / 2; x++) {
            xvar = var / image.width()  * (image.width() - x*2/unaffected-1);
            for (y = 0; y < image.height() / 2; y++) {
                yvar = var / image.height()   * (image.height() - y*2/unaffected -1);

                if (eff == RectangleGradient)
                    intensity = initial_intensity + QMAX(xvar, yvar);
                else
                    intensity = initial_intensity + sqrt(xvar * xvar + yvar * yvar);
                if (intensity > 1) intensity = 1;
                if (intensity < 0) intensity = 0;

                //SW
                ind = x + image.width()  * (image.height() - y -1) ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);

                //SE
                ind = image.width()-x-1 + image.width() * (image.height() - y - 1) ;
                r = qRed  (data[ind]) + (int)(intensity *
                                              (r_bgnd - qRed  (data[ind])));
                g = qGreen(data[ind]) + (int)(intensity *
                                              (g_bgnd - qGreen(data[ind])));
                b = qBlue (data[ind]) + (int)(intensity *
                                              (b_bgnd - qBlue (data[ind])));
                if (r > 255) r = 255; if (r < 0 ) r = 0;
                if (g > 255) g = 255; if (g < 0 ) g = 0;
                if (b > 255) b = 255; if (b < 0 ) b = 0;
                a = qAlpha(data[ind]);
                data[ind] = qRgba(r, g, b, a);
            }
        }
    }
#ifndef NDEBUG
    else cerr << "KImageEffect::blend effect not implemented" << endl;
#endif
    return image;
}

// Not very efficient as we create a third big image...
//
QImage& KImageEffect::blend(QImage &image1, QImage &image2,
			    GradientType gt, int xf, int yf)
{
  if (image1.width() == 0 || image1.height() == 0 ||
      image2.width() == 0 || image2.height() == 0)
    return image1;

  QImage image3;

  image3 = KImageEffect::unbalancedGradient(image1.size(),
				    QColor(0,0,0), QColor(255,255,255),
				    gt, xf, yf, 0);

  return blend(image1,image2,image3, Red); // Channel to use is arbitrary
}

// Blend image2 into image1, using an RBG channel of blendImage
//
QImage& KImageEffect::blend(QImage &image1, QImage &image2,
			    QImage &blendImage, RGBComponent channel)
{
    if (image1.width() == 0 || image1.height() == 0 ||
        image2.width() == 0 || image2.height() == 0 ||
        blendImage.width() == 0 || blendImage.height() == 0) {
#ifndef NDEBUG
      cerr << "KImageEffect::blend effect invalid image" << endl;
#endif
      return image1;
    }

    int r, g, b;
    int ind1, ind2, ind3;

    unsigned int x1, x2, x3, y1, y2, y3;
    unsigned int a;

    register int x, y;

    // for image1 and image2, we only handle depth 32
    if (image1.depth()<32) image1 = image1.convertDepth(32);
    if (image2.depth()<32) image2 = image2.convertDepth(32);

    // for blendImage, we handle depth 8 and 32
    if (blendImage.depth()<8) blendImage = blendImage.convertDepth(8);

    unsigned int *colorTable3 = (blendImage.depth()==8) ?
				 blendImage.colorTable():0;

    unsigned int *data1 =  (unsigned int *)image1.bits();
    unsigned int *data2 =  (unsigned int *)image2.bits();
    unsigned int *data3   =  (unsigned int *)blendImage.bits();
    unsigned char *data3b =  (unsigned char *)blendImage.bits();
    unsigned int color3;

    x1 = image1.width();     y1 = image1.height();
    x2 = image2.width();     y2 = image2.height();
    x3 = blendImage.width(); y3 = blendImage.height();

    for (y = 0; y < (int)y1; y++) {
	ind1 = x1*y;
	ind2 = x2*(y%y2);
	ind3 = x3*(y%y3);

	x=0;
	while(x < (int)x1) {
	  color3 = (colorTable3) ? colorTable3[data3b[ind3]] : data3[ind3];

          a = (channel == Red) ? qRed(color3) :
              (channel == Green) ? qGreen(color3) :
	      (channel == Blue) ? qBlue(color3) : qGray(color3);

	  r = (a*qRed(data1[ind1]) + (256-a)*qRed(data2[ind2]))/256;
	  g = (a*qGreen(data1[ind1]) + (256-a)*qGreen(data2[ind2]))/256;
	  b = (a*qBlue(data1[ind1]) + (256-a)*qBlue(data2[ind2]))/256;

	  a = qAlpha(data1[ind1]);
	  data1[ind1] = qRgba(r, g, b, a);

	  ind1++; ind2++; ind3++; x++;
	  if ( (x%x2) ==0) ind2 -= x2;
	  if ( (x%x3) ==0) ind3 -= x3;
        }
    }
    return image1;
}


//======================================================================
//
// Hash effects
//
//======================================================================

unsigned int KImageEffect::lHash(unsigned int c)
{
    unsigned char r = qRed(c), g = qGreen(c), b = qBlue(c), a = qAlpha(c);
    unsigned char nr, ng, nb;
    nr =(r >> 1) + (r >> 2); nr = nr > r ? 0 : nr;
    ng =(g >> 1) + (g >> 2); ng = ng > g ? 0 : ng;
    nb =(b >> 1) + (b >> 2); nb = nb > b ? 0 : nb;

    return qRgba(nr, ng, nb, a);
}


// -----------------------------------------------------------------------------

unsigned int KImageEffect::uHash(unsigned int c)
{
    unsigned char r = qRed(c), g = qGreen(c), b = qBlue(c), a = qAlpha(c);
    unsigned char nr, ng, nb;
    nr = r + (r >> 3); nr = nr < r ? ~0 : nr;
    ng = g + (g >> 3); ng = ng < g ? ~0 : ng;
    nb = b + (b >> 3); nb = nb < b ? ~0 : nb;

    return qRgba(nr, ng, nb, a);
}


// -----------------------------------------------------------------------------

QImage& KImageEffect::hash(QImage &image, Lighting lite, unsigned int spacing)
{
    if (image.width() == 0 || image.height() == 0) {
#ifndef NDEBUG
      cerr << "KImageEffect::hash effect invalid image" << endl;
#endif
      return image;
    }

    register int x, y;
    unsigned int *data =  (unsigned int *)image.bits();
    unsigned int ind;

    //CT no need to do it if not enough space
    if ((lite == NorthLite ||
         lite == SouthLite)&&
        (unsigned)image.height() < 2+spacing) return image;
    if ((lite == EastLite ||
         lite == WestLite)&&
        (unsigned)image.height() < 2+spacing) return image;

    if (lite == NorthLite || lite == SouthLite) {
        for (y = 0 ; y < image.height(); y = y + 2 + spacing) {
            for (x = 0; x < image.width(); x++) {
                ind = x + image.width() * y;
                data[ind] = lite==NorthLite?uHash(data[ind]):lHash(data[ind]);

                ind = ind + image.width();
                data[ind] = lite==NorthLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == EastLite || lite == WestLite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0; x < image.width(); x = x + 2 + spacing) {
                ind = x + image.width() * y;
                data[ind] = lite==EastLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==EastLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == NWLite || lite == SELite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0;
                 x < (int)(image.width() - ((y & 1)? 1 : 0) * spacing);
                 x = x + 2 + spacing) {
                ind = x + image.width() * y + ((y & 1)? 1 : 0);
                data[ind] = lite==NWLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==NWLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    else if (lite == SWLite || lite == NELite) {
        for (y = 0 ; y < image.height(); y++) {
            for (x = 0  + ((y & 1)? 1 : 0); x < image.width(); x = x + 2 + spacing) {
                ind = x + image.width() * y - ((y & 1)? 1 : 0);
                data[ind] = lite==SWLite?uHash(data[ind]):lHash(data[ind]);

                ind++;
                data[ind] = lite==SWLite?lHash(data[ind]):uHash(data[ind]);
            }
        }
    }

    return image;
}


//======================================================================
//
// Flatten effects
//
//======================================================================

QImage& KImageEffect::flatten(QImage &img, const QColor &ca,
                            const QColor &cb, int ncols)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    // a bitmap is easy...
    if (img.depth() == 1) {
	img.setColor(0, ca.rgb());
	img.setColor(1, cb.rgb());
	return img;
    }

    int r1 = ca.red(); int r2 = cb.red();
    int g1 = ca.green(); int g2 = cb.green();
    int b1 = ca.blue(); int b2 = cb.blue();
    int min = 0, max = 255;

    QRgb col;
    
    // Get minimum and maximum greylevel.
    if (img.numColors()) {
	// pseudocolor
	for (int i = 0; i < img.numColors(); i++) {
	    col = img.color(i);
	    int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
	    min = QMIN(min, mean);
	    max = QMAX(max, mean);
	}
    } else {
	// truecolor
	for (int y=0; y < img.height(); y++)
	    for (int x=0; x < img.width(); x++) {
		col = img.pixel(x, y);
		int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
		min = QMIN(min, mean);
		max = QMAX(max, mean);
	    }
    }

    // Conversion factors
    float sr = ((float) r2 - r1) / (max - min);
    float sg = ((float) g2 - g1) / (max - min);
    float sb = ((float) b2 - b1) / (max - min);


    // Repaint the image
    if (img.numColors()) {
	for (int i=0; i < img.numColors(); i++) {
	    col = img.color(i);
	    int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
	    int r = (int) (sr * (mean - min) + r1 + 0.5);
	    int g = (int) (sg * (mean - min) + g1 + 0.5);
	    int b = (int) (sb * (mean - min) + b1 + 0.5);
	    img.setColor(i, qRgba(r, g, b, qAlpha(col)));
	}
    } else {
	for (int y=0; y < img.height(); y++)
	    for (int x=0; x < img.width(); x++) {
		col = img.pixel(x, y);
		int mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
		int r = (int) (sr * (mean - min) + r1 + 0.5);
		int g = (int) (sg * (mean - min) + g1 + 0.5);
		int b = (int) (sb * (mean - min) + b1 + 0.5);
		img.setPixel(x, y, qRgba(r, g, b, qAlpha(col)));
	    }
    }


    // Dither if necessary
    if ( (ncols <= 0) || ((img.numColors() != 0) && (img.numColors() <= ncols)))
	return img;

    if (ncols == 1) ncols++;
    if (ncols > 256) ncols = 256;

    QColor *pal = new QColor[ncols];
    sr = ((float) r2 - r1) / (ncols - 1);
    sg = ((float) g2 - g1) / (ncols - 1);
    sb = ((float) b2 - b1) / (ncols - 1);

    for (int i=0; i<ncols; i++)
	pal[i] = QColor(r1 + int(sr*i), g1 + int(sg*i), b1 + int(sb*i));

    dither(img, pal, ncols);

    delete[] pal;
    return img;
}


//======================================================================
//
// Fade effects
//
//======================================================================

QImage& KImageEffect::fade(QImage &img, float val, const QColor &color)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    // We don't handle bitmaps
    if (img.depth() == 1)
	return img;

    unsigned char tbl[256];
    for (int i=0; i<256; i++)
	tbl[i] = (int) (val * i + 0.5);

    int red = color.red();
    int green = color.green();
    int blue = color.blue();

    QRgb col;
    int r, g, b, cr, cg, cb;

    if (img.depth() <= 8) {
	// pseudo color
	for (int i=0; i<img.numColors(); i++) {
	    col = img.color(i);
	    cr = qRed(col); cg = qGreen(col); cb = qBlue(col);
	    if (cr > red)
		r = cr - tbl[cr - red];
	    else
		r = cr + tbl[red - cr];
	    if (cg > green)
		g = cg - tbl[cg - green];
	    else
		g = cg + tbl[green - cg];
	    if (cb > blue)
		b = cb - tbl[cb - blue];
	    else
		b = cb + tbl[blue - cb];
	    img.setColor(i, qRgba(r, g, b, qAlpha(col)));
	}

    } else {
	// truecolor
        for (int y=0; y<img.height(); y++) {
            QRgb *data = (QRgb *) img.scanLine(y);
            for (int x=0; x<img.width(); x++) {
                col = *data;
                cr = qRed(col); cg = qGreen(col); cb = qBlue(col);
                if (cr > red)
                    r = cr - tbl[cr - red];
                else
                    r = cr + tbl[red - cr];
                if (cg > green)
                    g = cg - tbl[cg - green];
                else
                    g = cg + tbl[green - cg];
                if (cb > blue)
                    b = cb - tbl[cb - blue];
                else
                    b = cb + tbl[blue - cb];
                *data++ = qRgba(r, g, b, qAlpha(col));
            }
        }
    }

    return img;
}

//======================================================================
//
// Color effects
//
//======================================================================

// This code is adapted from code (C) Rik Hemsley <rik@kde.org>
//
// The formula used (r + b + g) /3 is different from the qGray formula
// used by Qt.  This is because our formula is much much faster.  If,
// however, it turns out that this is producing sub-optimal images,
// then it will have to change (kurt)
//
// It does produce lower quality grayscale ;-) Use fast == true for the fast
// algorithm, false for the higher quality one (mosfet).
QImage& KImageEffect::toGray(QImage &img, bool fast)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if(fast){
        if (img.depth() == 32) {
            register uchar * r(img.bits());
            register uchar * g(img.bits() + 1);
            register uchar * b(img.bits() + 2);

            uchar * end(img.bits() + img.numBytes());

            while (r != end) {

                *r = *g = *b = (((*r + *g) >> 1) + *b) >> 1; // (r + b + g) / 3

                r += 4;
                g += 4;
                b += 4;
            }
        }
        else
        {
            for (int i = 0; i < img.numColors(); i++)
            {
                register uint r = qRed(img.color(i));
                register uint g = qGreen(img.color(i));
                register uint b = qBlue(img.color(i));

                register uint gray = (((r + g) >> 1) + b) >> 1;
                img.setColor(i, qRgba(gray, gray, gray, qAlpha(img.color(i))));
            }
        }
    }
    else{
        int pixels = img.depth() > 8 ? img.width()*img.height() :
            img.numColors();
        unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
            (unsigned int *)img.colorTable();
        int val, i;
        for(i=0; i < pixels; ++i){
            val = qGray(data[i]);
            data[i] = qRgba(val, val, val, qAlpha(data[i]));
        }
    }
    return img;
}

// CT 29Jan2000 - desaturation algorithms
QImage& KImageEffect::desaturate(QImage &img, float desat)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if (desat < 0) desat = 0.;
    if (desat > 1) desat = 1.;
    int pixels = img.depth() > 8 ? img.width()*img.height() :
        img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
        (unsigned int *)img.colorTable();
    int h, s, v, i;
    QColor clr; // keep constructor out of loop (mosfet)
    for(i=0; i < pixels; ++i){
        clr.setRgb(data[i]);
	clr.hsv(&h, &s, &v);
	clr.setHsv(h, (int)(s * (1. - desat)), v);
	data[i] = clr.rgb();
    }
    return img;
}

// Contrast stuff (mosfet)
QImage& KImageEffect::contrast(QImage &img, int c)
{
    if (img.width() == 0 || img.height() == 0)
      return img;

    if(c > 255)
        c = 255;
    if(c < -255)
        c =  -255;
    int pixels = img.depth() > 8 ? img.width()*img.height() :
        img.numColors();
    unsigned int *data = img.depth() > 8 ? (unsigned int *)img.bits() :
        (unsigned int *)img.colorTable();
    int i, r, g, b;
    for(i=0; i < pixels; ++i){
        r = qRed(data[i]);
        g = qGreen(data[i]);
        b = qBlue(data[i]);
        if(qGray(data[i]) <= 127){
            if(r - c <= 255)
                r -= c;
            if(g - c <= 255)
                g -= c;
            if(b - c <= 255)
                b -= c;
        }
        else{
            if(r + c <= 255)
                r += c;
            if(g + c <= 255)
                g += c;
            if(b + c <= 255)
                b += c;
        }
        data[i] = qRgba(r, g, b, qAlpha(data[i]));
    }
    return(img);
}

//======================================================================
//
// Dithering effects
//
//======================================================================

// adapted from kFSDither (C) 1997 Martin Jones (mjones@kde.org)
//
// Floyd-Steinberg dithering
// Ref: Bitmapped Graphics Programming in C++
//      Marv Luse, Addison-Wesley Publishing, 1993.
QImage& KImageEffect::dither(QImage &img, const QColor *palette, int size)
{
    if (img.width() == 0 || img.height() == 0 ||
        palette == 0 || img.depth() <= 8)
      return img;

    QImage dImage( img.width(), img.height(), 8, size );
    int i;

    dImage.setNumColors( size );
    for ( i = 0; i < size; i++ )
        dImage.setColor( i, palette[ i ].rgb() );

    int *rerr1 = new int [ img.width() * 2 ];
    int *gerr1 = new int [ img.width() * 2 ];
    int *berr1 = new int [ img.width() * 2 ];

    memset( rerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( gerr1, 0, sizeof( int ) * img.width() * 2 );
    memset( berr1, 0, sizeof( int ) * img.width() * 2 );

    int *rerr2 = rerr1 + img.width();
    int *gerr2 = gerr1 + img.width();
    int *berr2 = berr1 + img.width();

    for ( int j = 0; j < img.height(); j++ )
    {
        uint *ip = (uint * )img.scanLine( j );
        uchar *dp = dImage.scanLine( j );

        for ( i = 0; i < img.width(); i++ )
        {
            rerr1[i] = rerr2[i] + qRed( *ip );
            rerr2[i] = 0;
            gerr1[i] = gerr2[i] + qGreen( *ip );
            gerr2[i] = 0;
            berr1[i] = berr2[i] + qBlue( *ip );
            berr2[i] = 0;
            ip++;
        }

        *dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0], palette, size );

        for ( i = 1; i < img.width()-1; i++ )
        {
            int indx = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
            *dp = indx;

            int rerr = rerr1[i];
            rerr -= palette[indx].red();
            int gerr = gerr1[i];
            gerr -= palette[indx].green();
            int berr = berr1[i];
            berr -= palette[indx].blue();

            // diffuse red error
            rerr1[ i+1 ] += ( rerr * 7 ) >> 4;
            rerr2[ i-1 ] += ( rerr * 3 ) >> 4;
            rerr2[  i  ] += ( rerr * 5 ) >> 4;
            rerr2[ i+1 ] += ( rerr ) >> 4;

            // diffuse green error
            gerr1[ i+1 ] += ( gerr * 7 ) >> 4;
            gerr2[ i-1 ] += ( gerr * 3 ) >> 4;
            gerr2[  i  ] += ( gerr * 5 ) >> 4;
            gerr2[ i+1 ] += ( gerr ) >> 4;

            // diffuse red error
            berr1[ i+1 ] += ( berr * 7 ) >> 4;
            berr2[ i-1 ] += ( berr * 3 ) >> 4;
            berr2[  i  ] += ( berr * 5 ) >> 4;
            berr2[ i+1 ] += ( berr ) >> 4;

            dp++;
        }

        *dp = nearestColor( rerr1[i], gerr1[i], berr1[i], palette, size );
    }

    delete [] rerr1;
    delete [] gerr1;
    delete [] berr1;

    img = dImage;
    return img;
}

int KImageEffect::nearestColor( int r, int g, int b, const QColor *palette, int size )
{
    if (palette == 0)
      return 0;

    int dr = palette[0].red() - r;
    int dg = palette[0].green() - g;
    int db = palette[0].blue() - b;

    int minDist =  dr*dr + dg*dg + db*db;
    int nearest = 0;

    for (int i = 1; i < size; i++ )
    {
        dr = palette[i].red() - r;
        dg = palette[i].green() - g;
        db = palette[i].blue() - b;

        int dist = dr*dr + dg*dg + db*db;

        if ( dist < minDist )
        {
            minDist = dist;
            nearest = i;
        }
    }

    return nearest;
}

bool KImageEffect::blend(
    const QImage & upper,
    const QImage & lower,
    QImage & output
)
{
  if (
      upper.width()  > lower.width()  ||
      upper.height() > lower.height() ||
      upper.depth() != 32             ||
      lower.depth() != 32
  )
  {
#ifndef NDEBUG
    cerr << "KImageEffect::blend : Sizes not correct\n" ;
#endif
    return false;
  }

  output = lower.copy();

  register uchar *i, *o;
  register int a;
  register int col;
  register int w = upper.width();
  int row(upper.height() - 1);

  do {

    i = upper.scanLine(row);
    o = output.scanLine(row);

    col = w << 2;
    --col;

    do {

      while (!(a = i[col]) && (col != 3)) {
        --col; --col; --col; --col;
      }

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

      --col;
      o[col] += ((i[col] - o[col]) * a) >> 8;

    } while (col--);

  } while (row--);

  return true;
}

#if 0
// Not yet...
bool KImageEffect::blend(
    const QImage & upper,
    const QImage & lower,
    QImage & output,
    const QRect & destRect
)
{
  output = lower.copy();
  return output;
}

#endif

bool KImageEffect::blend(
    int &x, int &y,
    const QImage & upper,
    const QImage & lower,
    QImage & output
)
{
  int cx=0, cy=0, cw=upper.width(), ch=upper.height();

  if ( upper.width() + x > lower.width()  ||
      upper.height() + y > lower.height() ||
      x < 0 || y < 0 ||
      upper.depth() != 32 || lower.depth() != 32 )
  {
    if ( x > lower.width() || y > lower.height() ) return false;
    if ( upper.width()<=0 || upper.height() <= 0 ) return false;
    if ( lower.width()<=0 || lower.height() <= 0 ) return false;

    if (x<0) {cx=-x; cw+=x; x=0; };
    if (cw + x > lower.width()) { cw=lower.width()-x; };
    if (y<0) {cy=-y; ch+=y; y=0; };
    if (ch + y > lower.height()) { ch=lower.height()-y; };

    if ( cx >= upper.width() || cy >= upper.height() ) return true;
    if ( cw <= 0 || ch <= 0 ) return true;
  }

  output.create(cw,ch,32);
//  output.setAlphaBuffer(true); // I should do some benchmarks to see if
	// this is worth the effort

  register QRgb *i, *o, *b;

  register int a;
  register int j,k;
  for (j=0; j<ch; j++)
  {
    b=reinterpret_cast<QRgb *>(&lower.scanLine(y+j) [ (x+cw) << 2 ]);
    i=reinterpret_cast<QRgb *>(&upper.scanLine(cy+j)[ (cx+cw) << 2 ]);
    o=reinterpret_cast<QRgb *>(&output.scanLine(j)  [ cw << 2 ]);

    k=cw-1;
    --b; --i; --o;
    do
    {
      while ( !(a=qAlpha(*i)) && k>0 )
      {
        i--;
//	*o=0;
	*o=*b;
	--o; --b;
	k--;
      };
//      *o=0xFF;
      *o = qRgb(qRed(*b) + (((qRed(*i) - qRed(*b)) * a) >> 8),
                qGreen(*b) + (((qGreen(*i) - qGreen(*b)) * a) >> 8),
                qBlue(*b) + (((qBlue(*i) - qBlue(*b)) * a) >> 8));
      --i; --o; --b;
    } while (k--);
  }

  return true;
}

bool KImageEffect::blendOnLower(
    int x, int y,
    const QImage & upper,
    const QImage & lower
)
{
  int cx=0, cy=0, cw=upper.width(), ch=upper.height();

  if ( upper.depth() != 32 || lower.depth() != 32 ) return false;
  if ( x + cw > lower.width()  ||
      y + ch > lower.height() ||
      x < 0 || y < 0 )
  {
    if ( x > lower.width() || y > lower.height() ) return true;
    if ( upper.width()<=0 || upper.height() <= 0 ) return true;
    if ( lower.width()<=0 || lower.height() <= 0 ) return true;

    if (x<0) {cx=-x; cw+=x; x=0; };
    if (cw + x > lower.width()) { cw=lower.width()-x; };
    if (y<0) {cy=-y; ch+=y; y=0; };
    if (ch + y > lower.height()) { ch=lower.height()-y; };

    if ( cx >= upper.width() || cy >= upper.height() ) return true;
    if ( cw <= 0 || ch <= 0 ) return true;
  }

  register uchar *i, *b;
  register int a;
  register int k;

  for (int j=0; j<ch; j++)
  {
    b=&lower.scanLine(y+j) [ (x+cw) << 2 ];
    i=&upper.scanLine(cy+j)[ (cx+cw) << 2 ];

    k=cw-1;
    --b; --i;
    do
    {
      while ( !(a=*i) && k>0 )
      {
        i-=4; b-=4; k--;
      };

      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
      *b += ( ((*i - *b) * a) >> 8 );
      --i; --b;
    } while (k--);
  }

  return true;
}

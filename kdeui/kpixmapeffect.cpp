#include "kpixmapeffect.h"
#include <qimage.h>
#include <qpainter.h>
#include <dither.h>
#include <qcolor.h>

#include <math.h>


void KPixmapEffect::gradient(KPixmap &pixmap, const QColor &ca,
                                     const QColor &cb, GradientType eff,
                                     int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;
    
    QImage image;
    
    register int x, y;
    
    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());
    
    if( eff == VerticalGradient || eff == HorizontalGradient ){
        QPixmap pmCrop;
        
        uint *p;
        uint rgb;

        pmCrop.resize((eff == HorizontalGradient ? pixmap.width()  : 30),
                      (eff == HorizontalGradient ? 30 : pixmap.height()));
        QImage image( (eff == HorizontalGradient ? pixmap.width()  : 30),
                      (eff == HorizontalGradient ? 30 : pixmap.height()), 32 );

        register int rl = rca << 16;
        register int gl = gca << 16;
        register int bl = bca << 16;
        
        if( eff == VerticalGradient ) {

            int rcdelta = ((1<<16) / pixmap.height()) * rDiff;
            int gcdelta = ((1<<16) / pixmap.height()) * gDiff;
            int bcdelta = ((1<<16) / pixmap.height()) * bDiff;
            
            for ( y = 0; y < pixmap.height(); y++ ) {
                p = (uint *) image.scanLine(y);
                
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;
                
                rgb = qRgb( (rl>>16), (gl>>16), (bl>>16) );
                
                for( x = 0; x < 30; x++ ) {
                    *p = rgb;
                    p++;
                }
            }
            
        }
        else {                  // must be HorizontalGradient
            
            unsigned int *o_src = (unsigned int *)image.scanLine(0);
            unsigned int *src = o_src;

            int rcdelta = ((1<<16) / pixmap.width()) * rDiff;
            int gcdelta = ((1<<16) / pixmap.width()) * gDiff;
            int bcdelta = ((1<<16) / pixmap.width()) * bDiff;

            for( x = 0; x < pixmap.width(); x++) {
                
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;
                
                *src++ = qRgb( (rl>>16), (gl>>16), (bl>>16));
            }
            
            src = o_src;
            
            // Believe it or not, manually copying in a for loop is faster
            // than calling memcpy for each scanline (on the order of ms...).
            // I think this is due to the function call overhead (mosfet).
            
            for(y = 1; y < 30; ++y) {
                
                p = (unsigned int *)image.scanLine(y);
                src = o_src;
                for(x=0; x < pixmap.width(); ++x)
                    *p++ = *src++;
            }
        }
        
        if(pmCrop.depth() < 15 ) {
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
            pmCrop.convertFromImage(image);
            delete [] dPal;
        }
        else
            pmCrop.convertFromImage( image );

        // Copy the cropped pixmap into the KPixmap.
        // Extract only a central column from the cropped pixmap
        // to avoid edge effects.
        
        int sSize = 20;
        int sOffset = 5;
        
        int s = ((eff == HorizontalGradient ? pixmap.height() : pixmap.width()) / sSize) + 1;
        
        QPainter paint;
        paint.begin( &pixmap );

        if(eff == HorizontalGradient) {
            for( int i=0; i<s; i++)
                paint.drawPixmap(0, sSize*i, pmCrop, 0, sOffset);
        }
        else  {
            for( int i=0; i<s; i++ )
                paint.drawPixmap( sSize*i, 0, pmCrop, sOffset, 0 , sSize, pixmap.height() );
        }
        
        paint.end();
        
    }
    
    else{
        
        float rfd, gfd, bfd;
        float rd = rca, gd = gca, bd = bca;

        QImage image(pixmap.width(), pixmap.height(), 32);
        unsigned char *xtable[3];
        unsigned char *ytable[3];

        unsigned int w = pixmap.width(), h = pixmap.height();
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
            for (x = 0; x < pixmap.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
                dir = eff == DiagonalGradient? x : pixmap.width() - x - 1;
                xtable[0][dir] = (unsigned char) rd;
                xtable[1][dir] = (unsigned char) gd;
                xtable[2][dir] = (unsigned char) bd;
            }
            rfd = (float)rDiff/h;
            gfd = (float)gDiff/h;
            bfd = (float)bDiff/h;
            rd = gd = bd = 0;
            for (y = 0; y < pixmap.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
                ytable[0][y] = (unsigned char) rd;
                ytable[1][y] = (unsigned char) gd;
                ytable[2][y] = (unsigned char) bd;
            }
            
            for (y = 0; y < pixmap.height(); y++) {
                unsigned int *scanline = (unsigned int *)image.scanLine(y);
                for (x = 0; x < pixmap.width(); x++) {
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
            
            rfd = (float)rDiff / pixmap.width();
            gfd = (float)gDiff / pixmap.width();
            bfd = (float)bDiff / pixmap.width();
            
            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;
            
            for (x = 0; x < pixmap.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd) 
            {
                xtable[0][x] = (unsigned char) abs((int)rd);
                xtable[1][x] = (unsigned char) abs((int)gd);
                xtable[2][x] = (unsigned char) abs((int)bd);
            }
            
            rfd = (float)rDiff/pixmap.height();
            gfd = (float)gDiff/pixmap.height();
            bfd = (float)bDiff/pixmap.height();
            
            rd = (float)rDiff/2;
            gd = (float)gDiff/2;
            bd = (float)bDiff/2;
            
            for (y = 0; y < pixmap.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd) 
            {
                ytable[0][y] = (unsigned char) abs((int)rd);
                ytable[1][y] = (unsigned char) abs((int)gd);
                ytable[2][y] = (unsigned char) abs((int)bd);
            }
            unsigned int rgb;
            int h = (pixmap.height()+1)>>1;
            for (y = 0; y < h; y++) {
                unsigned int *sl1 = (unsigned int *)image.scanLine(y);
                unsigned int *sl2 = (unsigned int *)image.scanLine(QMAX(pixmap.height()-y-1, y));
                
                int w = (pixmap.width()+1)>>1;
                int x2 = pixmap.width()-1;
                    
                for (x = 0; x < w; x++, x2--) {
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
                                              ytable[0][y]*ytable[0][y])*2),
                                   gcb - gSign *
                                   (int)sqrt((xtable[1][x]*xtable[1][x] +
                                              ytable[1][y]*ytable[1][y])*2),
                                   bcb - bSign *
                                   (int)sqrt((xtable[2][x]*xtable[2][x] +
                                              ytable[2][y]*ytable[2][y])*2));
                    }
                    
                    sl1[x] = sl2[x] = rgb;
                    sl1[x2] = sl2[x2] = rgb;
                }
            }
        }
        
        if(pixmap.depth() < 15 ) {
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

        delete [] xtable[0];
        delete [] xtable[1];
        delete [] xtable[2];
        delete [] ytable[0];
        delete [] ytable[1];
        delete [] ytable[2];
    }
}


//CT this was (before Dirk A. Mueller's speedup changes) 
//   merely the same code as in the above method, but it's supposedly
//   way less performant since it introduces a lot of supplementary tests
//   and simple math operations for the calculus of the balance.
//      (surprizingly, it isn't less performant, in the contrary :-)
//   Yes, I could have merged them, but then the excellent performance of
//   the balanced code would suffer with no other gain than a mere
//   source code and byte code size economy.

void KPixmapEffect::unbalancedGradient(KPixmap &pixmap, const QColor &ca,
                                     const QColor &cb, GradientType eff,
                                     int xfactor, int yfactor, int ncols)
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


    double xbal = xfactor/5000.;
    double ybal = yfactor/5000.;
    double rat;

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image;

    register int x, y;
    unsigned int *scanline;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient || eff == HorizontalGradient){
        QPixmap pmCrop;
        QColor cRow;

        uint *p;
        uint rgbRow;

        pmCrop.resize((eff == HorizontalGradient ? pixmap.width()  : 30),
                      (eff == HorizontalGradient ? 30 : pixmap.height()));
        QImage image( (eff == HorizontalGradient ? pixmap.width()  : 30),
                      (eff == HorizontalGradient ? 30 : pixmap.height()), 32 );

	if( eff == VerticalGradient) { 
	  for ( y = 0; y < pixmap.height(); y++ ) {
	    dir = _yanti ? y : pixmap.height() - 1 - y;
            p = (uint *) image.scanLine(dir);
            rat =  1 - exp( - (double)y  * ybal );
	    
            cRow.setRgb( rcb - (int) ( rDiff * rat ),
                         gcb - (int) ( gDiff * rat ),
                         bcb - (int) ( bDiff * rat ) );
	    
            rgbRow = cRow.rgb();
	    
            for( x = 0; x < 30; x++ ) {
	      *p = rgbRow;
	      p++;
            }
	  }
	}
	else {

	  unsigned int *src = (unsigned int *)image.scanLine(0);
	  for(x = 0; x < pixmap.width(); x++ ) 
	    {
	      dir = _xanti ? x : pixmap.width() - 1 - x;
	      rat = 1 - exp( - (double)x  * xbal );
	      
	      src[dir] = qRgb(rcb - (int) ( rDiff * rat ),
			    gcb - (int) ( gDiff * rat ),
			    bcb - (int) ( bDiff * rat ));
	    }
	  
	  // Believe it or not, manually copying in a for loop is faster
	  // than calling memcpy for each scanline (on the order of ms...).
	  // I think this is due to the function call overhead (mosfet).
	  
	  for(y = 1; y < 30; ++y)
	    {
	      scanline = (unsigned int *)image.scanLine(y);
	      for(x=0; x < pixmap.width(); ++x)
		scanline[x] = src[x];
	    }
	}
	  
	if(pmCrop.depth() < 15 ) {
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
	  pmCrop.convertFromImage(image);
	  delete [] dPal;
	}
	else
	  pmCrop.convertFromImage( image );
	
        // Copy the cropped pixmap into the KPixmap.
        // Extract only a central column from the cropped pixmap
        // to avoid edge effects.

        int sSize = 20;
        int sOffset = 5;

        int s = ((eff == HorizontalGradient ? 
		  pixmap.height() : 
		  pixmap.width()) / sSize) + 1;

        QPainter paint;
        paint.begin( &pixmap );

        if(eff == HorizontalGradient) {
	  for( int i=0; i<s; i++)
	    paint.drawPixmap(0, sSize*i, pmCrop, 0, sOffset);
        }
        else  {
	  for( int i=0; i<s; i++ )
	    paint.drawPixmap( sSize*i, 0, 
			      pmCrop, sOffset, 0 , sSize, pixmap.height() );
	}
	paint.end();

    }
    
    else {
      int w=pixmap.width(), h=pixmap.height();
      QImage image(w, h, 32);

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
              rat = 1 - exp( - (double)x * xbal );

              xtable[0][dir] = (unsigned char) ( rDiff/2 * rat );
              xtable[1][dir] = (unsigned char) ( gDiff/2 * rat );
              xtable[2][dir] = (unsigned char) ( bDiff/2 * rat );
          }
	  
	  for (y = 0; y < h; y++) {
              dir = _yanti ? y : h - 1 - y;
              rat =  1 - exp( - (double)y  * ybal );

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
                rat =  1 - exp( - (double)x * xbal );

                xtable[0][dir] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
                xtable[1][dir] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
                xtable[2][dir] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
            }

          for (y = 0; y < h; y++)
          {
              dir = _yanti ? y : h - 1 - y;

              rat =  1 - exp( - (double)y * ybal );

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
                                                    ytable[0][y]*ytable[0][y])*2),
                                         gcb - gSign *
                                         (int)sqrt((xtable[1][x]*xtable[1][x] +
                                                    ytable[1][y]*ytable[1][y])*2),
                                         bcb - bSign *
                                         (int)sqrt((xtable[2][x]*xtable[2][x] +
                                                    ytable[2][y]*ytable[2][y])*2));
                  }
              }
          }
      }

      if(pixmap.depth() < 15 ) {
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

      delete [] xtable[0];
      delete [] xtable[1];
      delete [] xtable[2];
      delete [] ytable[0];
      delete [] ytable[1];
      delete [] ytable[2];

    }
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
  unsigned char *segTbl = new unsigned char[segColors];
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
  delete [] segTbl;
}

void KPixmapEffect::channelIntensity(QImage &image, float percent,
				     RGBComponent channel,
				     bool brighten)
{
  int i, tmp, c;
  int segColors = image.depth() > 8 ? 256 : image.numColors();
  unsigned char *segTbl = new unsigned char[segColors];
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
  delete [] segTbl;
}


void KPixmapEffect::blend(QImage &image, float initial_intensity,
			  const QColor &bgnd, GradientType eff,
			  int ncols)
{
    int r_bgnd = bgnd.red(), g_bgnd = bgnd.green(), b_bgnd = bgnd.blue();
    int r, g, b;
    int ind;
    float intensity = initial_intensity, var = 1. - initial_intensity;

    register int x, y;
    
    unsigned int *data =  (unsigned int *)image.bits();
    
    if( eff == VerticalGradient ||
	eff == HorizontalGradient) {
      var /= (eff == VerticalGradient?image.height():image.width());
      for (y = 0; y < image.height() ; y++) {
	intensity = eff == VerticalGradient? intensity + var : 
		                             initial_intensity;
	for (x = 0; x < image.width() ; x++) {
	  if (eff == HorizontalGradient) intensity += var;
	  ind = x + image.width()  * y ;
	  r = qRed  (data[ind]) + (int)(intensity * 
					(r_bgnd - qRed  (data[ind]))); 
	  g = qGreen(data[ind]) + (int)(intensity * 
					(g_bgnd - qGreen(data[ind]))); 
	  b = qBlue (data[ind]) + (int)(intensity * 
					(b_bgnd - qBlue (data[ind])));
	  if (r > 255) r = 255; if (r < r_bgnd ) r = r_bgnd;
	  if (g > 255) g = 255; if (g < g_bgnd ) g = g_bgnd;
	  if (b > 255) b = 255; if (b < b_bgnd ) b = b_bgnd;
	  data[ind]=qRgb(r, g, b);
	}
      }
    }
    else {
      float xvar = var / 2 / image.width();
      float yvar = var / 2 / image.height();
      if (eff == DiagonalGradient  || eff == CrossDiagonalGradient) {
	for (x = 0; x < image.width() ; x++) {
	  intensity = initial_intensity + xvar * 
	    (eff == DiagonalGradient? x : image.width() - x -1);
	  for (y = 0; y < image.height() ; y++) {
	    intensity += yvar;
	    ind = x + image.width()  * y ;
	    r = qRed  (data[ind]) + (int)(intensity * 
					  (r_bgnd - qRed  (data[ind]))); 
	    g = qGreen(data[ind]) + (int)(intensity * 
					  (g_bgnd - qGreen(data[ind]))); 
	    b = qBlue (data[ind]) + (int)(intensity * 
					  (b_bgnd - qBlue (data[ind])));
	    if (r > 255) r = 255; if (r < r_bgnd ) r = r_bgnd;
	    if (g > 255) g = 255; if (g < g_bgnd ) g = g_bgnd;
	    if (b > 255) b = 255; if (b < b_bgnd ) b = b_bgnd;
	    data[ind]=qRgb(r, g, b);
	  }
	}
      }
      else debug("not yet implemented");
    }
    
}

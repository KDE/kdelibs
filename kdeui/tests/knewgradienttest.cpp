#include "knewgradienttest.h"
#include <kapp.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>

int cols = 3, rows = 3; // how many

// -----------------------------------------------------------------------------
// Kpixmapeffect.h follows, modified

/*
 * (C)Copyright 1999
 * Cristian Tibirna  <ctibirna@total.net>
 * Daniel M. Duley <mosfet@kde.org>
 * Dirk A. Mueller <dmuell@gmx.net>
 *
 */

#include <kpixmap.h>
#include <qimage.h>

/**
 * This class includes various pixmap based graphical effects. Everything is
 * static, so there is no need to create an instance of this class. You can
 * just call the static methods. They are encapsulated here merely to provide
 * a common namespace.
 */
class KNewPixmapEffect
{
public:
    enum GradientType { VerticalGradient, HorizontalGradient,
			DiagonalGradient, CrossDiagonalGradient,
			PyramidGradient, RectangleGradient, 
			PipeCrossGradient, EllipticGradient };
    enum RGBComponent { Red, Green, Blue };

    enum Lighting {NorthLite, NWLite, WestLite, SWLite, 
		   SouthLite, SELite, EastLite, NELite}; 

    /**
     * Create a gradient from color a to color b of the specified type.
     *
     * @param size The desired size of the gradient.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param ncols The number of colors to use when not running on a
     * truecolor display. The gradient will be dithered to this number of
     * colors. Pass 0 to prevent dithering.
     */
    static QImage gradient(const QSize &size, const QColor &ca, 
	    const QColor &cb, GradientType type, int ncols=3);

    /**
     * The same as above, only works on a KPixmap now.
     */
    static void gradient(KPixmap &pixmap, const QColor &ca,
                                 const QColor &cb, GradientType type,
                                 int ncols=3);

    /**
     * Create an unbalanced gradient.
     * An unbalanced gradient is a gradient where the transition from 
     * color a to color b is not linear, but in this case, exponential.
     *
     * @param size The desired size of the gradient.
     * @param ca Color a
     * @param cb Color b
     * @param type The type of gradient.
     * @param xfactor The x decay length. Use a value between -200 and 200.
     * @param yfactor The y decay length.
     * @param ncols The number of colors. See KNewPixmapEffect:gradient.
     */
    static QImage unbalancedGradient(const QSize &size, const QColor &ca, 
	    const QColor &cb, GradientType type, int xfactor = 100,
	    int yfactor = 100, int ncols = 3);

    /**
     * Same as above, only this works on a KPixmap.
     */
    static void unbalancedGradient(KPixmap &pixmap, const QColor &ca, 
	    const QColor &cb, GradientType type, int xfactor = 100, 
	    int yfactor = 100, int ncols=3);

    /**
     * Either brightens or dims the image by a specified percent.
     * For example, .5 will modify the colors by 50%. All percent values
     * should be positive, use bool brighten to set if the image gets
     * brightened or dimmed.
     */
    static void intensity(QImage &image, float percent, bool brighten=true);

    /**
     * Either brightens or dims a pixmap by a specified percent.
     */
    inline static void intensity(KPixmap &pixmap, float percent,
                                 bool brighten = true);
    /**
     * Modifies the intensity of a image's RGB channel component.
     */
    static void channelIntensity(QImage &image, float percent,
                                 RGBComponent channel,
                                 bool brighten = true);
    /**
     * Modifies the intensity of a pixmap's RGB channel component.
     */
    inline static void channelIntensity(KPixmap &pixmap, float percent,
                                        RGBComponent channel,
                                        bool brighten = true);
    
    /**
     * Blends the provided image into a background of the indicated color
     * @param initial_intensity this parameter takes values from -1 to 1: 
     *              a) if positive: how much to fade the image in its 
     *                              less affected spot
     *              b) if negative: roughly indicates how much of the image 
     *                              remains unaffected
     * @param bgnd indicates the color of the background to blend in
     * @param eff lets you choose what kind of blending you like
     * @param anti_dir blend in the opposite direction (makes no much sense
     *                  with concentric blending effects)
     */
    static void blend(QImage &image, float initial_intensity, 
		    const QColor &bgnd, GradientType eff,
		    bool anti_dir=false);

    /**
     * Blends the provided pixmap (see the other method prototype)
     */
    static void blend(KPixmap &pixmap, float initial_intensity, 
		    const QColor &bgnd, GradientType eff,
		    bool anti_dir=false, int ncols=3);

    /**
     * Builds a hash on any given QImage
     * @param lite - the hash faces the indicated lighting (cardinal poles)
     * @param spacing - how many unmodified pixels inbetween hashes
     */
    static void hash(QImage &image, Lighting lite=NorthLite, 
		   unsigned int spacing=0);

    /**
     * As above, on a KPixmap
     */
    static void hash(KPixmap &pixmap, Lighting lite=NorthLite, 
		       unsigned int spacing=0, int ncols=3);


    /**
     * Create an image with a pattern. The pattern is an 8x8 bitmap, 
     * specified by an array of 8 integers.
     *
     * @param size The desired size.
     * @param ca Color a
     * @param cb Color b
     * @param pattern The pattern.
     * @return A QImage painted with the pattern.
     */
    static QImage pattern(const QSize &size, const QColor &ca,
	    const QColor &cb, unsigned pattern[8]);

    /**
     * Same a above, only this works on a KPixmap.
     */
    static void pattern(KPixmap &pixmap, const QColor &ca,
	    const QColor &cb, unsigned pattern[8]);

    /**
     * Create a pattern from an image. KNewPixmapEffect::flatten()
     * is used to recolor the image between color a to color b.
     *
     * @param img A QImage containing the pattern.
     * @param ncols The number of colors to use. The image will be
     * dithered to this depth. Pass zero to prevent dithering.
     */
    static QImage pattern(const QSize &size, const QColor &ca,
	    const QColor &cb, QImage img, int ncols=0);

    /**
     * The same as above, only this works on a pixmap.
     * No dithering is done if the display is truecolor.
     */
    static void pattern(KPixmap &pixmap, const QColor &ca,
	    const QColor &cb, QImage img, int ncols=8);
    
    /**
     * This recolors an image. The most dark color will become color a, 
     * the most bright one color b, and in between.
     *
     * @param image A QImage to process.
     * @param ca Color a
     * @param cb Color b
     * @param ncols The number of colors to use. Pass zero to prevent
     * dithering.
     */
    static void flatten(QImage &image, const QColor &ca, 
	    const QColor &cb, int ncols=0);
};



inline void KNewPixmapEffect::intensity(KPixmap &pixmap, float percent,
                                     bool brighten)
{
    QImage image = pixmap.convertToImage();
    intensity(image, percent, brighten);
    pixmap.convertFromImage(image);
}

inline void KNewPixmapEffect::channelIntensity(KPixmap &pixmap, float percent,
                                            RGBComponent channel,
                                            bool brighten)
{
    QImage image = pixmap.convertToImage();
    channelIntensity(image, percent, channel, brighten);
    pixmap.convertFromImage(image);
}


/**
 * Helper function to fast calc some altered (lighten, shaded) colors (CT)
 *
 */
unsigned int lHash(unsigned int c);
unsigned int uHash(unsigned int c);


// -----------------------------------------------------------------------------
// KPixmapeffect.cpp follows, modified
/*
 * (C)Copyright 1999
 * Cristian Tibirna  <ctibirna@total.net>
 * Daniel M. Duley <mosfet@kde.org>
 * Dirk A. Mueller <dmuell@gmx.net>
 *
 */

#include <math.h>

#include <qimage.h>
#include <qpainter.h>
#include <qcolor.h>

#include <dither.h>
#include <kstddirs.h>
#include <kpixmapeffect.h>
    
void KNewPixmapEffect::gradient(KPixmap &pixmap, const QColor &ca,
	const QColor &cb, GradientType eff, int ncols)
{
    if(eff == VerticalGradient ||
        eff == HorizontalGradient ||
        eff == DiagonalGradient) {
        
        int rDiff, gDiff, bDiff;
        int rca, gca, bca, rcb, gcb, bcb;
        
        register int x, y;
        
        rDiff = (rcb = cb.red())   - (rca = ca.red());
        gDiff = (gcb = cb.green()) - (gca = ca.green());
        bDiff = (bcb = cb.blue())  - (bca = ca.blue());

        register int rl = rca << 16;
        register int gl = gca << 16;
        register int bl = bca << 16;

        int rcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * rDiff;
        int gcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * gDiff;
        int bcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * bDiff;
        
        QPainter p;

        p.begin(&pixmap);

        // these for-loops could be merged, but the if's in the inner loop
        // would make it slow
        switch(eff) {
        case VerticalGradient:
            for ( y = 0; y < pixmap.height(); y++ ) {
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;

                p.setPen(QColor(rl>>16, gl>>16, bl>>16));
                p.drawLine(0, y, pixmap.width()-1, y);
            }
            break;
        case HorizontalGradient:
            for( x = 0; x < pixmap.width(); x++) {
                rl += rcdelta;
                gl += gcdelta;
                bl += bcdelta;
                
                //debug("rl: %d, gl: %d, bl: %d", rl>>16, gl>>16, bl>>16);
                p.setPen(QColor(rl>>16, gl>>16, bl>>16));
                p.drawLine(x, 0, x, pixmap.height()-1);
            }
            break;
        case DiagonalGradient:
            if(pixmap.width() > pixmap.height()) {
                for( x = 0; x < 2*pixmap.width(); x++) {
                    rl += rcdelta;
                    gl += gcdelta;
                    bl += bcdelta;
                    
                    p.setPen(QPen(QColor((rl>>16), (gl>>16), (bl>>16))));
                    p.drawLine(QMIN(x, pixmap.width()), QMIN(0, x-pixmap.width()),
                               0, QMIN(x, pixmap.height()));
                }
            }
            else {
                //
            }

                    
        break;
        default:
        break; // never happens
        }
        
        p.end();
        
    }
    else {
        QImage image = gradient(pixmap.size(), ca, cb, eff, ncols);
        pixmap.convertFromImage(image);
    }
}

    
QImage KNewPixmapEffect::gradient(const QSize &size, const QColor &ca, 
	const QColor &cb, GradientType eff, int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;
    
    QImage image(size, 32);
    
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
        
        delete [] xtable[0];
        delete [] xtable[1];
        delete [] xtable[2];
        delete [] ytable[0];
        delete [] ytable[1];
        delete [] ytable[2];
    }

#if 0    
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
        kFSDither dither(dPal, ncols);
        image = dither.dither(image);
        delete [] dPal;
    }
#endif    

    return image;
}


//CT this was (before Dirk A. Mueller's speedup changes) 
//   merely the same code as in the above method, but it's supposedly
//   way less performant since it introduces a lot of supplementary tests
//   and simple math operations for the calculus of the balance.
//      (surprizingly, it isn't less performant, in the contrary :-)
//   Yes, I could have merged them, but then the excellent performance of
//   the balanced code would suffer with no other gain than a mere
//   source code and byte code size economy.

void KNewPixmapEffect::unbalancedGradient(KPixmap &pixmap, const QColor &ca,
	const QColor &cb, GradientType eff, int xfactor, int yfactor, 
	int ncols)
{
    QImage img = unbalancedGradient(pixmap.size(), ca, cb,  eff, xfactor, 
	    yfactor, ncols);
    pixmap.convertFromImage(img);
}

QImage KNewPixmapEffect::unbalancedGradient(const QSize &size, const QColor &ca, 
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


    double xbal = xfactor/5000.;
    double ybal = yfactor/5000.;
    double rat;

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image(size, 32);

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
            rat =  1 - exp( - (double)y  * ybal );
	    
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
	      rat = 1 - exp( - (double)x  * xbal );
	      
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

      if (ncols && (QPixmap::defaultDepth() < 15 )) {
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
void KNewPixmapEffect::intensity(QImage &image, float percent, bool brighten)
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

void KNewPixmapEffect::channelIntensity(QImage &image, float percent,
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


//======================================================================
//
// Blend effects
//
//======================================================================
void KNewPixmapEffect::blend(QImage &image, float initial_intensity, 
			  const QColor &bgnd, GradientType eff,
			  bool anti_dir)
{
    int r_bgnd = bgnd.red(), g_bgnd = bgnd.green(), b_bgnd = bgnd.blue();
    int r, g, b;
    int ind;

    unsigned int xi, xf, yi, yf;
    unsigned int a; // used for saving alpha channel info, 
                    //    as proposed by Brian Rolfe <brianr@corel.ca>

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

    if (anti_dir) { initial_intensity = intensity = 1.; var = -var;}

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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);
	}
      }
    }
    else if (eff == DiagonalGradient  || eff == CrossDiagonalGradient) {
      float xvar = var / 2 / image.width();  // / unaffected;
      float yvar = var / 2 / image.height(); // / unaffected;
      float tmp;

      /* set the image domain to apply the effect to
      if (anti_dir) {
	xi = (int)(image.width()*(1-unaffected));
	yi = (int)(image.height()*(1-unaffected));
      }
      else {
	xi = (int)(image.width()*unaffected);
	yi = (int)(image.height()*unaffected);
      }
      */

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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);
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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);

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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);
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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);

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
	  data[ind] = qRgb(r, g, b) | ((uint)(a & 0xff) << 24);
	}
      }
    }
      
    else debug("not implemented");
}

    
void KNewPixmapEffect::blend(KPixmap &pixmap, float initial_intensity,
			  const QColor &bgnd, GradientType eff, 
			  bool anti_dir, int ncols)
{
  QImage image = pixmap.convertToImage();
  blend(image, initial_intensity, bgnd, eff, anti_dir);

  unsigned int tmp;

  if(pixmap.depth() < 15 ) {
    if ( ncols < 2 || ncols > 256 )
      ncols = 3;
    QColor *dPal = new QColor[ncols];
    for (int i=0; i<ncols; i++) {
      tmp = 0 + 255 * i / ( ncols - 1 );
      dPal[i].setRgb ( tmp, tmp, tmp );
    }
    kFSDither dither(dPal, ncols);
    image = dither.dither(image);
    pixmap.convertFromImage(image);
    delete [] dPal;
  }
  else
    pixmap.convertFromImage(image);
}

unsigned int lHash(unsigned int c)
{
  unsigned char r = qRed(c), g = qGreen(c), b = qBlue(c), a = qAlpha(c);
  unsigned char nr, ng, nb;
  nr =(r >> 1) + (r >> 2); nr = nr > r ? 0 : nr;
  ng =(g >> 1) + (g >> 2); ng = ng > g ? 0 : ng;
  nb =(b >> 1) + (b >> 2); nb = nb > b ? 0 : nb;
  
  return qRgb(nr, ng, nb) | (uint) ((0xff & a) << 24);
}

inline unsigned int uHash(unsigned int c) 
{
  unsigned char r = qRed(c), g = qGreen(c), b = qBlue(c), a = qAlpha(c);
  unsigned char nr, ng, nb;
  nr = r + (r >> 3); nr = nr < r ? ~0 : nr;
  ng = g + (g >> 3); ng = ng < g ? ~0 : ng;
  nb = b + (b >> 3); nb = nb < b ? ~0 : nb;
  
  return qRgb(nr, ng, nb) | (uint) ((0xff & a) << 24);
}

void KNewPixmapEffect::hash(QImage &image, Lighting lite, unsigned int spacing)
{
  register int x, y;
  unsigned int *data =  (unsigned int *)image.bits();
  unsigned int ind;

  //CT no need to do it if not enough space
  if ((lite == NorthLite ||
       lite == SouthLite)&&
      image.height() < 2+spacing) return;
  if ((lite == EastLite ||
       lite == WestLite)&&
      image.height() < 2+spacing) return;

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
	   x < image.width() - ((y & 1)? 1 : 0) * spacing;
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
  
}

void KNewPixmapEffect::hash(KPixmap &pixmap, Lighting lite, 
			 unsigned int spacing, int ncols)
{
  QImage image = pixmap.convertToImage();
  hash(image, lite, spacing);

  unsigned int tmp;

  if(pixmap.depth() < 15 ) {
    if ( ncols < 2 || ncols > 256 )
      ncols = 3;
    QColor *dPal = new QColor[ncols];
    for (int i=0; i<ncols; i++) {
      tmp = 0 + 255 * i / ( ncols - 1 );
      dPal[i].setRgb ( tmp, tmp, tmp );
    }
    kFSDither dither(dPal, ncols);
    image = dither.dither(image);
    pixmap.convertFromImage(image);
    delete [] dPal;
  }
  else
    pixmap.convertFromImage(image);
}


QImage KNewPixmapEffect::pattern(const QSize &size, const QColor &ca, 
	const QColor &cb, unsigned pat[8])
{
    int x, y;
    QImage img(size, 8, 2);
    img.setColor(0, ca.rgb());
    img.setColor(1, cb.rgb());
    for (y=0; y < QMIN(size.height(),8); y++)
	for (x=0; x<size.width(); x++) {
	    if (pat[y] & (1 << (x&7)))
		img.setPixel(x, y, 0);
	    else
		img.setPixel(x, y, 1);
	}

    if (size.height() > 8)
	for (y=8; y<size.height(); y++)
	    memcpy(img.scanLine(y), img.scanLine(y&7), img.bytesPerLine());
    
    return img;
}


void KNewPixmapEffect::pattern(KPixmap &pixmap, const QColor &ca, 
	const QColor &cb, unsigned pat[8])
{
    QImage img = pattern(pixmap.size(), ca, cb, pat);
    pixmap.convertFromImage(img);
}


QImage KNewPixmapEffect::pattern(const QSize &size, const QColor &ca, 
	const QColor &cb, QImage img, int ncols)
{
    flatten(img, ca, cb, ncols);

    // tile img onto img2
    int x, y;
    QImage img2(size, img.depth());

    if (img.numColors()) {
	img2.setNumColors(img.numColors());
	for (x=0; x < img.numColors(); x++)
	    img2.setColor(x, img.color(x));
	for (y=0; y < QMIN(img2.height(),img.height()); y++)
	    for (x=0; x < img2.width(); x++)
		img2.setPixel(x, y, img.pixelIndex(x % img.width(), y));
    } else {
	for (y=0; y < QMIN(img2.height(),img.height()); y++)
	    for (x=0; x < img2.width(); x++)
		img2.setPixel(x, y, img.pixel(x % img.width(), y));
    }
    
    if (img2.height() > img.height())
	for (y=8; y < img2.height(); y++) {
	    memcpy(img2.scanLine(y), img2.scanLine(y % img.height()), 
		    img2.bytesPerLine());
	}

    return img2;
}

	
void KNewPixmapEffect::pattern(KPixmap &pixmap, const QColor &ca, 
	const QColor &cb, QImage img, int ncols)
{
    if (pixmap.depth() >= 15)
	ncols = 0;
    flatten(img, ca, cb, ncols);

    KPixmap pmtile;
    pmtile.convertFromImage(img);
    pixmap.tile(pmtile);
}


void KNewPixmapEffect::flatten(QImage &img, const QColor &ca, 
	const QColor &cb, int ncols)
{
    // a bitmap is easy...
    if (img.depth() == 1) {
	img.setColor(0, ca.rgb());
	img.setColor(1, cb.rgb());
	return;
    }

    int r1 = ca.red(); int r2 = cb.red();
    int g1 = ca.green(); int g2 = cb.green();
    int b1 = ca.blue(); int b2 = cb.blue();
    int min = 0, max = 255;

    int mean;
    QRgb col;

    // Get minimum and maximum greylevel.
    if (img.numColors()) {
	// pseudocolor
	for (int i = 0; i < img.numColors(); i++) {
	    col = img.color(i);
	    mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
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
    int r, g, b;
    double sr = ((double) r2 - r1) / (max - min);
    double sg = ((double) g2 - g1) / (max - min);
    double sb = ((double) b2 - b1) / (max - min);
    

    // Repaint the image
    if (img.numColors()) {
	for (int i=0; i < img.numColors(); i++) {
	    col = img.color(i);
	    mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
	    r = (int) (sr * (mean - min) + r1 + 0.5);
	    g = (int) (sg * (mean - min) + g1 + 0.5);
	    b = (int) (sb * (mean - min) + b1 + 0.5);
	    img.setColor(i, qRgb(r, g, b));
	}
    } else {
	for (int y=0; y < img.height(); y++)
	    for (int x=0; x < img.width(); x++) {
		col = img.pixel(x, y);
		mean = (qRed(col) + qGreen(col) + qBlue(col)) / 3;
		r = (int) (sr * (mean - min) + r1 + 0.5);
		g = (int) (sg * (mean - min) + g1 + 0.5);
		b = (int) (sb * (mean - min) + b1 + 0.5);
		img.setPixel(x, y, qRgb(r, g, b));
	    }
    }


    // Dither if necessary
    if ( (ncols <= 0) || 
	 ((img.numColors() != 0) && (img.numColors() <= ncols))
       )
	return;

    if (ncols == 1) ncols++;
    if (ncols > 256) ncols = 256;

    QColor *pal = new QColor[ncols];
    sr = ((double) r2 - r1) / (ncols - 1);
    sg = ((double) g2 - g1) / (ncols - 1);
    sb = ((double) b2 - b1) / (ncols - 1);

    for (int i=0; i<ncols; i++)
	pal[i] = QColor(r1 + sr*i, g1 + sg*i, b1 + sb*i);
	
    kFSDither dither(pal, ncols);
    img = dither.dither(img);

    delete[] pal;
}

    

// -----------------------------------------------------------------------------

void KGradientWidget::paintEvent(QPaintEvent */*ev*/)
{
    QTime time;
    int it, ft;
    QString say;

    QColor ca = Qt::black, cb = Qt::blue;

    int x = 0, y = 0;

    pix.resize(width()/cols, height()/rows);
    QPainter p(this);
    p.setPen(Qt::white);

    // draw once, so that the benchmarking be fair :-)
    KNewPixmapEffect::gradient(pix,ca, cb, KNewPixmapEffect::VerticalGradient);

    // vertical
    time.start();
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix,ca, cb, KNewPixmapEffect::VerticalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix,ca, cb, KNewPixmapEffect::HorizontalGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

#if 0    
    // elliptic
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix, ca, cb, KNewPixmapEffect::EllipticGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);
#endif    

    y++; // next row
    x = 0; // reset the columns

#if 0    
    // diagonal
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix,ca, cb, KNewPixmapEffect::DiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix,ca, cb, KNewPixmapEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);
#endif    

    y++; // next row
    x = 0; // reset the columns

#if 0
    // pyramidal
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix, ca, cb, KNewPixmapEffect::PyramidGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // rectangular
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix, ca, cb, KNewPixmapEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KNewPixmapEffect::gradient(pix, ca, cb, KNewPixmapEffect::PipeCrossGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
#endif    

}

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "KGradientTest");
    KGradientWidget w;
    w.resize(250 * cols, 250 * rows);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}
    

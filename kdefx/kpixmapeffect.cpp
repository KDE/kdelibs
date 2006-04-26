/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

*/

#include <qimage.h>
#include <qpainter.h>

#include "kpixmapeffect.h"
#include "kimageeffect.h"

//======================================================================
//
// Gradient effects
//
//======================================================================


QPixmap& KPixmapEffect::gradient(QPixmap &pixmap, const QColor &ca,
	const QColor &cb, GradientType eff, int ncols)
{
    if(pixmap.depth() > 8 &&
       (eff == VerticalGradient || eff == HorizontalGradient)) {

        int rDiff, gDiff, bDiff;
        int rca, gca, bca /*, rcb, gcb, bcb*/;

        register int x, y;

        rDiff = (/*rcb = */ cb.red())   - (rca = ca.red());
        gDiff = (/*gcb = */ cb.green()) - (gca = ca.green());
        bDiff = (/*bcb = */ cb.blue())  - (bca = ca.blue());

        register int rl = rca << 16;
        register int gl = gca << 16;
        register int bl = bca << 16;

        int rcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * rDiff;
        int gcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * gDiff;
        int bcdelta = ((1<<16) / (eff == VerticalGradient ? pixmap.height() : pixmap.width())) * bDiff;

        QPainter p(&pixmap);

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

                p.setPen(QColor(rl>>16, gl>>16, bl>>16));
                p.drawLine(x, 0, x, pixmap.height()-1);
            }
            break;
        default:
            ;
        }
    }
    else {
        QImage image = KImageEffect::gradient(pixmap.size(), ca, cb,
                                              (KImageEffect::GradientType) eff, ncols);
        pixmap = QPixmap::fromImage(image);
    }

    return pixmap;
}


// -----------------------------------------------------------------------------

QPixmap& KPixmapEffect::unbalancedGradient(QPixmap &pixmap, const QColor &ca,
         const QColor &cb, GradientType eff, int xfactor, int yfactor,
         int ncols)
{
    QImage image = KImageEffect::unbalancedGradient(pixmap.size(), ca, cb,
                                 (KImageEffect::GradientType) eff,
                                 xfactor, yfactor, ncols);
    pixmap = QPixmap::fromImage(image);

    return pixmap;
}


//======================================================================
//
// Intensity effects
//
//======================================================================



QPixmap& KPixmapEffect::intensity(QPixmap &pixmap, float percent)
{
    QImage image = pixmap.toImage();
    KImageEffect::intensity(image, percent);
    pixmap = QPixmap::fromImage(image);

    return pixmap;
}


// -----------------------------------------------------------------------------

QPixmap& KPixmapEffect::channelIntensity(QPixmap &pixmap, float percent,
                                     RGBComponent channel)
{
    QImage image = pixmap.toImage();
    KImageEffect::channelIntensity(image, percent,
                   (KImageEffect::RGBComponent) channel);
    pixmap = QPixmap::fromImage(image);

    return pixmap;
}


//======================================================================
//
// Blend effects
//
//======================================================================


QPixmap& KPixmapEffect::blend(QPixmap &pixmap, float initial_intensity,
			  const QColor &bgnd, GradientType eff,
			  bool anti_dir, int ncols)
{

    QImage image = pixmap.toImage();
    if (image.depth() <=8)
        image = image.convertToFormat(QImage::Format_RGB32); //Sloww..

    KImageEffect::blend(image, initial_intensity, bgnd,
                  (KImageEffect::GradientType) eff, anti_dir);

    unsigned int tmp;

    if(pixmap.depth() <= 8 ) {
        if ( ncols < 2 || ncols > 256 )
            ncols = 3;
        QColor *dPal = new QColor[ncols];
        for (int i=0; i<ncols; i++) {
            tmp = 0 + 255 * i / ( ncols - 1 );
            dPal[i].setRgb ( tmp, tmp, tmp );
        }
        KImageEffect::dither(image, dPal, ncols);
        pixmap = QPixmap::fromImage(image);
        delete [] dPal;
    }
    else
        pixmap = QPixmap::fromImage(image);

    return pixmap;
}


//======================================================================
//
// Hash effects
//
//======================================================================

QPixmap& KPixmapEffect::hash(QPixmap &pixmap, Lighting lite,
			 unsigned int spacing, int ncols)
{
    QImage image = pixmap.toImage();
    KImageEffect::hash(image, (KImageEffect::Lighting) lite, spacing);

    unsigned int tmp;

    if(pixmap.depth() <= 8 ) {
        if ( ncols < 2 || ncols > 256 )
            ncols = 3;
        QColor *dPal = new QColor[ncols];
        for (int i=0; i<ncols; i++) {
            tmp = 0 + 255 * i / ( ncols - 1 );
            dPal[i].setRgb ( tmp, tmp, tmp );
        }
        KImageEffect::dither(image, dPal, ncols);
        pixmap = QPixmap::fromImage(image);
        delete [] dPal;
    }
    else
        pixmap = QPixmap::fromImage(image);

    return pixmap;
}


//======================================================================
//
// Pattern effects
//
//======================================================================

#if 0
void KPixmapEffect::pattern(QPixmap &pixmap, const QColor &ca,
	const QColor &cb, unsigned pat[8])
{
    QImage img = pattern(pixmap.size(), ca, cb, pat);
    pixmap = QPixmap::fromImage(img);
}
#endif

// -----------------------------------------------------------------------------

QPixmap KPixmapEffect::pattern(const QPixmap& pmtile, QSize size,
                       const QColor &ca, const QColor &cb, int ncols)
{
    if (pmtile.depth() > 8)
	ncols = 0;

    QImage img = pmtile.toImage();
    KImageEffect::flatten(img, ca, cb, ncols);
    QPixmap pixmap;
    pixmap = QPixmap::fromImage(img);

    return KPixmapEffect::createTiled(pixmap, size);
}


// -----------------------------------------------------------------------------

QPixmap KPixmapEffect::createTiled(const QPixmap& pixmap, QSize size)
{
    QPixmap pix(size);

    QPainter p(&pix);
    p.drawTiledPixmap(0, 0, size.width(), size.height(), pixmap);

    return pix;
}


//======================================================================
//
// Fade effects
//
//======================================================================

QPixmap& KPixmapEffect::fade(QPixmap &pixmap, double val, const QColor &color)
{
    QImage img = pixmap.toImage();
    KImageEffect::fade(img, val, color);
    pixmap = QPixmap::fromImage(img);

    return pixmap;
}


// -----------------------------------------------------------------------------
QPixmap& KPixmapEffect::toGray(QPixmap &pixmap, bool fast)
{
    QImage img = pixmap.toImage();
    KImageEffect::toGray(img, fast);
    pixmap = QPixmap::fromImage(img);

    return pixmap;
}

// -----------------------------------------------------------------------------
QPixmap& KPixmapEffect::desaturate(QPixmap &pixmap, float desat)
{
    QImage img = pixmap.toImage();
    KImageEffect::desaturate(img, desat);
    pixmap = QPixmap::fromImage(img);

    return pixmap;
}
// -----------------------------------------------------------------------------
QPixmap& KPixmapEffect::contrast(QPixmap &pixmap, int c)
{
    QImage img = pixmap.toImage();
    KImageEffect::contrast(img, c);
    pixmap = QPixmap::fromImage(img);

    return pixmap;
}

//======================================================================
//
// Dither effects
//
//======================================================================

// -----------------------------------------------------------------------------
QPixmap& KPixmapEffect::dither(QPixmap &pixmap, const QColor *palette, int size)
{
    QImage img = pixmap.toImage();
    KImageEffect::dither(img, palette, size);
    pixmap = QPixmap::fromImage(img);

    return pixmap;
}

//======================================================================
//
// Other effects
//
//======================================================================

QPixmap KPixmapEffect::selectedPixmap( const QPixmap &pix, const QColor &col )
{
    QImage img = pix.toImage();
    KImageEffect::selectedImage(img, col);
    QPixmap outPix;
    outPix = QPixmap::fromImage(img);
    return outPix;
}

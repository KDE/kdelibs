/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
              (C) 1998, 1999 Daniel M. Duley <mosfet@kde.org>
              (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// $Id$

#include <qimage.h>
#include <qpainter.h>
#include <dither.h>

#include "kpixmapeffect.h"
#include "kimageeffect.h"

//======================================================================
//
// Gradient effects
//
//======================================================================


KPixmap& KPixmapEffect::gradient(KPixmap &pixmap, const QColor &ca,
	const QColor &cb, GradientType eff, int ncols)
{
    if(pixmap.depth() > 8 &&
       (eff == VerticalGradient || eff == HorizontalGradient)) {
        
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
        pixmap.convertFromImage(image);
    }
    
    return pixmap;
}


// -----------------------------------------------------------------------------

KPixmap& KPixmapEffect::unbalancedGradient(KPixmap &pixmap, const QColor &ca,
         const QColor &cb, GradientType eff, int xfactor, int yfactor,
         int ncols)
{
    QImage image = KImageEffect::unbalancedGradient(pixmap.size(), ca, cb,
                                 (KImageEffect::GradientType) eff,
                                 xfactor, yfactor, ncols);
    pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Intensity effects
//
//======================================================================



KPixmap& KPixmapEffect::intensity(KPixmap &pixmap, float percent)
{
    QImage image = pixmap.convertToImage();
    KImageEffect::intensity(image, percent);
    pixmap.convertFromImage(image);

    return pixmap;
}


// -----------------------------------------------------------------------------

KPixmap& KPixmapEffect::channelIntensity(KPixmap &pixmap, float percent,
                                     RGBComponent channel)
{
    QImage image = pixmap.convertToImage();
    KImageEffect::channelIntensity(image, percent,
                   (KImageEffect::RGBComponent) channel);
    pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Blend effects
//
//======================================================================


KPixmap& KPixmapEffect::blend(KPixmap &pixmap, float initial_intensity,
			  const QColor &bgnd, GradientType eff,
			  bool anti_dir, int ncols)
{
    QImage image = pixmap.convertToImage();
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
        kFSDither dither(dPal, ncols);
        image = dither.dither(image);
        pixmap.convertFromImage(image);
        delete [] dPal;
    }
    else
        pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Hash effects
//
//======================================================================

KPixmap& KPixmapEffect::hash(KPixmap &pixmap, Lighting lite,
			 unsigned int spacing, int ncols)
{
    QImage image = pixmap.convertToImage();
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
        kFSDither dither(dPal, ncols);
        image = dither.dither(image);
        pixmap.convertFromImage(image);
        delete [] dPal;
    }
    else
        pixmap.convertFromImage(image);

    return pixmap;
}


//======================================================================
//
// Pattern effects
//
//======================================================================

#if 0
void KPixmapEffect::pattern(KPixmap &pixmap, const QColor &ca,
	const QColor &cb, unsigned pat[8])
{
    QImage img = pattern(pixmap.size(), ca, cb, pat);
    pixmap.convertFromImage(img);
}
#endif

// -----------------------------------------------------------------------------
	
KPixmap KPixmapEffect::pattern(const KPixmap& pmtile, QSize size,
                       const QColor &ca, const QColor &cb, int ncols)
{
    if (pmtile.depth() > 8)
	ncols = 0;

    QImage img = pmtile.convertToImage();
    KImageEffect::flatten(img, ca, cb, ncols);
    KPixmap pixmap;
    pixmap.convertFromImage(img);

    return KPixmapEffect::createTiled(pixmap, size);
}


// -----------------------------------------------------------------------------

KPixmap KPixmapEffect::createTiled(const KPixmap& pixmap, QSize size)
{
    KPixmap pix;

    QPainter p(&pix);
    p.drawTiledPixmap(0, 0, size.width(), size.height(), pixmap);

    return pix;
}


//======================================================================
//
// Fade effects
//
//======================================================================

KPixmap& KPixmapEffect::fade(KPixmap &pixmap, double val, const QColor &color)
{
    QImage img = pixmap.convertToImage();
    KImageEffect::fade(img, val, color);
    pixmap.convertFromImage(img);

    return pixmap;
}


// -----------------------------------------------------------------------------
KPixmap& KPixmapEffect::toGray(KPixmap &pixmap, bool fast)
{
    QImage img = pixmap.convertToImage();
    KImageEffect::toGray(img, fast);
    pixmap.convertFromImage(img);

    return pixmap;
}

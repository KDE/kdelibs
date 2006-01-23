/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
 * 			Stephan Kulow <coolo@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qcolor.h>

#include <stdlib.h>
#include "kpixmap.h"

// Fast diffuse dither to 3x3x3 color cube
// Based on Qt's image conversion functions
static bool kdither_32_to_8( const QImage *src, QImage *dst )
{
    // register QRgb *p;
    uchar  *b;
    int	    y;
	
    if ( !dst->create(src->width(), src->height(), 8, 256) ) {
	qWarning("KPixmap: destination image not valid\n");
	return false;
    }

    int ncols = 256;

    static uint bm[16][16];
    static int init=0;
    if (!init) {

	// Build a Bayer Matrix for dithering
	init = 1;
	int n, i, j;

	bm[0][0]=0;

	for (n=1; n<16; n*=2)
	    for (i=0; i<n; i++)
		for (j=0; j<n; j++) {
		    bm[i][j]*=4;
		    bm[i+n][j]=bm[i][j]+2;
		    bm[i][j+n]=bm[i][j]+3;
		    bm[i+n][j+n]=bm[i][j]+1;
		}

	for (i=0; i<16; i++)
	    for (j=0; j<16; j++)
		bm[i][j]<<=8;
    }

    dst->setNumColors( ncols );

#define MAX_R 2
#define MAX_G 2
#define MAX_B 2
#define INDEXOF(r,g,b) (((r)*(MAX_G+1)+(g))*(MAX_B+1)+(b))

    int rc, gc, bc;

    for ( rc=0; rc<=MAX_R; rc++ )		// build 2x2x2 color cube
        for ( gc=0; gc<=MAX_G; gc++ )
	    for ( bc=0; bc<=MAX_B; bc++ ) {
		dst->setColor( INDEXOF(rc,gc,bc),
		qRgb( rc*255/MAX_R, gc*255/MAX_G, bc*255/MAX_B ) );
	    }	

    int sw = src->width();
    int* line1[3];
    int* line2[3];
    int* pv[3];

    line1[0] = new int[src->width()];
    line2[0] = new int[src->width()];
    line1[1] = new int[src->width()];
    line2[1] = new int[src->width()];
    line1[2] = new int[src->width()];
    line2[2] = new int[src->width()];
    pv[0] = new int[sw];
    pv[1] = new int[sw];
    pv[2] = new int[sw];

    for ( y=0; y < src->height(); y++ ) {
	// p = (QRgb *)src->scanLine(y);
	b = dst->scanLine(y);
	int endian = (QImage::systemByteOrder() == QImage::BigEndian);
	int x;
	const uchar* q = src->scanLine(y);
	const uchar* q2 = src->scanLine(y+1 < src->height() ? y + 1 : 0);

	for (int chan = 0; chan < 3; chan++) {
	    b = dst->scanLine(y);
	    int *l1 = (y&1) ? line2[chan] : line1[chan];
	    int *l2 = (y&1) ? line1[chan] : line2[chan];
	    if ( y == 0 ) {
		for (int i=0; i<sw; i++)
		    l1[i] = q[i*4+chan+endian];
	    }
	    if ( y+1 < src->height() ) {
		for (int i=0; i<sw; i++)
		    l2[i] = q2[i*4+chan+endian];
	    }

	    // Bi-directional error diffusion
	    if ( y&1 ) {
		for (x=0; x<sw; x++) {
		    int pix = qMax(qMin(2, (l1[x] * 2 + 128)/ 255), 0);
		    int err = l1[x] - pix * 255 / 2;
		    pv[chan][x] = pix;

		    // Spread the error around...
		    if ( x+1<sw ) {
			l1[x+1] += (err*7)>>4;
			l2[x+1] += err>>4;
		    }
		    l2[x]+=(err*5)>>4;
		    if (x>1)
			l2[x-1]+=(err*3)>>4;
		}
	    } else {
		for (x=sw; x-->0; ) {
		    int pix = qMax(qMin(2, (l1[x] * 2 + 128)/ 255), 0);
		    int err = l1[x] - pix * 255 / 2;
		    pv[chan][x] = pix;

		    // Spread the error around...
		    if ( x > 0 ) {
			l1[x-1] += (err*7)>>4;
			l2[x-1] += err>>4;
		    }
		    l2[x]+=(err*5)>>4;
		    if (x+1 < sw)
			l2[x+1]+=(err*3)>>4;
		}
	    }
	}

	if (!endian) {
	    for (x=0; x<sw; x++)
		*b++ = INDEXOF(pv[2][x],pv[1][x],pv[0][x]);
	} else {
	    for (x=0; x<sw; x++)
		*b++ = INDEXOF(pv[0][x],pv[1][x],pv[2][x]);
	}

    }

    delete [] line1[0];
    delete [] line2[0];
    delete [] line1[1];
    delete [] line2[1];
    delete [] line1[2];
    delete [] line2[2];
    delete [] pv[0];
    delete [] pv[1];
    delete [] pv[2];
	
#undef MAX_R
#undef MAX_G
#undef MAX_B
#undef INDEXOF

    return true;
}

KPixmap::KPixmap( uint width, uint height )
  : QPixmap( width, height )
{
}

KPixmap::~KPixmap()
{
}

bool KPixmap::load( const QString& fileName, const char *format,
		    int conversion_flags )
{
    QImage image;

    bool result = image.load( fileName, format );
	
    if ( result ) {
	detach();
	result = convertFromImage( image, conversion_flags );
    }
    return result;
}

bool KPixmap::load( const QString& fileName, const char *format,
		    ColorMode mode )
{
    int conversion_flags = 0;
    switch (mode) {
    case Color:
	conversion_flags |= Qt::ColorOnly;
	break;
    case Mono:
	conversion_flags |= Qt::MonoOnly;
	break;
    case LowColor:
	conversion_flags |= LowOnly;
	break;
    case WebColor:
	conversion_flags |= WebOnly;
	break;
    default:
	break;// Nothing.
    }
    return load( fileName, format, conversion_flags );
}

bool KPixmap::convertFromImage( const QImage &img, ColorMode mode )
{
    int conversion_flags = 0;
    switch (mode) {
    case Color:
	conversion_flags |= Qt::ColorOnly;
	break;
    case Mono:
	conversion_flags |= Qt::MonoOnly;
	break;
    case LowColor:
	conversion_flags |= LowOnly;
	break;
    case WebColor:
	conversion_flags |= WebOnly;
	break;
    default:
	break;	// Nothing.
    }
    return convertFromImage( img, conversion_flags );
}

static Qt::ImageConversionFlags intToQtFlags(int conversion_flags)
{
    //### An evil hack to convert types. This needs redesign, or to be killed
    conversion_flags = conversion_flags & ~ KColorMode_Mask;

    Qt::ImageConversionFlags flags;
    flags  = ~flags;            //Now flags has all bits set.
    flags &= conversion_flags;  //..which means this now puts conversion_flags into flags
    return flags;
}

bool KPixmap::convertFromImage( const QImage &img, int conversion_flags  )
{
    if ( img.isNull() ) {
#if defined(CHECK_NULL)
	qWarning( "KPixmap::convertFromImage: Cannot convert a null image" );
#endif
	return false;
    }
    detach();					// detach other references
	
    int dd = defaultDepth();

    // If color mode not one of KPixmaps extra modes nothing to do
    if ( ( conversion_flags & KColorMode_Mask ) != LowOnly &&
	 ( conversion_flags & KColorMode_Mask ) != WebOnly ) {
	    return QPixmap::convertFromImage ( img, intToQtFlags(conversion_flags) );
    }

    // If the default pixmap depth is not 8bpp, KPixmap color modes have no
    // effect. Ignore them and use AutoColor instead.
    if ( dd > 8 ) {
	if ( ( conversion_flags & KColorMode_Mask ) == LowOnly ||
	     ( conversion_flags & KColorMode_Mask ) == WebOnly )
	    conversion_flags = (conversion_flags & ~KColorMode_Mask) | Auto;
	return QPixmap::convertFromImage ( img, intToQtFlags(conversion_flags) );
    }
	
    if ( ( conversion_flags & KColorMode_Mask ) == LowOnly ) {
	// Here we skimp a little on the possible conversion modes
	// Don't offer ordered or threshold dither of RGB channels or
	// diffuse or ordered dither of alpha channel. It hardly seems
	// worth the effort for this specialized mode.
	
	// If image uses icon palette don't dither it.
	if( img.numColors() > 0 && img.numColors() <=40 ) {
	    if ( checkColorTable( img ) )
		return QPixmap::convertFromImage( img, QPixmap::Auto );
	}
	
	QBitmap mask;
	bool isMask = false;

	QImage  image = img.convertDepth(32);
	QImage tImage( image.width(), image.height(), 8, 256 );
	
	if( img.hasAlphaBuffer() ) {
	    image.setAlphaBuffer( true );
	    tImage.setAlphaBuffer( true );
	    isMask = mask.convertFromImage( img.createAlphaMask() );
	}
	
	kdither_32_to_8( &image, &tImage );
		
	if( QPixmap::convertFromImage( tImage ) ) {
	    if ( isMask ) QPixmap::setMask( mask );
		return true;
	} else
	    return false;
    } else {
	QImage  image = img.convertDepth( 32 );
	image.setAlphaBuffer( img.hasAlphaBuffer() );
	conversion_flags = (conversion_flags & ~Qt::ColorMode_Mask) | Auto;
	return QPixmap::convertFromImage ( image, intToQtFlags(conversion_flags) );
    }
}

static QColor* kpixmap_iconPalette = 0;

bool KPixmap::checkColorTable( const QImage &image )
{
    int i = 0;

    if (kpixmap_iconPalette == 0) {
	kpixmap_iconPalette = new QColor[40];
	
	// Standard palette
	kpixmap_iconPalette[i++] = Qt::red;
	kpixmap_iconPalette[i++] = Qt::green;
	kpixmap_iconPalette[i++] = Qt::blue;
	kpixmap_iconPalette[i++] = Qt::cyan;
	kpixmap_iconPalette[i++] = Qt::magenta;
	kpixmap_iconPalette[i++] = Qt::yellow;
	kpixmap_iconPalette[i++] = Qt::darkRed;
	kpixmap_iconPalette[i++] = Qt::darkGreen;
	kpixmap_iconPalette[i++] = Qt::darkBlue;
	kpixmap_iconPalette[i++] = Qt::darkCyan;
	kpixmap_iconPalette[i++] = Qt::darkMagenta;
	kpixmap_iconPalette[i++] = Qt::darkYellow;
	kpixmap_iconPalette[i++] = Qt::white;
	kpixmap_iconPalette[i++] = Qt::lightGray;
	kpixmap_iconPalette[i++] = Qt::gray;
	kpixmap_iconPalette[i++] = Qt::darkGray;
	kpixmap_iconPalette[i++] = Qt::black;
	
	// Pastels
	kpixmap_iconPalette[i++] = QColor( 255, 192, 192 );
	kpixmap_iconPalette[i++] = QColor( 192, 255, 192 );
	kpixmap_iconPalette[i++] = QColor( 192, 192, 255 );
	kpixmap_iconPalette[i++] = QColor( 255, 255, 192 );
	kpixmap_iconPalette[i++] = QColor( 255, 192, 255 );
	kpixmap_iconPalette[i++] = QColor( 192, 255, 255 );

	// Reds
	kpixmap_iconPalette[i++] = QColor( 64,   0,   0 );
	kpixmap_iconPalette[i++] = QColor( 192,  0,   0 );

	// Oranges
	kpixmap_iconPalette[i++] = QColor( 255, 128,   0 );
	kpixmap_iconPalette[i++] = QColor( 192,  88,   0 );
	kpixmap_iconPalette[i++] = QColor( 255, 168,  88 );
	kpixmap_iconPalette[i++] = QColor( 255, 220, 168 );

	// Blues
	kpixmap_iconPalette[i++] = QColor(   0,   0, 192 );

	// Turquoise
	kpixmap_iconPalette[i++] = QColor(   0,  64,  64 );
	kpixmap_iconPalette[i++] = QColor(   0, 192, 192 );

	// Yellows
	kpixmap_iconPalette[i++] = QColor(  64,  64,   0 );
	kpixmap_iconPalette[i++] = QColor( 192, 192,   0 );

	// Greens
	kpixmap_iconPalette[i++] = QColor(   0,  64,   0 );
	kpixmap_iconPalette[i++] = QColor(   0, 192,   0 );

	// Purples
	kpixmap_iconPalette[i++] = QColor( 192,   0, 192 );

	// Greys
	kpixmap_iconPalette[i++] = QColor(  88,  88,  88 );
	kpixmap_iconPalette[i++] = QColor(  48,  48,  48 );
	kpixmap_iconPalette[i++] = QColor( 220, 220, 220 );
	
    }

    QRgb* ctable = image.colorTable().data();

    int ncols = image.numColors();
    int j;

    // Allow one failure which could be transparent background
    int failures = 0;

    for ( i=0; i<ncols; i++ ) {
	for ( j=0; j<40; j++ ) {
	    if ( kpixmap_iconPalette[j].red() == qRed( ctable[i] ) &&
		 kpixmap_iconPalette[j].green() == qGreen( ctable[i] ) &&
		 kpixmap_iconPalette[j].blue() == qBlue( ctable[i] ) ) {
		break;
	    }
	}
	
	if ( j == 40 ) {
	    failures ++;			
	}
    }

    return ( failures <= 1 );

}

KPixmap::KPixmap(const QPixmap& p)
    : QPixmap(p)
{
}

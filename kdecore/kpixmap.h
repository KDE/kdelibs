/* This file is part of the KDE libraries
    Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
						Stephan Kulow				  

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

#ifndef __KPIXMAP_H__
#define __KPIXMAP_H__

#include <qpixmap.h>

const int KColorMode_Mask	= 0x00000300;
const int WebOnly 	= 0x00000200;
const int LowOnly	= 0x00000300;

/**
 * Off-screen paint device with extended features.

 * KPixmap has two new color modes, WebColor and LowColor, applicable
 * to 8bpp displays.

 * In WebColor mode all images are dithered to the Netscape palette,
 * even when they have their own color table. WebColor is the default
 * mode for KPixmap so that standard applications can share the Netscape
 * palette across the desktop.

 * In LowColor mode images are checked to see if their color table
 * matches the KDE icon palette. If the color tables do not match, the
 * images are dithered to a minimal 3x3x3 color cube. LowColor mode can
 * be used to load icons, background images etc. so that components of
 * the desktop which are always present use no more than 40 colors.

 * @author Mark Donohoe (donohoe@kde.org)
 * @version $Id$
 */
class KPixmap : public QPixmap
{
public:
	enum ColorMode { Auto, Color, Mono, LowColor, WebColor };
    
	/**
	 * Creates a null pixmap
	 */
	KPixmap() {};
	
	/**
	 * Destroys the pixmap.
	 */
	~KPixmap() {};
    
	/**
	 * Fills the pixmap with a color blend running from color ca to
	 * color cb.

	 * If upDown is TRUE the blend will run from the top to the
	 * bottom of the pixmap. If upDown is FALSE the blend will run
	 * from the right to the left of the pixmap.

	 * By default, the blend will use 3 colors on 8 bpp displays,
	 * 32 colors on 16 bpp displays and unlimited colors at higher
	 * dislay depths. For 8bpp displays ncols specifies an alternative
	 * number of colors to use. The greater the number of colors
	 * allocated the better the appearance of the gradient but the
	 * longer it takes to make.
	 */
	void gradientFill( QColor ca, QColor cb, bool upDown = TRUE, 
			int ncols = 3 );
	
	/**
	 * Fills the pixmap with a two color pattern, specified by the
	 * pattern bits in pattern[], the color ca to be used for the
	 * background and the color cb to be used for the foreground.
	 */
	void patternFill( QColor ca, QColor cb, uint pattern[8] );
	
	/** 
	 * Converts an image and sets this pixmap. Returns TRUE if
	 * successful. 
	 *
	 * The conversion_flags argument is a bitwise-OR from the
	 * following choices. The options marked (default) are the
	 * choice if no other choice from the list is included (they
	 * are zero):
	 *
	 * Color/Mono preference
	 *
	 * @li WebColor -  If the image has depth 1 and contains
	 * only black and white pixels then the pixmap becomes monochrome. If
	 * the pixmap has a depth of 8 bits per pixel then the Netscape
	 * palette is used for the pixmap color table.
	 * @li LowColor - If the image has depth 1 and contains only black and
	 * white pixels then the pixmap becomes monochrome. If the pixmap has a
	 * depth of 8 bits per pixel and the image does not posess a color table
	 * that matches the Icon palette a 3x3x3 color cube is used for the
	 * pixmap color table.
	 * @li AutoColor (default) - If the image has depth 1 and contains
	 * only black and white pixels, then the pixmap becomes
	 * monochrome.
	 * @li ColorOnly - The pixmap is dithered/converted to the native
	 * display depth.
	 * @li MonoOnly - The pixmap becomes monochrome. If necessary, it
	 * is dithered using the chosen dithering algorithm.
	 *
	 * Dithering mode preference, for RGB channels
	 *
	 * @li DiffuseDither (default) - a high quality dither
	 * @li OrderedDither - a faster more ordered dither
	 * @li ThresholdDither - no dithering, closest color is used
	 *
	 * Dithering mode preference, for alpha channel
	 *
	 * @li DiffuseAlphaDither - a high quality dither
	 * @li OrderedAlphaDither - a faster more ordered dither
	 * @li ThresholdAlphaDither (default) - no dithering
	 *
	 * Color matching versus dithering preference
	 * 
	 * @li PreferDither - always dither 32-bit images when the image
	 * is being converted to 8-bits. This is the default when
	 * converting to a pixmap.
	 * @li AvoidDither - only dither 32-bit images if the image has
	 * more than 256 colours and it is being converted to 8-bits.
	 * This is the default when an image is converted for the
	 * purpose of saving to a file.
	 *
	 * Passing 0 for conversion_flags gives all the default
	 * options.
	 */
	bool convertFromImage( const QImage &img, int conversion_flags );
	
	/*
	 * This is an overloaded member function, provided for
	 * convenience. It differs from the above function only in
	 * what argument(s) it accepts.
	 */
	bool convertFromImage( const QImage &img, ColorMode mode = WebColor );
	
	/**
	 * Loads a pixmap from the file fileName. Returns TRUE if
	 * successful, or FALSE if the pixmap could not be loaded. 
	 * 
	 * If format is specified, the loader attempts to read the
	 * pixmap using the specified format. If format is not
	 * specified (default), the loader reads a few bytes from the
	 * header to guess the file format.
	 *
	 * See the convertFromImage() documentation for a description
	 * of the conversion_flags argument.
	 *
	 * The QImageIO documentation lists the supported image
	 * formats and explains how to add extra formats.
	 */
	bool load( const QString& fileName, const QString& format, 
		int conversion_flags );
	
	/*
	 * This is an overloaded member function, provided for
	 * convenience. It differs from the above function only in
	 * what argument(s) it accepts.
	 */
	bool load( const QString& fileName, 
		const QString& format = QString::null,
		ColorMode mode = WebColor );
	/*
	 * Returns TRUE of the image is posessed of a color table that
	 * matches the Icon palette or FALSE otherwise.

	 * An image with one color not found in the Icon palette is
	 * considered to make a match, since this extra color may be a
	 * transparent background.
	 */
	bool checkColorTable(const QImage &image);	
};

#endif


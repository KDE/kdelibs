/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
 * 			Stephan Kulow <coolo@kde.org>
 *
 *  $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef __KPIXMAP_H__
#define __KPIXMAP_H__

#include <config.h>
#include <qpixmap.h>

const int KColorMode_Mask	= 0x00000300;
const int WebOnly 	= 0x00000200;
const int LowOnly	= 0x00000300;

class KPixmapPrivate;

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
        enum GradientMode { Horizontal, Vertical, Diagonal, CrossDiagonal };

	/**
	 * Constructs a null pixmap.
	 */
         KPixmap() : QPixmap() {};

	/**
	 * Destructs the pixmap.
	 */
	~KPixmap() {};

	/**
	 * Copies the QPixmap @p pix.
         */
	KPixmap(const QPixmap& pix);

	/**
	 * Converts an image and sets this pixmap.
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
	 * @li DiffuseDither (default) - A high quality dither.
	 * @li OrderedDither - A faster more ordered dither.
	 * @li ThresholdDither - No dithering, closest color is used.
	 *
	 * Dithering mode preference, for alpha channel
	 *
	 * @li DiffuseAlphaDither - A high quality dither.
	 * @li OrderedAlphaDither - A faster more ordered dither.
	 * @li ThresholdAlphaDither (default) - No dithering.
	 *
	 * Color matching versus dithering preference
	 *
	 * @li PreferDither - Always dither 32-bit images when the image
	 * is being converted to 8-bits. This is the default when
	 * converting to a pixmap.
	 * @li AvoidDither - Only dither 32-bit images if the image has
	 * more than 256 colours and it is being converted to 8-bits.
	 * This is the default when an image is converted for the
	 * purpose of saving to a file.
	 *
	 * Passing 0 for @p conversion_flags gives all the default
	 * options.
	 * @return @p true if successful.
	 **/
	bool convertFromImage( const QImage &img, int conversion_flags );

	/**
	 * This is an overloaded member function, provided for
	 * convenience. It differs from the above function only in
	 * what argument(s) it accepts.
	 **/
	bool convertFromImage( const QImage &img, ColorMode mode = WebColor );

	/**
	 * Loads a pixmap from the file @p fileName.
	 *
	 * If format is specified, the loader attempts to read the
	 * pixmap using the specified format. If format is not
	 * specified (default), the loader reads a few bytes from the
	 * header to guess the file format.
	 *
	 * See the @ref convertFromImage() documentation for a description
	 * of the conversion_flags argument.
	 *
	 * The @ref QImageIO documentation lists the supported image
	 * formats and explains how to add extra formats.
	 *
	 * @return @p true if successful, or false if the pixmap
	 *  could not be loaded.
	 **/
	bool load( const QString& fileName, const char *format,
		int conversion_flags );

	/**
	 * This is an overloaded member function, provided for
	 * convenience. It differs from the above function only in
	 * what argument(s) it accepts.
	 **/
	bool load( const QString& fileName,
		const char *format = 0,
		ColorMode mode = WebColor );

	/**
	 * Returns true if the image posesses a color table that
	 * matches the Icon palette or false otherwise.
	 *
	 * An image with one color not found in the Icon palette is
	 * considered to be a match, since this extra color may be a
	 * transparent background.
	 **/
	bool checkColorTable(const QImage &image);

private:
    KPixmapPrivate *d;
};

#endif

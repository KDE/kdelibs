/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
//
// Floyd-Steinberg dithering
// Ref: Bitmapped Graphics Programming in C++
//      Marv Luse, Addison-Wesley Publishing, 1993.
//

#ifndef __DITHER_H__
#define __DITHER_H__

#include <qimage.h>

/**
* Allows Floyd-Steinberg dithering for low-colour situations.
* @short Floyd-Steinberg dithering.
* @version $Id$
* @author Marv Luse (tm Addison Wesley Publishing)
*/
class kFSDither
{
public:
	/**
	*/
	kFSDither( const QColor *pal, int pSize );

	/**
	*/
	QImage dither( const QImage &i );

private:
	/**
	*/
	int nearestColor( int r, int g, int b );

private:
	/**
	*/
	const QColor *palette;
	/**
	*/
	int palSize;
};

#endif	// __DITHER_H__


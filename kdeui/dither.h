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


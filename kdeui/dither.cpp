//-----------------------------------------------------------------------------
//
// Floyd-Steinberg dithering
// Ref: Bitmapped Graphics Programming in C++
//      Marv Luse, Addison-Wesley Publishing, 1993.
//

#include <qcolor.h>
#include "dither.h"

kFSDither::kFSDither( const QColor *pal, int pSize )
{
	palette = pal;
	palSize = pSize;
}

QImage kFSDither::dither( const QImage &iImage )
{
	if ( iImage.depth() <= 8 )
	{
		warning( "Only dithering of 24bpp images supported" );
		return QImage();
	}

	QImage dImage( iImage.width(), iImage.height(), 8, palSize );
	int i;

	dImage.setNumColors( palSize );
	for ( i = 0; i < palSize; i++ )
		dImage.setColor( i, palette[ i ].rgb() );

	int *rerr1 = new int [ iImage.width() * 2 ];
	int *gerr1 = new int [ iImage.width() * 2 ];
	int *berr1 = new int [ iImage.width() * 2 ];

	memset( rerr1, 0, sizeof( int ) * iImage.width() * 2 );
	memset( gerr1, 0, sizeof( int ) * iImage.width() * 2 );
	memset( berr1, 0, sizeof( int ) * iImage.width() * 2 );

	int *rerr2 = rerr1 + iImage.width();
	int *gerr2 = gerr1 + iImage.width();
	int *berr2 = berr1 + iImage.width();

	for ( int j = 0; j < iImage.height(); j++ )
	{
		uint *ip = (uint * )iImage.scanLine( j );
		uchar *dp = dImage.scanLine( j );

		for ( i = 0; i < iImage.width(); i++ )
		{
			rerr1[i] = rerr2[i] + qRed( *ip );
			rerr2[i] = 0;
			gerr1[i] = gerr2[i] + qGreen( *ip );
			gerr2[i] = 0;
			berr1[i] = berr2[i] + qBlue( *ip );
			berr2[i] = 0;
			ip++;
		}

		*dp++ = nearestColor( rerr1[0], gerr1[0], berr1[0] );

		for ( i = 1; i < iImage.width()-1; i++ )
		{
			int indx = nearestColor( rerr1[i], gerr1[i], berr1[i] );
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

		*dp = nearestColor( rerr1[i], gerr1[i], berr1[i] );
	}

	delete [] rerr1;
	delete [] gerr1;
	delete [] berr1;

	return dImage;
}

int kFSDither::nearestColor( int r, int g, int b )
{
	int dr = palette[0].red() - r;
	int dg = palette[0].green() - g;
	int db = palette[0].blue() - b;

	int minDist =  dr*dr + dg*dg + db*db;
	int nearest = 0;

	for (int i = 1; i < palSize; i++ )
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


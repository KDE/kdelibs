#include <stdio.h>
#include <string.h>
#include <qiodevice.h>
#include <qcolor.h>
#include <qfile.h>
#include <qwmatrix.h>

#include "xview.h"

#define BUFSIZE 1024

void kimgio_xv_read( QImageIO *_imageio )
{      
	int x=-1;
	int y=-1;
	int maxval=-1;
	QIODevice *iodev = _imageio->ioDevice();

	char str[ BUFSIZE ];

	// magic number must be "P7 332"
	iodev->readLine( str, BUFSIZE );
	if (strncmp(str,"P7 332",6)) return;

	// next line #XVVERSION
	iodev->readLine( str, BUFSIZE );
	if (strncmp(str, "#XVVERSION", 10)) 
		return;

	// now it gets interesting, #BUILTIN means we are out.
	// if IMGINFO comes, we are happy!
	iodev->readLine( str, BUFSIZE );
	if (strncmp(str, "#IMGINFO:", 9))
		return;

	// after this an #END_OF_COMMENTS signals everything to be ok!
	iodev->readLine( str, BUFSIZE );
	if (strncmp(str, "#END_OF", 7))
		return;

	// now a last line with width, height, maxval which is 
	// supposed to be 255
	iodev->readLine( str, BUFSIZE );
	sscanf(str, "%d %d %d", &x, &y, &maxval);

	if (maxval != 255) return;

	// now follows a binary block of x*y bytes. 
	int blocksize = x*y;
	char *block = new char[ blocksize ];

	if (iodev->readBlock(block, blocksize) != blocksize ) 
	{
		return;
	}

	// Create the image
	QImage image( x, y, 8, maxval + 1, QImage::BigEndian );

	// how do the color handling? they are absolute 24bpp
	// or at least can be calculated as such.
	int r,g,b;

	for ( int j = 0; j < 256; j++ )
	{
		r =  ((int) ((j >> 5) & 0x07)) << 5;      
		g =  ((int) ((j >> 2) & 0x07)) << 5;   
		b =  ((int) ((j >> 0) & 0x03)) << 6;

		image.setColor( j, qRgb( r, g, b ) );
	}

	for ( int py = 0; py < y; py++ )
	{
		uchar *data = image.scanLine( py );	
		memcpy( data, block + py * x, x );
	}

	_imageio->setImage( image );
	_imageio->setStatus( 0 );

	delete [] block;
	return;
}

void kimgio_xv_write( QImageIO *imageio )
{
	QIODevice& f = *( imageio->ioDevice() );

	// Removed "f.open(...)" and "f.close()" (tanghus)

	const QImage& image = imageio->image();
	int w = image.width(), h = image.height();

	char str[ 1024 ];

	// magic number must be "P7 332"
	f.writeBlock( "P7 332\n", 7 );

	// next line #XVVERSION
	f.writeBlock( "#XVVERSION:\n", 12 );

	// now it gets interesting, #BUILTIN means we are out.
	// if IMGINFO comes, we are happy!
	f.writeBlock( "#IMGINFO:\n", 10 );

	// after this an #END_OF_COMMENTS signals everything to be ok!
	f.writeBlock( "#END_OF_COMMENTS:\n", 18 );

	// now a last line with width, height, maxval which is supposed to be 255
	sprintf( str, "%i %i 255\n", w, h );
	f.writeBlock( str, strlen( str ) );


	if ( image.depth() == 1 )
	{
		image.convertDepth( 8 );
	}

	uchar buffer[ 128 ];

	for ( int py = 0; py < h; py++ )
	{
		uchar *data = image.scanLine( py );
		for ( int px = 0; px < w; px++ )
		{
			int r, g, b;
			if ( image.depth() == 32 )
			{
				QRgb *data32 = (QRgb*) data;
				r = qRed( *data32 ) >> 5;
				g = qGreen( *data32 ) >> 5;		
				b = qBlue( *data32 ) >> 6;
				data += sizeof( QRgb );
			}
			else 
			{
				QRgb color = image.color( *data );
				r = qRed( color ) >> 5;
				g = qGreen( color ) >> 5;		
				b = qBlue( color ) >> 6;
				data++;
			}
			buffer[ px ] = ( r << 5 ) | ( g << 2 ) | b;
		}
		f.writeBlock( (const char*)buffer, w );
	}

	imageio->setStatus( 0 );
}

extern "C" void kimgio_init_xview() {
    // XV thumbnails
    QImageIO::defineIOHandler( "XV", "^P7 332", "T", 
			       kimgio_xv_read, kimgio_xv_write );
}


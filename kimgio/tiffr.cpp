// This library is distributed under the conditions of the GNU LGPL.

#include "config.h"

#ifdef HAVE_LIBTIFF

#include <tiffio.h>

#include <qimage.h>
#include <qfile.h>

#include <assert.h>

#include "tiffr.h"

static tsize_t tiff_read( thandle_t handle, tdata_t buf, tsize_t size )
{
    QIODevice *dev = reinterpret_cast<QIODevice *>( handle );
    return dev->readBlock( reinterpret_cast<char *>( buf ), size );
}

static tsize_t tiff_write( thandle_t, tdata_t, tsize_t )
{
    return 0;
}

static toff_t tiff_seek( thandle_t handle, toff_t off, int whence )
{
    QIODevice *dev = reinterpret_cast<QIODevice *>( handle );

    if ( whence == SEEK_CUR )
	off += dev->at();
    else if ( whence == SEEK_END )
	off += dev->size();

    if ( !dev->at( off ) )
	return ( toff_t )-1;

    return dev->at();
}

static toff_t tiff_size( thandle_t handle )
{
    QIODevice *dev = reinterpret_cast<QIODevice *>( handle );
    return dev->size();
}

static int tiff_close( thandle_t )
{
    return 0;
}

static int tiff_map( thandle_t, tdata_t *, toff_t * )
{
    return 0;
}

static void tiff_unmap( thandle_t, tdata_t, toff_t )
{
}

void kimgio_tiff_read( QImageIO *io )
{
	TIFF *tiff;
	uint32 width, height;
	uint32 *data;

	// FIXME: use qdatastream

	// open file
	tiff = TIFFClientOpen( QFile::encodeName( io->fileName() ), "r",
			       ( thandle_t )io->ioDevice(),
			       tiff_read, tiff_write, tiff_seek, tiff_close, 
			       tiff_size, tiff_map, tiff_unmap );

	if( tiff == 0 ) {
		return;
	}

	// create image with loaded dimensions
	TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH,	&width );
	TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );

	QImage image( width, height, 32 );
	data = (uint32 *)image.bits();

	//Sven: changed to %ld for 64bit machines
	//debug( "unsigned size: %ld, uint32 size: %ld",
	//	(long)sizeof(unsigned), (long)sizeof(uint32) );

	// read data
	bool stat =TIFFReadRGBAImage( tiff, width, height, data );

	if( stat == 0 ) {
		TIFFClose( tiff );
		return;
	}

	// reverse red and blue
	for( unsigned i = 0; i < width * height; ++i )
	{
		uint32 red = ( 0x00FF0000 & data[i] ) >> 16;
		uint32 blue = ( 0x000000FF & data[i] ) << 16;
		data[i] &= 0xFF00FF00;
		data[i] += red + blue;
	}

	// reverse image (it's upside down)
	for( unsigned ctr = 0; ctr < (height>>1); ) {
		unsigned *line1 = (unsigned *)image.scanLine( ctr );
		unsigned *line2 = (unsigned *)image.scanLine( height 
			- ( ++ctr ) );

		for( unsigned x = 0; x < width; x++ ) {
			int temp = *line1;
			*line1 = *line2;
			*line2 = temp;
			line1++;
			line2++;
		}

		// swap rows
	}

	// set channel order to Qt order
	// FIXME: Right now they are the same, but will it change?

//	for( int ctr = (image.numBytes() / sizeof(uint32))+1; ctr ; ctr-- ) {
//		// TODO: manage alpha with TIFFGetA
//		*data = qRgb( TIFFGetR( *data ), 
//			TIFFGetG( *data ), TIFFGetB( *data ) );
//		data++;
//	}
	TIFFClose( tiff );

	io->setImage( image );
	io->setStatus ( 0 );
}

void kimgio_tiff_write( QImageIO * )
{
	// TODO: stub
}

#endif

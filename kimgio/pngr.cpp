/*
* PNGR.CPP -- QImageIO read/write handlers for
*       the PNG graphic format using libpng.
*
*       Copyright (c) October 1998, Sirtaj Singh Kang.  Distributed under
*       the LGPL.
*
*       $Id$
*/

#ifdef HAVE_CONFIG_H
#include"config.h"
#endif

#ifdef HAVE_LIBPNG

#include<stdio.h>
#include<stdlib.h>
#include<qimage.h>
#include<qfile.h>

extern "C" {
#include<png.h>
}

void kimgio_png_read( QImageIO *io )
{
	// open png file

	QImage image;
	FILE *fp = fopen ( io->fileName(), "r" );
	int passes = 0;

	if( fp == 0 ) {
		debug( "Couldn't open %s for reading.", io->fileName().ascii() );
		return;
	}

		
	// init png structures

	png_structp png_ptr = png_create_read_struct(  // image ptr
		PNG_LIBPNG_VER_STRING, 0, 0, 0 );

	if( png_ptr == 0 ) {
		fclose( fp );
		return;
	}

	png_infop png_info = png_create_info_struct( png_ptr ); // info ptr

	if( png_info == 0 ) {
		png_destroy_read_struct( &png_ptr, 0, 0 );
		fclose( fp );
		return;
	}

	png_infop png_end = png_create_info_struct( png_ptr );

	if( !png_end ) {
		png_destroy_read_struct( &png_ptr, &png_info, 0 );
		fclose( fp );
		return;
	}

	// error jump point

	if( setjmp( png_ptr->jmpbuf ) ) {
		png_destroy_read_struct( &png_ptr, &png_info, &png_end );
		fclose( fp );
		return;
	}

	// read header
	png_init_io( png_ptr, fp );
	png_read_info( png_ptr, png_info );

	// transformations
	png_set_packing( png_ptr );
	png_set_strip_16( png_ptr );

	if( png_ptr->color_type & PNG_COLOR_TYPE_GRAY ) {
		png_set_gray_to_rgb( png_ptr );
	}
	else {
		png_set_expand( png_ptr );
	}

	if( ! (png_info->color_type & PNG_COLOR_MASK_ALPHA) ) {
		debug( "using filler" );
		png_set_filler( png_ptr, 0, PNG_FILLER_BEFORE );
	}

	passes = png_set_interlace_handling ( png_ptr );

	png_read_update_info( png_ptr, png_info );

	if ( png_info->color_type != PNG_COLOR_TYPE_RGB_ALPHA ) {
		debug( "Colortype %d is not rgb/alpha",
			png_info->color_type );
	}

	if( png_info->bit_depth != 8 ) {
		debug( "Depth %d is not 8", png_info->bit_depth );
	}

	// create image
	if ( !image.create( png_info->width, png_info->height, 32 ) ) {
		// out of memory
		warning( "Out of memory creating QImage." );
		png_destroy_read_struct( &png_ptr, &png_info, &png_end );
		fclose( fp );
		return;
	}

	// read image
	for( ; passes; passes-- ) {
		for( unsigned row = 0; row < png_info->height; row++ ) {
			png_read_row( png_ptr, image.scanLine( row ), NULL );
		}
	}

	if ( png_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA ) {
		debug( "Colortype %d is rgb/alpha",
			png_info->color_type );
		image.setAlphaBuffer(true);
	}
	else {
		unsigned *pixels = (unsigned *) image.bits();
		for( unsigned row = 0; row < png_info->height; row++ ) {
			for( int i = 0; i < image.width(); i++ ) {
				*pixels = *pixels >> 8;
				pixels++;
			}
		}
	}

	png_read_end( png_ptr, png_info );

	io->setImage( image );
	io->setStatus( 0 );
	
	// clean up 
	png_destroy_read_struct( &png_ptr, &png_info, &png_end );
	fclose( fp );

	return;
}

#if (defined PNG_LIBPNG_VER) && (PNG_LIBPNG_VER >= 100)

void kimgio_png_write( QImageIO *iio )
{
	QIODevice *f = ( iio->ioDevice() );
	FILE *fp = 0;
	png_structp png_ptr;
	png_infop info_ptr;
        int colortype = 0;

	const QImage& image = iio->image();
	int w = image.width(), h = image.height();

	int numcolors = image.numColors();
	int depth = image.depth() == 1 ? 1 : 8;

	//debug("Size:\t%d X %d\n\tColors:\t%d\n\tDepth:\t%d",
	//	w, h, numcolors, image.depth());

	if(numcolors > 0) {
		//debug("PALETTE");
		colortype = PNG_COLOR_TYPE_PALETTE;
	}
	else if(image.hasAlphaBuffer()) {
		//debug("RGB_ALPHA");
		colortype = PNG_COLOR_TYPE_RGB_ALPHA;
	}
	else {
		//debug("RGB");
		colortype = PNG_COLOR_TYPE_RGB;
	}

	// open the file
	fp = fdopen(((QFile*)f)->handle(), "wb");
	if (fp == 0) {
		iio->setStatus( -1 );
		return;
	}

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	if (png_ptr == 0) {
		fclose(fp);
		iio->setStatus( -2 );
		return;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == 0) {
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
		iio->setStatus( -3 );
		return;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	*/
	png_set_IHDR(png_ptr, info_ptr, w, h, depth, colortype,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* set the palette if there is one.  REQUIRED for indexed-color images */

	if(numcolors > 0) {
		info_ptr->palette = (png_colorp)png_malloc(png_ptr, numcolors * sizeof (png_color));
		for(int i = 0; i < numcolors; i++) {
			info_ptr->palette[i].red = qRed(image.color(i));
			info_ptr->palette[i].blue = qBlue(image.color(i));
			info_ptr->palette[i].green = qGreen(image.color(i));
		}
		png_set_PLTE(png_ptr, info_ptr, info_ptr->palette, numcolors);
	}

	//optional significant bit chunk

	if(image.isGrayscale()) {
		info_ptr->sig_bit.gray = 8;
	}
	else {
		info_ptr->sig_bit.red = 8;
		info_ptr->sig_bit.green = 8;
		info_ptr->sig_bit.blue = 8;
	}

	if(image.hasAlphaBuffer())
		info_ptr->sig_bit.alpha = 8;

  
	// Optional gamma chunk is strongly suggested if you have any guess
	// as to the correct gamma of the image.
	//png_set_gAMA(png_ptr, info_ptr, gamma);

	// Optionally write comments into the image
	//text_ptr[0].key = "Title";
	//text_ptr[0].text = "Mona Lisa";
	//text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	//text_ptr[1].key = "Author";
	//text_ptr[1].text = "Leonardo DaVinci";
	//text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	//text_ptr[2].key = "Description";
	//text_ptr[2].text = "<long text>";
	//text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
	//png_set_text(png_ptr, info_ptr, text_ptr, 2);

	// Write the file header information.  REQUIRED
	png_write_info(png_ptr, info_ptr);

	// Once we write out the header, the compression type on the text
	// chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
	// PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
	// at the end.

	// pack pixels into bytes
	png_set_packing( png_ptr );
	png_set_strip_16( png_ptr );

	// swap location of alpha bytes from ARGB to RGBA 
	//png_set_swap_alpha(png_ptr);

	// Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
	// RGB (4 channels -> 3 channels). The second parameter is not used.
	if ( depth == 8 && !image.hasAlphaBuffer() )
		png_set_filler(png_ptr, 0,
	    QImage::systemByteOrder() == QImage::BigEndian ?
		PNG_FILLER_BEFORE : PNG_FILLER_AFTER);

	// flip BGR pixels to RGB
	//png_set_bgr(png_ptr);

	// swap bytes of 16-bit files to most significant byte first
	//png_set_swap(png_ptr);

	// swap bits of 1, 2, 4 bit packed pixel formats
	//png_set_packswap(png_ptr);

	// The easiest way to write the image (you may have a different memory
	// layout, however, so choose what fits your needs best).  You need to
	// use the first method if you aren't handling interlacing yourself.

#define entire
	// One of the following output methods is REQUIRED 
#ifdef entire // write out the entire image data in one call

	png_byte **row_pointers = image.jumpTable();
	png_write_image(png_ptr, row_pointers);

#else // (no_entire) write out the image data by one or more scanlines
	// If you are only writing one row at a time, this works

	for (int y = 0; y < h; y++) {
		png_bytep row_pointer = image.scanLine(y);
		png_write_rows(png_ptr, row_pointer, 1);
	}
#endif // (no_entire) use only one output method

	// You can write optional chunks like tEXt, zTXt, and tIME at the end as well.

	// It is REQUIRED to call this to finish writing the rest of the file
	png_write_end(png_ptr, info_ptr);

	// if you malloced the palette, free it here
	if(numcolors > 0)
		free(info_ptr->palette);

	// if you allocated any text comments, free them here

	// clean up after the write, and free any memory allocated
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	// close the file
	fclose(fp);

	iio->setStatus( 0 );

	return;
}
#else
	// png library is too old

void kimgio_png_write( QImageIO *iio )
{
        // TODO: implement this
        warning("kimgio_png_write: not yet implemented for old PNG libraries");
}

#endif

extern "C" void kimgio_init_png() {
}

#endif /* HAVE_LIBPNG */

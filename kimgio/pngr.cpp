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
#include<qimage.h>

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
		debug( "Couldn't open %s for reading.", io->fileName() );
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

	unsigned *pixels = (unsigned *) image.bits();
	for( unsigned row = 0; row < png_info->height; row++ ) {
		for( int i = 0; i < image.width(); i++ ) {
			*pixels = *pixels >> 8;
			pixels++;
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

void kimgio_png_write( QImageIO * )
{
	fprintf( stderr, "Warning: PNG writing not yet implemented.\n" );
	return;
}

#endif /* HAVE_LIBPNG */

/* This file is part of the KDE project
   Copyright (C) 2003 Dominik Seichter <domseichter@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

/* this code supports:
 * reading:
 *     run length encoded true color tga files
 *     uncompressed true color tga files
 * writing:
 *     uncompressed true color tga files
 */

#include "tga.h"

#include <qimage.h>
#include <qdatastream.h>

/*
 * uncompressed TGA magic header
 */
unsigned char targaMagic[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* 
 * compressed TGA magic header
 */
unsigned char compMagic[12] = { 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/*
 * the origin of the image (default is TOP_LEFT)
 */
enum { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };

/*
 * Read one pixel and return its color 
 */
int getData( QDataStream* s, int bpp )
{
    unsigned char* data = new unsigned char[bpp];

    for( int d = 0; d < bpp; d++ )
        *s >> data[d];

    int color = 0;
    if( bpp == 4 )
        color = qRgba( data[0], data[1], data[2], data[3] );
    else 
        color = qRgb( data[0], data[1], data[2] );

    delete [] data;
    return color;
}

/*
 * checks wether y is inside of the image
 * when origin is of mode m
 */
bool checky( int y, int h, int m ) 
{
    if( m == TOP_LEFT ) 
	return (y < h);
    else if( m == BOTTOM_LEFT || m == BOTTOM_RIGHT )
	return ( y >= 0 );

    return false;
}

  /*
   * checks wether x is inside of the image
   * when origin is of mode m
   */
  bool checkx( int x, int w, int m ) 
  {
      if( m == TOP_LEFT || m == BOTTOM_LEFT ) 
	  return (x < w);
      else if( m == BOTTOM_RIGHT )
	  return ( x >= 0 );

      return false;
  }

  void kimgio_tga_read( QImageIO *io )
  {
      unsigned char header[6];
      bool compressed = false;

      QDataStream s( io->ioDevice() );
      s.setByteOrder( QDataStream::LittleEndian );

      /*
       * check whether it is a targa file or not
       */
      for( int i = 0; i < 12; i++ ) {
	  unsigned char a;
	  s >> a;
	  if( a != targaMagic[i] && a!= compMagic[i]) {
	      io->setImage( 0 );
	      io->setStatus( -1 );            
	      return;
	  }

	  // check if it is a compressed targa file
	  if( i == 2 && a == compMagic[i] )
	      compressed = true;
      }

      for( int i = 0; i < 6; i++ )
	  s >> header[i];
      if( s.atEnd()) {
	  io->setImage( 0 );
	  io->setStatus( -1 );            
	  return;
      }

      int width  = header[1] * 256 + header[0];
      int height = header[3] * 256 + header[2];
      int bpp = header[4];
      int bit = header[5];
      int bytesPerPixel = bpp / 8;

    /* Bit values:
     * bit 0-3: number of alpha bits per fixel 
     * bit 4-5: origin of image:
     *  - 0 0 bottom left
     *  - 1 0 bottom right
     *  - 0 1 top left // that's what we write
     *  - 1 1 top right
     */

    int mode;
    if( (bit | 0)  == 0 )
	mode = BOTTOM_LEFT;
    else if( (bit & 8) == 8 )
	/*
	 * should be BOTTOM_RIGHT,
	 * but GIMP writes them this way.
	 */
	mode = BOTTOM_LEFT;
    else if( (bit & 32) == 32 )
	mode = TOP_LEFT;
    else
	mode = TOP_LEFT;

    if( bytesPerPixel != 3 &&  bytesPerPixel != 4 ) {	
	io->setImage( 0 );
	io->setStatus( -1 );
	return;
    }

    QImage img;
    if( !img.create( width, height, (bpp == 24 ? 32 : bpp) )) {
        io->setImage( 0 );
        io->setStatus( -1 );
        return;
    }

    /*
     * Enable alpha buffer for transparent images
     */
    if( img.depth() == 32 ) 
	img.setAlphaBuffer( true );


    int x = 0;
    int y = 0;
    int addy = 1;
    int addx = 1;
    if( mode == BOTTOM_LEFT || mode == BOTTOM_RIGHT ) {
	y = height - 1;
	addy = -1;
    }

    if( mode == BOTTOM_RIGHT || mode == TOP_RIGHT ) {
	x = width - 1;
	addx = -1;
    }

    /*
     * we have to restore the value of x after each loop
     */
    int oldx = x;
    if( !compressed ) {
	for( ; !s.atEnd() && checky( y, height, mode ); y += addy )
	    for( x = oldx; checkx( x, width, mode ); x += addx  ) {
		img.setPixel( x, y, getData( &s, bytesPerPixel ) );
	    }
    } else {
        unsigned char cur;
        while( !s.atEnd() && checky( y, height, mode ) ) {
            while( checkx( x, width, mode ) ) {
                s >> cur;
                if( (cur & 128) == 128 ) {
                    // found a RLE chunk
                    int length = (cur & 127) + 1;
                    int color = getData( &s, bytesPerPixel );
                    for( int i = 0; i < length; i++ ) {
                        img.setPixel( x, y, color );
                        x += addx;
                    }
                } else {
                    int length = (cur & 127) + 1;
                    for( int i = 0; i < length; i++ ) {
                        img.setPixel( x, y, getData( &s, bytesPerPixel ) );
                        x += addx;
                   }
                }
            }
            y += addy;
            x = oldx;
        }
    }

    img = img.swapRGB();
    
    io->setImage( img );
    io->setStatus( 0 );
}


void kimgio_tga_write( QImageIO *io )
{
    QDataStream s( io->ioDevice() );
    s.setByteOrder( QDataStream::LittleEndian );

    const QImage img = io->image();
    const bool hasAlpha = img.hasAlphaBuffer();
    for( int i = 0; i < 12; i++ )
        s << targaMagic[i];

    // write header
    s << Q_UINT16( img.width() ); // width
    s << Q_UINT16( img.height() ); // height
    s << Q_UINT8( hasAlpha ? 32 : 24 ); // depth (24 bit RGB + 8 bit alpha)
    s << Q_UINT8( hasAlpha ? 0x24 : 0x20 ); // top left image (0x20) + 8 bit alpha (0x4)

    for( int y = 0; y < img.height(); y++ )
        for( int x = 0; x < img.width(); x++ ) {
            const QRgb color = img.pixel( x, y );
            s << Q_UINT8( qBlue( color ) );
            s << Q_UINT8( qGreen( color ) );
            s << Q_UINT8( qRed( color ) );
            if( hasAlpha )
                s << Q_UINT8( qAlpha( color ) );
        }

    io->setStatus( 0 );
}
 

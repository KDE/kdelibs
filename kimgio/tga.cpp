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

enum { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };

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

    int width  = header[1] * 256 + header[0];
    int height = header[3] * 256 + header[2];
    int bpp = header[4];
    int bit = header[5];
    int bytesPerPixel = bpp / 8;

    /* Bit values:
     * bit 0-3: number of alpha bits per fixel 
     * bit 4-5: origin of image:
     *  - 0 0 bottom left
     *  - 0 1 bottom right
     *  - 1 1 top left // that's what we write
     *  - 1 0 top right
     */

    int mode;
    if( bit == 0 )
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

    if( !compressed ) {
	if( mode == BOTTOM_LEFT ) {
	    for( int y = height - 1; y >= 0; y-- )
		for( int x = 0; x < width; x++ ) {
		    img.setPixel( x, y, getData( &s, bytesPerPixel ) );
		}
	} else {
	    for( int y = 0; y < height; y++ )
		for( int x = 0; x < width; x++ ) {
		    img.setPixel( x, y, getData( &s, bytesPerPixel ) );
		}
	}
    } else {
        int x = 0;
        int y = 0;
        unsigned char cur;
        while( y < height ) {
            while( x < width ) {
                s >> cur;
                if( (cur & 128) == 128 ) {
                    // found a RLE chunk
                    int length = (cur & 127) + 1;
                    int color = getData( &s, bytesPerPixel );
                    for( int i = 0; i < length; i++ ) {
                        img.setPixel( x, y, color );
                        x++;
                    }
                } else {
                    int length = (cur & 127) + 1;
                    for( int i = 0; i < length; i++ ) {
                        img.setPixel( x, y, getData( &s, bytesPerPixel ) );
                        x++;
                   }
                }
            }
            y++;
            x = 0;
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

    QImage img = io->image().swapRGB();
    for( int i = 0; i < 12; i++ )
        s << targaMagic[i];

    // write header
    s << (unsigned char)0;
    s << (unsigned char)(img.width() / 256); // width
    s << (unsigned char)0;
    s << (unsigned char)(img.height() / 256); // height
    s << (unsigned char)img.depth(); // depth
    s << (unsigned char)36;

    for( int y = 0; y < img.height(); y++ )
        for( int x = 0; x < img.width(); x++ ) {
            int color = img.pixel( x, y );
            s << (unsigned char)qRed( color );
            s << (unsigned char)qGreen( color );
            s << (unsigned char)qBlue( color );
            if( img.depth() == 32 )
                s << (unsigned char)qAlpha( color );
        }

    io->setStatus( 0 );
}
 

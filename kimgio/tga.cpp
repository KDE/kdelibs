/* This file is part of the KDE project
   Copyright (C) 2003 Dominik Seichter <domseichter@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "tga.h"

#include <qimage.h>
#include <qdatastream.h>
#include <kdebug.h>

int getData( QDataStream* s, int bpp )
{
    unsigned char* data = new unsigned char[bpp];


    for( int d = 0; d < bpp; d++ )
        *s >> data[d];

    int color = 0;
    if( bpp == 4 )
        color = qRgba( data[0], data[1], data[2], data[3] );
    else if( bpp == 3 )
        color = qRgb( data[0], data[1], data[2] );
    else
        color = qRgb( data[0], data[1], data[2] );

    delete [] data;
    return color;
}

// uncompressed TGA magic header
unsigned char targaMagic[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void kimgio_tga_read( QImageIO *io )
{
    QDataStream s( io->ioDevice() );
    s.setByteOrder( QDataStream::LittleEndian );
    unsigned char header[6];
    // compressed
    unsigned char compMagic[12] = { 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    bool compressed = false;
    // check if it is a targa file
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
    int bytesPerPixel = bpp / 8;

    QImage img;
    if( !img.create( width, height, (bpp == 24 ? 32 : bpp) )) {
        io->setImage( 0 );
        io->setStatus( -1 );
        return;
    }

    if( !compressed ) {
        for( int y = 0; y < height; y++ )
            for( int x = 0; x < width; x++ ) {
                img.setPixel( x, y, getData( &s, bytesPerPixel ) );
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
    s << (unsigned char)32;

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
 

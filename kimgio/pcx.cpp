/* This file is part of the KDE project
   Copyright (C) 2002 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "pcx.h"

#include <qimage.h>

#include <kdebug.h>

QDataStream &operator>>( QDataStream &s, PALETTE &pal )
{
  for ( int i=0; i<16; ++i )
    s >> pal.p[ i ].r >> pal.p[ i ].g >> pal.p[ i ].b;

  return s;
}

QDataStream &operator>>( QDataStream &s, PCXHEADER &ph )
{
  s >> ph.Manufacturer;
  s >> ph.Version;
  s >> ph.Encoding;
  s >> ph.Bpp;
  s >> ph.XMin >> ph.YMin >> ph.XMax >> ph.YMax;
  s >> ph.HDpi >> ph.YDpi;
  s >> ph.Palette;
  s >> ph.Reserved;
  s >> ph.NPlanes;
  s >> ph.BytesPerLine;
  s >> ph.PaletteInfo;
  s >> ph.HScreenSize;
  s >> ph.VScreenSize;

  return s;
}

QDataStream &operator<<( QDataStream &s, PALETTE &pal )
{
  for ( int i=0; i<16; ++i )
    s << pal.p[ i ].r << pal.p[ i ].g << pal.p[ i ].b;

  return s;
}

static PCXHEADER header;
static QImage img;
Q_UINT16 w, h;

static void readLine( QDataStream &s, QByteArray &buf )
{
  Q_UINT32 i=0;
  Q_UINT32 size = buf.size();
  Q_UINT8 byte, count;

  if ( header.Encoding == 1 )
  {
    // Uncompress the image data
    while ( i < size )
    {
      count = 1;
      s >> byte;
      if ( byte > 0xc0 )
      {
        count = byte - 0xc0;
        s >> byte;
      }
      while ( count-- && i < size )
        buf[ i++ ] = byte;
    }
  }
  else
  {
    // Image is not compressed (possible?)
    while ( i < size )
    {
      s >> byte;
      buf[ i++ ] = byte;
    }
  }
}

static void readImage1( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 1, 2, QImage::BigEndian );

  for ( int y=0; y<h; ++y )
  {
    readLine( s, buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *( img.scanLine( y )+x ) = ~buf[ x ];
  }

  // Set the color palette
  img.setColor( 0, qRgb( 0, 0, 0 ) );
  img.setColor( 1, qRgb( 255, 255, 255 ) );
}

static void readImage4( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine*4 );
  QByteArray pixbuf( w );

  img.create( w, h, 8, 16, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    pixbuf.fill( 0 );
    readLine( s, buf );

    for ( int p=0; p<4; p++ )
    {
      Q_UINT32 offset = p*header.BytesPerLine;
      for ( int x=0; x<w; ++x )
        if ( buf[ offset + ( x/8 ) ] & ( 128 >> ( x%8 ) ) )
          pixbuf[ x ] += ( 1 << p );
    }

    for ( int x=0; x<w; ++x )
      *( img.scanLine( y )+x ) = pixbuf[ x ];
  }

  // Read the palette
  for ( int i=0; i<16; ++i )
  {
    struct PALETTE pal = header.Palette;
    img.setColor( i, qRgb( pal.p[ i ].r, pal.p[ i ].g, pal.p[ i ].b ) );
  }
}

static void readImage8( QDataStream &s )
{
  QByteArray buf( header.BytesPerLine );

  img.create( w, h, 8, 256, QImage::IgnoreEndian );

  for ( int y=0; y<h; ++y )
  {
    readLine( s, buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
      *( img.scanLine( y )+x ) = buf[ x ];
  }

  Q_UINT8 flag;
  s >> flag;
  kdDebug() << "Flag: " << flag << endl;

  if ( flag == 12 && header.Version == 5 )
  {
    // Read the palette
    Q_UINT8 r, g, b;
    for ( int i=0; i<256; ++i )
    {
      s >> r >> g >> b;
      img.setColor( i, qRgb( r, g, b ) );
    }
  }
}

static void readImage24( QDataStream &s )
{
  QByteArray r_buf( header.BytesPerLine );
  QByteArray g_buf( header.BytesPerLine );
  QByteArray b_buf( header.BytesPerLine );

  img.create( w, h, 32 );

  for ( int y=0; y<h; ++y )
  {
    readLine( s, r_buf );
    readLine( s, g_buf );
    readLine( s, b_buf );

    for ( int x=0; x<header.BytesPerLine; ++x )
    {
      uint *p = ( uint * )img.scanLine( y )+x;
      *p = qRgb( r_buf[ x ], g_buf[ x ], b_buf[ x ] );
    }
  }
}

void kimgio_pcx_read( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  s >> header;

  if ( header.Manufacturer != 0x0a )
  {
    io->setStatus( -1 );
    return;
  }

  w = ( header.XMax-header.XMin ) + 1;
  h = ( header.YMax-header.YMin ) + 1;

  kdDebug() << "Manufacturer: " << header.Manufacturer << endl;
  kdDebug() << "Version: " << header.Version << endl;
  kdDebug() << "Encoding: " << header.Encoding << endl;
  kdDebug() << "Bpp: " << header.Bpp << endl;
  kdDebug() << "Width: " << w << endl;
  kdDebug() << "Height: " << h << endl;
  kdDebug() << "Window: " << header.XMin << "," << header.XMax << "," 
            << header.YMin << "," << header.YMax << endl;
  kdDebug() << "BytesPerLine: " << header.BytesPerLine << endl;
  kdDebug() << "NPlanes: " << header.NPlanes << endl;

  // Skip the rest of the header
  Q_UINT8 byte;
  while ( s.device()->at() < 128 )
    s >> byte;

  if ( header.Bpp == 1 && header.NPlanes == 1 )
  {
    readImage1( s );
  }
  else if ( header.Bpp == 1 && header.NPlanes == 4 )
  {
    readImage4( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 1 )
  {
    readImage8( s );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 3 )
  {
    readImage24( s );
  }

  kdDebug() << "Image Bytes: " << img.numBytes() << endl;
  kdDebug() << "Image Bytes Per Line: " << img.bytesPerLine() << endl;
  kdDebug() << "Image Depth: " << img.depth() << endl;

  io->setImage( img );
  io->setStatus( 0 );
}

void kimgio_pcx_write( QImageIO *io )
{
  QDataStream s( io->ioDevice() );
  s.setByteOrder( QDataStream::LittleEndian );

  QImage img = io->image();

  struct PCXHEADER header;

  header.Manufacturer = 0x0a;
  header.Version = 5;
  header.Encoding = 1;

  header.XMin = 0;
  header.YMin = 0;
  header.XMax = img.width()-1;
  header.YMax = img.height()-1;

  io->setStatus( -1 );
}

/* vim: et sw=2 ts=2
*/


/* This file is part of the KDE project
   Copyright (C) 2002-2005 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "pcx.h"

#include <qimage.h>

#include <kdebug.h>

static QDataStream &operator>>( QDataStream &s, RGB &rgb )
{
  s >> rgb.r >> rgb.g >> rgb.b;

  return s;
}

static QDataStream &operator>>( QDataStream &s, Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s >> pal.rgb[ i ];

  return s;
}

static QDataStream &operator>>( QDataStream &s, PCXHEADER &ph )
{
  s >> ph.Manufacturer;
  s >> ph.Version;
  s >> ph.Encoding;
  s >> ph.Bpp;
  s >> ph.XMin >> ph.YMin >> ph.XMax >> ph.YMax;
  s >> ph.HDpi >> ph.YDpi;
  s >> ph.ColorMap;
  s >> ph.Reserved;
  s >> ph.NPlanes;
  s >> ph.BytesPerLine;
  s >> ph.PaletteInfo;
  s >> ph.HScreenSize;
  s >> ph.VScreenSize;

  // Skip the rest of the header
  Q_UINT8 byte;
  while ( s.device()->at() < 128 )
    s >> byte;

  return s;
}

static QDataStream &operator<<( QDataStream &s, const RGB &rgb )
{
  s << rgb.r << rgb.g << rgb.b;

  return s;
}

static QDataStream &operator<<( QDataStream &s, const Palette &pal )
{
  for ( int i=0; i<16; ++i )
    s << pal.rgb[ i ];

  return s;
}

static QDataStream &operator<<( QDataStream &s, const PCXHEADER &ph )
{
  s << ph.Manufacturer;
  s << ph.Version;
  s << ph.Encoding;
  s << ph.Bpp;
  s << ph.XMin << ph.YMin << ph.XMax << ph.YMax;
  s << ph.HDpi << ph.YDpi;
  s << ph.ColorMap;
  s << ph.Reserved;
  s << ph.NPlanes;
  s << ph.BytesPerLine;
  s << ph.PaletteInfo;
  s << ph.HScreenSize;
  s << ph.VScreenSize;

  Q_UINT8 byte = 0;
  for ( int i=0; i<54; ++i )
    s << byte;

  return s;
}

PCXHEADER::PCXHEADER()
{
  // Initialize all data to zero
  QByteArray dummy( 128 );
  dummy.fill( 0 );
  QDataStream s( &dummy, QIODevice::ReadOnly );
  s >> *this;
}

static void readLine( QDataStream &s, QByteArray &buf, const PCXHEADER &header )
{
  Q_UINT32 i=0;
  Q_UINT32 size = buf.size();
  Q_UINT8 byte, count;

  if ( header.isCompressed() )
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

static void readImage1( QImage &img, QDataStream &s, const PCXHEADER &header )
{
  QByteArray buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 1, 2, QImage::BigEndian ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf, header );
    uchar *p = img.scanLine( y );
    unsigned int bpl = qMin((Q_UINT16)((header.width()+7)/8), header.BytesPerLine);
    for ( unsigned int x=0; x< bpl; ++x )
      p[ x ] = buf[x];
  }

  // Set the color palette
  img.setColor( 0, qRgb( 0, 0, 0 ) );
  img.setColor( 1, qRgb( 255, 255, 255 ) );
}

static void readImage4( QImage &img, QDataStream &s, const PCXHEADER &header )
{
  QByteArray buf( header.BytesPerLine*4 );
  QByteArray pixbuf( header.width() );

  if(!img.create( header.width(), header.height(), 8, 16 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    pixbuf.fill( 0 );
    readLine( s, buf, header );

    for ( int i=0; i<4; i++ )
    {
      Q_UINT32 offset = i*header.BytesPerLine;
      for ( int x=0; x<header.width(); ++x )
        if ( buf[ offset + ( x/8 ) ] & ( 128 >> ( x%8 ) ) )
          pixbuf[ x ] = (int)(pixbuf[ x ]) + ( 1 << i );
    }

    uchar *p = img.scanLine( y );
    for ( int x=0; x<header.width(); ++x )
      p[ x ] = pixbuf[ x ];
  }

  // Read the palette
  for ( int i=0; i<16; ++i )
    img.setColor( i, header.ColorMap.color( i ) );
}

static void readImage8( QImage &img, QDataStream &s, const PCXHEADER &header )
{
  QByteArray buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 8, 256 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, buf, header );

    uchar *p = img.scanLine( y );
    unsigned int bpl = qMin(header.BytesPerLine, (Q_UINT16)header.width());
    for ( unsigned int x=0; x<bpl; ++x )
      p[ x ] = buf[ x ];
  }

  Q_UINT8 flag;
  s >> flag;
  kdDebug( 399 ) << "Palette Flag: " << flag << endl;

  if ( flag == 12 && ( header.Version == 5 || header.Version == 2 ) )
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

static void readImage24( QImage &img, QDataStream &s, const PCXHEADER &header )
{
  QByteArray r_buf( header.BytesPerLine );
  QByteArray g_buf( header.BytesPerLine );
  QByteArray b_buf( header.BytesPerLine );

  if(!img.create( header.width(), header.height(), 32 ))
    return;

  for ( int y=0; y<header.height(); ++y )
  {
    if ( s.atEnd() )
    {
      img.reset();
      return;
    }

    readLine( s, r_buf, header );
    readLine( s, g_buf, header );
    readLine( s, b_buf, header );

    uint *p = ( uint * )img.scanLine( y );
    for ( int x=0; x<header.width(); ++x )
      p[ x ] = qRgb( r_buf[ x ], g_buf[ x ], b_buf[ x ] );
  }
}

static void writeLine( QDataStream &s, QByteArray &buf )
{
  Q_UINT32 i = 0;
  Q_UINT32 size = buf.size();
  Q_UINT8 count, data;
  char byte;

  while ( i < size )
  {
    count = 1;
    byte = buf[ i++ ];

    while ( ( i < size ) && ( byte == buf[ i ] ) && ( count < 63 ) )
    {
      ++i;
      ++count;
    }

    data = byte;

    if ( count > 1 || data >= 0xc0 )
    {
      count |= 0xc0;
      s << count;
    }

    s << data;
  }
}

static void writeImage1( QImage &img, QDataStream &s, PCXHEADER &header )
{
  img = img.convertBitOrder( QImage::BigEndian );

  header.Bpp = 1;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  s << header;

  QByteArray buf( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    Q_UINT8 *p = img.scanLine( y );

    // Invert as QImage uses reverse palette for monochrome images?
    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = ~p[ i ];

    writeLine( s, buf );
  }
}

static void writeImage4( QImage &img, QDataStream &s, PCXHEADER &header )
{
  header.Bpp = 1;
  header.NPlanes = 4;
  header.BytesPerLine = header.width()/8;

  for ( int i=0; i<16; ++i )
    header.ColorMap.setColor( i, img.color( i ) );

  s << header;

  QByteArray buf[ 4 ];

  for ( int i=0; i<4; ++i )
      buf[ i ].resize( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    Q_UINT8 *p = img.scanLine( y );

    for ( int i=0; i<4; ++i )
      buf[ i ].fill( 0 );

    for ( int x=0; x<header.width(); ++x )
    {
      for ( int i=0; i<4; ++i )
        if ( *( p+x ) & ( 1 << i ) )
          buf[ i ][ x/8 ] = (int)(buf[ i ][ x/8 ])| 1 << ( 7-x%8 );
    }

    for ( int i=0; i<4; ++i )
      writeLine( s, buf[ i ] );
  }
}

static void writeImage8( QImage &img, QDataStream &s, PCXHEADER &header )
{
  header.Bpp = 8;
  header.NPlanes = 1;
  header.BytesPerLine = img.bytesPerLine();

  s << header;

  QByteArray buf( header.BytesPerLine );

  for ( int y=0; y<header.height(); ++y )
  {
    Q_UINT8 *p = img.scanLine( y );

    for ( int i=0; i<header.BytesPerLine; ++i )
      buf[ i ] = p[ i ];

    writeLine( s, buf );
  }

  // Write palette flag
  Q_UINT8 byte = 12;
  s << byte;

  // Write palette
  for ( int i=0; i<256; ++i )
    s << RGB( img.color( i ) );
}

static void writeImage24( QImage &img, QDataStream &s, PCXHEADER &header )
{
  header.Bpp = 8;
  header.NPlanes = 3;
  header.BytesPerLine = header.width();

  s << header;

  QByteArray r_buf( header.width() );
  QByteArray g_buf( header.width() );
  QByteArray b_buf( header.width() );

  for ( int y=0; y<header.height(); ++y )
  {
    uint *p = ( uint * )img.scanLine( y );

    for ( int x=0; x<header.width(); ++x )
    {
      QRgb rgb = *p++;
      r_buf[ x ] = qRed( rgb );
      g_buf[ x ] = qGreen( rgb );
      b_buf[ x ] = qBlue( rgb );
    }

    writeLine( s, r_buf );
    writeLine( s, g_buf );
    writeLine( s, b_buf );
  }
}


PCXHandler::PCXHandler()
{
}

bool PCXHandler::canRead() const
{
    return canRead(device());
}

bool PCXHandler::read(QImage *outImage)
{
  QDataStream s( device() );
  s.setByteOrder( QDataStream::LittleEndian );

  if ( s.device()->size() < 128 )
  {
    return false;
  }

  PCXHEADER header;

  s >> header;

  if ( header.Manufacturer != 10 || s.atEnd())
  {
    return false;
  }

  int w = header.width();
  int h = header.height();

  kdDebug( 399 ) << "Manufacturer: " << header.Manufacturer << endl;
  kdDebug( 399 ) << "Version: " << header.Version << endl;
  kdDebug( 399 ) << "Encoding: " << header.Encoding << endl;
  kdDebug( 399 ) << "Bpp: " << header.Bpp << endl;
  kdDebug( 399 ) << "Width: " << w << endl;
  kdDebug( 399 ) << "Height: " << h << endl;
  kdDebug( 399 ) << "Window: " << header.XMin << "," << header.XMax << ","
                 << header.YMin << "," << header.YMax << endl;
  kdDebug( 399 ) << "BytesPerLine: " << header.BytesPerLine << endl;
  kdDebug( 399 ) << "NPlanes: " << header.NPlanes << endl;

  QImage img;

  if ( header.Bpp == 1 && header.NPlanes == 1 )
  {
    readImage1( img, s, header );
  }
  else if ( header.Bpp == 1 && header.NPlanes == 4 )
  {
    readImage4( img, s, header );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 1 )
  {
    readImage8( img, s, header );
  }
  else if ( header.Bpp == 8 && header.NPlanes == 3 )
  {
    readImage24( img, s, header );
  }

  kdDebug( 399 ) << "Image Bytes: " << img.numBytes() << endl;
  kdDebug( 399 ) << "Image Bytes Per Line: " << img.bytesPerLine() << endl;
  kdDebug( 399 ) << "Image Depth: " << img.depth() << endl;

  if ( !img.isNull() )
  {
    *outImage = img;
    return true;
  }
  else
  {
    return false;
  }
}

bool PCXHandler::write(const QImage &image)
{
  QDataStream s( device() );
  s.setByteOrder( QDataStream::LittleEndian );

  QImage img = image;

  int w = img.width();
  int h = img.height();

  kdDebug( 399 ) << "Width: " << w << endl;
  kdDebug( 399 ) << "Height: " << h << endl;
  kdDebug( 399 ) << "Depth: " << img.depth() << endl;
  kdDebug( 399 ) << "BytesPerLine: " << img.bytesPerLine() << endl;
  kdDebug( 399 ) << "Num Colors: " << img.numColors() << endl;

  PCXHEADER header;

  header.Manufacturer = 10;
  header.Version = 5;
  header.Encoding = 1;
  header.XMin = 0;
  header.YMin = 0;
  header.XMax = w-1;
  header.YMax = h-1;
  header.HDpi = 300;
  header.YDpi = 300;
  header.Reserved = 0;
  header.PaletteInfo =1;

  if ( img.depth() == 1 )
  {
    writeImage1( img, s, header );
  }
  else if ( img.depth() == 8 && img.numColors() <= 16 )
  {
    writeImage4( img, s, header );
  }
  else if ( img.depth() == 8 )
  {
    writeImage8( img, s, header );
  }
  else if ( img.depth() == 32 )
  {
    writeImage24( img, s, header );
  }

  return true;
}

QByteArray PCXHandler::name() const
{
    return "pcx";
}

bool PCXHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("PCXHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[1];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0)
                device->ungetChar(head[readBytes-- - 1]);
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head, "\012", 1) == 0;
}

class PCXPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList PCXPlugin::keys() const
{
    return QStringList() << "pcx" << "PCX";
}

QImageIOPlugin::Capabilities PCXPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pcx" || format == "PCX")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && PCXHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}

QImageIOHandler *PCXPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new PCXHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(PCXPlugin)
Q_EXPORT_PLUGIN2(pcx, PCXPlugin)

/* vim: et sw=2 ts=2
*/


/* This file is part of the KDE project
   Copyright (C) 2002 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PCX_H
#define PCX_H

#include <qglobal.h>
#include <qdatastream.h>
#include <qcolor.h>

class QImageIO;

extern "C"
{
  void kimgio_pcx_read( QImageIO * );
  void kimgio_pcx_write( QImageIO * );
}

class RGB
{
  public:
    RGB() { }

    RGB( const QRgb color )
    {
      r = qRed( color );
      g = qGreen( color );
      b = qBlue( color );
    }

    Q_UINT8 r;
    Q_UINT8 g;
    Q_UINT8 b;
};

class Palette
{
  public:
    Palette() { }

    void setColor( int i, const QRgb color )
    {
      rgb[ i ] = RGB( color );
    }

    QRgb color( int i )
    {
      return qRgb( rgb[ i ].r, rgb[ i ].g, rgb[ i ].b );
    }

    struct RGB rgb[ 16 ];
};

class PCXHEADER
{
  public:
    PCXHEADER()
    {
      reset();
    }

    void reset();

    Q_UINT8  Manufacturer;    // Constant Flag, 10 = ZSoft .pcx
    Q_UINT8  Version;         // Version information·
                              // 0 = Version 2.5 of PC Paintbrush·
                              // 2 = Version 2.8 w/palette information·
                              // 3 = Version 2.8 w/o palette information·
                              // 4 = PC Paintbrush for Windows(Plus for
                              //     Windows uses Ver 5)·
                              // 5 = Version 3.0 and > of PC Paintbrush
                              //     and PC Paintbrush +, includes
                              //     Publisher's Paintbrush . Includes
                              //     24-bit .PCX files·
    Q_UINT8  Encoding;        // 1 = .PCX run length encoding
    Q_UINT8  Bpp;             // Number of bits to represent a pixel
                              // (per Plane) - 1, 2, 4, or 8·
    Q_UINT16 XMin;
    Q_UINT16 YMin;
    Q_UINT16 XMax;
    Q_UINT16 YMax;
    Q_UINT16 HDpi;
    Q_UINT16 YDpi;
    Palette  ColorMap;
    Q_UINT8  Reserved;        // Should be set to 0.
    Q_UINT8  NPlanes;         // Number of color planes
    Q_UINT16 BytesPerLine;    // Number of bytes to allocate for a scanline
                              // plane.  MUST be an EVEN number.  Do NOT
                              // calculate from Xmax-Xmin.·
    Q_UINT16 PaletteInfo;     // How to interpret palette- 1 = Color/BW,
                              // 2 = Grayscale ( ignored in PB IV/ IV + )·
    Q_UINT16 HScreenSize;     // Horizontal screen size in pixels. New field
                              // found only in PB IV/IV Plus
    Q_UINT16 VScreenSize;     // Vertical screen size in pixels. New field
                              // found only in PB IV/IV Plus
};

#endif // PCX_H

/* vim: et sw=2 ts=2
*/

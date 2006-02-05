/* This file is part of the KDE project
   Copyright (C) 2002-2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PCX_H
#define PCX_H


#include <QImageIOHandler>
#include <QDataStream>
#include <QColor>

class PCXHandler : public QImageIOHandler
{
public:
    PCXHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

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

    quint8 r;
    quint8 g;
    quint8 b;
};

class Palette
{
  public:
    Palette() { }

    void setColor( int i, const QRgb color )
    {
      rgb[ i ] = RGB( color );
    }

    QRgb color( int i ) const
    {
      return qRgb( rgb[ i ].r, rgb[ i ].g, rgb[ i ].b );
    }

    struct RGB rgb[ 16 ];
};

class PCXHEADER
{
  public:
    PCXHEADER();

    inline int width() const { return ( XMax-XMin ) + 1; }
    inline int height() const { return ( YMax-YMin ) + 1; }
    inline bool isCompressed() const { return ( Encoding==1 ); }

    quint8  Manufacturer;    // Constant Flag, 10 = ZSoft .pcx
    quint8  Version;         // Version information·
                              // 0 = Version 2.5 of PC Paintbrush·
                              // 2 = Version 2.8 w/palette information·
                              // 3 = Version 2.8 w/o palette information·
                              // 4 = PC Paintbrush for Windows(Plus for
                              //     Windows uses Ver 5)·
                              // 5 = Version 3.0 and > of PC Paintbrush
                              //     and PC Paintbrush +, includes
                              //     Publisher's Paintbrush . Includes
                              //     24-bit .PCX files·
    quint8  Encoding;        // 1 = .PCX run length encoding
    quint8  Bpp;             // Number of bits to represent a pixel
                              // (per Plane) - 1, 2, 4, or 8·
    quint16 XMin;
    quint16 YMin;
    quint16 XMax;
    quint16 YMax;
    quint16 HDpi;
    quint16 YDpi;
    Palette  ColorMap;
    quint8  Reserved;        // Should be set to 0.
    quint8  NPlanes;         // Number of color planes
    quint16 BytesPerLine;    // Number of bytes to allocate for a scanline
                              // plane.  MUST be an EVEN number.  Do NOT
                              // calculate from Xmax-Xmin.·
    quint16 PaletteInfo;     // How to interpret palette- 1 = Color/BW,
                              // 2 = Grayscale ( ignored in PB IV/ IV + )·
    quint16 HScreenSize;     // Horizontal screen size in pixels. New field
                              // found only in PB IV/IV Plus
    quint16 VScreenSize;     // Vertical screen size in pixels. New field
                              // found only in PB IV/IV Plus
} KDE_PACKED;

#endif // PCX_H

/* vim: et sw=2 ts=2
*/

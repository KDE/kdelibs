
/*
 * $Id$
 * kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 *
 */   

#include <cstring>
#include <cstdlib>
#include <valarray>

#include <qimage.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qpaintdevicemetrics.h>

#include "ico.h"

namespace
{
    // Global header
    struct IcoHeader
    {
        enum Type { Icon = 1, Cursor };
        Q_UINT16 reserved;
        Q_UINT16 type;
        Q_UINT16 count;
    };

    inline QDataStream& operator >>( QDataStream& s, IcoHeader& h )
    {
        return s >> h.reserved >> h.type >> h.count;
    }

    // Based on qt_read_dib et al. from qimage.cpp
    // (c) 1992-2002 Trolltech AS.
    struct BMP_INFOHDR
    {
        static const Q_UINT32 Size = 40;
        Q_UINT32  biSize;                // size of this struct
        Q_UINT32  biWidth;               // pixmap width
        Q_UINT32  biHeight;              // pixmap height
        Q_UINT16  biPlanes;              // should be 1
        Q_UINT16  biBitCount;            // number of bits per pixel
        enum Compression { RGB = 0 };
        Q_UINT32  biCompression;         // compression method
        Q_UINT32  biSizeImage;           // size of image
        Q_UINT32  biXPelsPerMeter;       // horizontal resolution
        Q_UINT32  biYPelsPerMeter;       // vertical resolution
        Q_UINT32  biClrUsed;             // number of colors used
        Q_UINT32  biClrImportant;        // number of important colors
    };
    const Q_UINT32 BMP_INFOHDR::Size;
  
    QDataStream& operator >>( QDataStream &s, BMP_INFOHDR &bi )
    {
        s >> bi.biSize;
        if ( bi.biSize == BMP_INFOHDR::Size )
        {
            s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
            s >> bi.biCompression >> bi.biSizeImage;
            s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
            s >> bi.biClrUsed >> bi.biClrImportant;
        }
        return s;
    }

#if 0
    QDataStream &operator<<( QDataStream &s, const BMP_INFOHDR &bi )
    {
        s << bi.biSize;
        s << bi.biWidth << bi.biHeight;
        s << bi.biPlanes;
        s << bi.biBitCount;
        s << bi.biCompression;
        s << bi.biSizeImage;
        s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
        s << bi.biClrUsed << bi.biClrImportant;
        return s;
    }
#endif

    // Header for every icon in the file
    struct IconRec
    {
        unsigned char width;
        unsigned char height;
        Q_UINT16 colors;
        Q_UINT16 hotspotX;
        Q_UINT16 hotspotY;
        Q_UINT32 dibSize;
        Q_UINT32 dibOffset;
    };

    inline QDataStream& operator >>( QDataStream& s, IconRec& r )
    {
        return s >> r.width >> r.height >> r.colors
                 >> r.hotspotX >> r.hotspotY
                 >> r.dibSize >> r.dibOffset;
    }
}

extern "C" void kimgio_ico_read( QImageIO* io )
{
    QIODevice::Offset offset = io->ioDevice()->at();

    QDataStream stream( io->ioDevice() );
    stream.setByteOrder( QDataStream::LittleEndian );
    IcoHeader header;
    stream >> header;
    if ( !header.count ||
         ( header.type != IcoHeader::Icon && header.type != IcoHeader::Cursor) )
        return;

    QPaintDeviceMetrics metrics( QApplication::desktop() );
    unsigned preferredSize = 32;
    unsigned preferredDepth = metrics.depth() > 8 ? 0 : 16;
    if ( io->parameters() )
    {
        QStringList params = QStringList::split( ':', io->parameters() );
        if ( params.count() ) preferredSize = params[ 0 ].toUInt();
        if ( params.count() >= 2 ) preferredDepth = params[ 1 ].toInt();
    }

    QValueList< IconRec > icons;
    unsigned best = 0;
    for ( unsigned i = 0; i < header.count; ++i )
    {
        if ( stream.atEnd() ) return;
        IconRec current;
        stream >> current;
        icons.append( current );
        unsigned d1 = std::abs( int( current.width - preferredSize ) );
        unsigned d2 = std::abs( int( icons[ best ].width - preferredSize ) );
        if ( d2 < d1 ) continue;
        else if ( d1 < d2 ) best = i;

        if ( preferredDepth == 0 && ( icons[ best ].colors == 0 ||
                                      icons[ best ].colors > current.colors ) )
            continue;

        if ( std::abs( int( current.colors - preferredDepth ) ) <
             std::abs( int( icons[ best ].colors - preferredDepth ) ) ) best = i;
    }
    IconRec ico = icons[ best ];

    if ( io->ioDevice()->size() < ico.dibOffset + BMP_INFOHDR::Size ) return;

    io->ioDevice()->at( offset + ico.dibOffset );
    BMP_INFOHDR dibHeader;
    stream >> dibHeader;
    if ( dibHeader.biSize != BMP_INFOHDR::Size ||
         dibHeader.biCompression != BMP_INFOHDR::RGB ||
         ( dibHeader.biBitCount != 1 && dibHeader.biBitCount != 4 &&
           dibHeader.biBitCount != 8 && dibHeader.biBitCount != 24 &&
           dibHeader.biBitCount != 32 ) ||
         io->ioDevice()->size() < ico.dibOffset + ico.dibSize ||
         ico.dibSize < dibHeader.biSize ) return;

    unsigned colors = dibHeader.biBitCount >= 24 ?
                      0 : dibHeader.biClrUsed ?
                      dibHeader.biClrUsed : 1 << dibHeader.biBitCount;
    // Always create a 32-bit image to get the mask right
    QImage icon( ico.width, ico.height, 32 );
    if ( icon.isNull() ) return;
    icon.setAlphaBuffer( true );

    std::valarray< QRgb > colorTable( QRgb( 0 ), 1 << dibHeader.biBitCount );
    for ( unsigned i = 0; i < colors; ++i )
    {
        unsigned char rgb[ 4 ];
        stream.readRawBytes( reinterpret_cast< char* >( &rgb ), sizeof( rgb ) );
        colorTable[ i ] = qRgb( rgb[ 2 ], rgb[ 1 ], rgb[ 0 ] );
    }

    unsigned bpl;
    switch ( dibHeader.biBitCount )
    {
        // 8-bit aligned
        case 1: bpl = ( ico.width + 7 ) >> 3; break;
        case 4: bpl = ( ico.width + 1 ) >> 1; break;
        case 8: bpl = ico.width; break;
        // 32-bit aligned
        case 24: bpl = ( ( ico.width * 3 + 3 ) >> 2 ) << 2; break;
        case 32: bpl = ico.width << 2; break;
    }
    unsigned char* buf = new unsigned char[ bpl ];
    unsigned char** lines = icon.jumpTable();
    for ( unsigned y = ico.height; y--; )
    {
        stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
        unsigned char* pixel = buf;
        QRgb* p = reinterpret_cast< QRgb* >( lines[ y ] );
        switch ( dibHeader.biBitCount )
        {
            case 1:
                for ( unsigned x = 0; x < ico.width; ++x )
                    *p++ = colorTable[ ( pixel[ x >> 3 ] >>
                                         ( 7 - ( x & 0x07 ) ) ) & 1 ];
                break;
            case 4:
                for ( unsigned x = 0; x < ico.width; ++x )
                    if ( x & 1 ) *p++ = colorTable[ pixel[ x >> 1 ] >> 4 ];
                    else *p++ = colorTable[ pixel[ x >> 1 ] & 0x0f ];
                break;
            case 8:
                for ( unsigned x = 0; x < ico.width; ++x )
                    *p++ = colorTable[ pixel[ x ] ];
                break;
            case 24:
                for ( unsigned x = 0; x < ico.width; ++x )
                    *p++ = qRgb( pixel[ 3 * x + 2 ],
                                 pixel[ 3 * x + 1 ],
                                 pixel[ 3 * x ] );
                break;
            case 32:
                for ( unsigned x = 0; x < ico.width; ++x )
                    *p++ = qRgba( pixel[ 3 * x + 3 ],
                                  pixel[ 3 * x + 2 ],
                                  pixel[ 3 * x + 1 ],
                                  pixel[ 3 * x ] );
                break;
        }
    }
    delete[] buf;

    if ( dibHeader.biBitCount < 32 )
    {
        // Traditional 1-bit mask
        bpl = ( ico.width + 7 ) >> 3;
        buf = new unsigned char[ bpl ];
        for ( unsigned y = ico.height; y--; )
        {
            stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
            QRgb* p = reinterpret_cast< QRgb* >( lines[ y ] );
            for ( unsigned x = 0; x < ico.width; ++x, ++p )
                if ( ( ( buf[ x >> 3 ] >> ( 7 - ( x & 0x07 ) ) ) & 1 ) )
                    *p &= RGB_MASK;
        }
        delete[] buf;
    }

    if ( header.type == IcoHeader::Cursor )
    {
        icon.setText( "X-HotspotX", 0, QString::number( ico.hotspotX ) );
        icon.setText( "X-HotspotY", 0, QString::number( ico.hotspotY ) );
    }
    io->setImage(icon);
    io->setStatus(0);
}

#if 0
void kimgio_ico_write(QImageIO *io)
{
    if (io->image().isNull())
        return;

    QByteArray dibData;
    QDataStream dib(dibData, IO_ReadWrite);
    dib.setByteOrder(QDataStream::LittleEndian);

    QImage pixels = io->image();
    QImage mask;
    if (io->image().hasAlphaBuffer())
        mask = io->image().createAlphaMask();
    else
        mask = io->image().createHeuristicMask();
    mask.invertPixels();
    for ( int y = 0; y < pixels.height(); ++y )
        for ( int x = 0; x < pixels.width(); ++x )
            if ( mask.pixel( x, y ) == 0 ) pixels.setPixel( x, y, 0 );

    if (!qt_write_dib(dib, pixels))
        return;

   uint hdrPos = dib.device()->at();
    if (!qt_write_dib(dib, mask))
        return;
    memmove(dibData.data() + hdrPos, dibData.data() + hdrPos + BMP_WIN + 8, dibData.size() - hdrPos - BMP_WIN - 8);
    dibData.resize(dibData.size() - BMP_WIN - 8);
        
    QDataStream ico(io->ioDevice());
    ico.setByteOrder(QDataStream::LittleEndian);
    IcoHeader hdr;
    hdr.reserved = 0;
    hdr.type = Icon;
    hdr.count = 1;
    ico << hdr.reserved << hdr.type << hdr.count;
    IconRec rec;
    rec.width = io->image().width();
    rec.height = io->image().height();
    if (io->image().numColors() <= 16)
        rec.colors = 16;
    else if (io->image().depth() <= 8)
        rec.colors = 256;
    else
        rec.colors = 0;
    rec.hotspotX = 0;
    rec.hotspotY = 0;
    rec.dibSize = dibData.size();
    ico << rec.width << rec.height << rec.colors
        << rec.hotspotX << rec.hotspotY << rec.dibSize;
    rec.dibOffset = ico.device()->at() + sizeof(rec.dibOffset);
    ico << rec.dibOffset;

    BMP_INFOHDR dibHeader;
    dib.device()->at(0);
    dib >> dibHeader;
    dibHeader.biHeight = io->image().height() << 1;
    dib.device()->at(0);
    dib << dibHeader;

    ico.writeRawBytes(dibData.data(), dibData.size());
    io->setStatus(0);
}
#endif

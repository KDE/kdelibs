
/*
 * $Id: ico.cpp 392281 2005-02-22 16:29:46Z orlovich $
 * kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 *
 */   

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include <qimage.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qmemarray.h>
#include <qpaintdevicemetrics.h>

#include <kdelibs_export.h>

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
        Q_UINT32 size;
        Q_UINT32 offset;
    };

    inline QDataStream& operator >>( QDataStream& s, IconRec& r )
    {
        return s >> r.width >> r.height >> r.colors
                 >> r.hotspotX >> r.hotspotY >> r.size >> r.offset;
    }

    struct LessDifference
    {
        LessDifference( unsigned s, unsigned c )
            : size( s ), colors( c ) {}

        bool operator ()( const IconRec& lhs, const IconRec& rhs ) const
        {
            // closest size match precedes everything else
            if ( std::abs( int( lhs.width - size ) ) <
                 std::abs( int( rhs.width - size ) ) ) return true;
            else if ( std::abs( int( lhs.width - size ) ) >
                 std::abs( int( rhs.width - size ) ) ) return false;
            else if ( colors == 0 )
            {
                // high/true color requested
                if ( lhs.colors == 0 ) return true;
                else if ( rhs.colors == 0 ) return false;
                else return lhs.colors > rhs.colors;
            }
            else
            {
                // indexed icon requested
                if ( lhs.colors == 0 && rhs.colors == 0 ) return false;
                else if ( lhs.colors == 0 ) return false;
                else return std::abs( int( lhs.colors - colors ) ) <
                            std::abs( int( rhs.colors - colors ) );
            }
        }
        unsigned size;
        unsigned colors;
    };

    bool loadFromDIB( QDataStream& stream, const IconRec& rec, QImage& icon )
    {
        BMP_INFOHDR header;
        stream >> header;
        if ( stream.atEnd() || header.biSize != BMP_INFOHDR::Size ||
             header.biSize > rec.size ||
             header.biCompression != BMP_INFOHDR::RGB ||
             ( header.biBitCount != 1 && header.biBitCount != 4 &&
               header.biBitCount != 8 && header.biBitCount != 24 &&
               header.biBitCount != 32 ) ) return false;

        unsigned paletteSize, paletteEntries;

        if (header.biBitCount > 8)
        {
            paletteEntries = 0;
            paletteSize    = 0;
        }
        else
        {
            paletteSize    = (1 << header.biBitCount);
            paletteEntries = paletteSize;
            if (header.biClrUsed && header.biClrUsed < paletteSize)
                paletteEntries = header.biClrUsed;
        }
        
        // Always create a 32-bit image to get the mask right
        // Note: this is safe as rec.width, rec.height are bytes
        icon.create( rec.width, rec.height, 32 );
        if ( icon.isNull() ) return false;
        icon.setAlphaBuffer( true );

        QMemArray< QRgb > colorTable( paletteSize );
        
        colorTable.fill( QRgb( 0 ) );
        for ( unsigned i = 0; i < paletteEntries; ++i )
        {
            unsigned char rgb[ 4 ];
            stream.readRawBytes( reinterpret_cast< char* >( &rgb ),
                                 sizeof( rgb ) );
            colorTable[ i ] = qRgb( rgb[ 2 ], rgb[ 1 ], rgb[ 0 ] );
        }

        unsigned bpl = ( rec.width * header.biBitCount + 31 ) / 32 * 4;
        
        unsigned char* buf = new unsigned char[ bpl ];
        unsigned char** lines = icon.jumpTable();
        for ( unsigned y = rec.height; !stream.atEnd() && y--; )
        {
            stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
            unsigned char* pixel = buf;
            QRgb* p = reinterpret_cast< QRgb* >( lines[ y ] );
            switch ( header.biBitCount )
            {
                case 1:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = colorTable[
                            ( pixel[ x / 8 ] >> ( 7 - ( x & 0x07 ) ) ) & 1 ];
                    break;
                case 4:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        if ( x & 1 ) *p++ = colorTable[ pixel[ x / 2 ] & 0x0f ];
                        else *p++ = colorTable[ pixel[ x / 2 ] >> 4 ];
                    break;
                case 8:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = colorTable[ pixel[ x ] ];
                    break;
                case 24:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = qRgb( pixel[ 3 * x + 2 ],
                                     pixel[ 3 * x + 1 ],
                                     pixel[ 3 * x ] );
                    break;
                case 32:
                    for ( unsigned x = 0; x < rec.width; ++x )
                        *p++ = qRgba( pixel[ 4 * x + 2 ],
                                      pixel[ 4 * x + 1 ],
                                      pixel[ 4 * x ],
                                      pixel[ 4 * x  + 3] );
                    break;
            }
        }
        delete[] buf;

        if ( header.biBitCount < 32 )
        {
            // Traditional 1-bit mask
            bpl = ( rec.width + 31 ) / 32 * 4;
            buf = new unsigned char[ bpl ];
            for ( unsigned y = rec.height; y--; )
            {
                stream.readRawBytes( reinterpret_cast< char* >( buf ), bpl );
                QRgb* p = reinterpret_cast< QRgb* >( lines[ y ] );
                for ( unsigned x = 0; x < rec.width; ++x, ++p )
                    if ( ( ( buf[ x / 8 ] >> ( 7 - ( x & 0x07 ) ) ) & 1 ) )
                        *p &= RGB_MASK;
            }
            delete[] buf;
        }
        return true;
    }
}

extern "C" KDE_EXPORT void kimgio_ico_read( QImageIO* io )
{
    QIODevice::Offset offset = io->ioDevice()->at();

    QDataStream stream( io->ioDevice() );
    stream.setByteOrder( QDataStream::LittleEndian );
    IcoHeader header;
    stream >> header;
    if ( stream.atEnd() || !header.count ||
         ( header.type != IcoHeader::Icon && header.type != IcoHeader::Cursor) )
        return;

    QPaintDeviceMetrics metrics( QApplication::desktop() );
    unsigned requestedSize = 32;
    unsigned requestedColors = metrics.depth() > 8 ? 0 : metrics.depth();
    int requestedIndex = -1;
    if ( io->parameters() )
    {
        QStringList params = QStringList::split( ';', io->parameters() );
        QMap< QString, QString > options;
        for ( QStringList::ConstIterator it = params.begin();
              it != params.end(); ++it )
        {
            QStringList tmp = QStringList::split( '=', *it );
            if ( tmp.count() == 2 ) options[ tmp[ 0 ] ] = tmp[ 1 ];
        }
        if ( options[ "index" ].toUInt() )
            requestedIndex = options[ "index" ].toUInt();
        if ( options[ "size" ].toUInt() )
            requestedSize = options[ "size" ].toUInt();
        if ( options[ "colors" ].toUInt() )
            requestedColors = options[ "colors" ].toUInt();
    }

    typedef std::vector< IconRec > IconList;
    IconList icons;
    for ( unsigned i = 0; i < header.count; ++i )
    {
        if ( stream.atEnd() ) return;
        IconRec rec;
        stream >> rec;
        icons.push_back( rec );
    }
    IconList::const_iterator selected;
    if (requestedIndex >= 0) {
        selected = std::min( icons.begin() + requestedIndex, icons.end() );
    } else {
        selected = std::min_element( icons.begin(), icons.end(),
                          LessDifference( requestedSize, requestedColors ) );
    }
    if ( stream.atEnd() || selected == icons.end() ||
         offset + selected->offset > io->ioDevice()->size() )
        return;

    io->ioDevice()->at( offset + selected->offset );
    QImage icon;
    if ( loadFromDIB( stream, *selected, icon ) )
    {
        icon.setText( "X-Index", 0, QString::number( selected - icons.begin() ) );
        if ( header.type == IcoHeader::Cursor )
        {
            icon.setText( "X-HotspotX", 0, QString::number( selected->hotspotX ) );
            icon.setText( "X-HotspotY", 0, QString::number( selected->hotspotY ) );
        }
        io->setImage(icon);
        io->setStatus(0);
    }
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

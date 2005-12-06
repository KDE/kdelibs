/* This file is part of the KDE project
   Copyright (C) 2003 Ignacio Castaño <castano@ludicon.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This code is based on Thacher Ulrich PSD loading code released
   on public domain. See: http://tulrich.com/geekstuff/
*/

/* this code supports:
 * reading:
 *     rle and raw psd files
 * writing:
 *     not supported
 */

#include "psd.h"

#include <qimage.h>
#include <qdatastream.h>

#include <kdebug.h>

typedef Q_UINT32 uint;
typedef Q_UINT16 ushort;
typedef Q_UINT8 uchar;

namespace {	// Private.

	enum ColorMode {
		CM_BITMAP = 0,
		CM_GRAYSCALE = 1,
		CM_INDEXED = 2,
		CM_RGB = 3,
		CM_CMYK = 4,
		CM_MULTICHANNEL = 7,
		CM_DUOTONE = 8,
		CM_LABCOLOR = 9
	};

	struct PSDHeader {
		uint signature;
		ushort version;
		uchar reserved[6];
		ushort channel_count;
		uint height;
		uint width;
		ushort depth;
		ushort color_mode;
	};

	static QDataStream & operator>> ( QDataStream & s, PSDHeader & header )
	{
		s >> header.signature;
		s >> header.version;
		for( int i = 0; i < 6; i++ ) {
			s >> header.reserved[i];
		}
		s >> header.channel_count;
		s >> header.height;
		s >> header.width;
		s >> header.depth;
		s >> header.color_mode;
		return s;
	}
        static bool seekBy(QDataStream& s, unsigned int bytes)
        {
                char buf[4096];
                while (bytes) {
                        unsigned int num= qMin(bytes,( unsigned int )sizeof(buf));
                        unsigned int l = num;
                        s.readRawBytes(buf, l);
                        if(l != num)
                          return false;
                        bytes -= num;
                }
                return true;
        }

	// Check that the header is a valid PSD.
	static bool IsValid( const PSDHeader & header )
	{
		if( header.signature != 0x38425053 ) {	// '8BPS'
			return false;
		}
		return true;
	}

	// Check that the header is supported.
	static bool IsSupported( const PSDHeader & header )
	{
		if( header.version != 1 ) {
			return false;
		}
		if( header.channel_count > 16 ) {
			return false;
		}
		if( header.depth != 8 ) {
			return false;
		}
		if( header.color_mode != CM_RGB ) {
			return false;
		}
		return true;
	}

	// Load the PSD image.
	static bool LoadPSD( QDataStream & s, const PSDHeader & header, QImage & img )
	{
		// Create dst image.
		if( !img.create( header.width, header.height, 32 )) {
			return false;
		}

		uint tmp;

		// Skip mode data.
		s >> tmp;
		s.device()->at( s.device()->at() + tmp );

		// Skip image resources.
		s >> tmp;
		s.device()->at( s.device()->at() + tmp );

		// Skip the reserved data.
		s >> tmp;
		s.device()->at( s.device()->at() + tmp );

		// Find out if the data is compressed.
		// Known values:
		//   0: no compression
		//   1: RLE compressed
		ushort compression;
		s >> compression;

		if( compression > 1 ) {
			// Unknown compression type.
			return false;
		}

		uint channel_num = header.channel_count;

		// Clear the image.
		if( channel_num < 4 ) {
			img.fill(qRgba(0, 0, 0, 0xFF));
		}
		else {
			// Enable alpha.
			img.setAlphaBuffer( true );

			// Ignore the other channels.
			channel_num = 4;
		}

		const uint pixel_count = header.height * header.width;

		static const uint components[4] = {2, 1, 0, 3}; // @@ Is this endian dependant?

		if( compression ) {

			// Skip row lengths.
                        if(!seekBy(s, header.height*header.channel_count*sizeof(ushort)))
                                return false;

			// Read RLE data.
			for(uint channel = 0; channel < channel_num; channel++) {

				uchar * ptr = img.bits() + components[channel];

				uint count = 0;
				while( count < pixel_count ) {
					uchar c;
                                        if(s.atEnd())
                                                return false;
					s >> c;
					uint len = c;

					if( len < 128 ) {
						// Copy next len+1 bytes literally.
						len++;
						count += len;
                                                if ( count > pixel_count )
                                                        return false;

						while( len != 0 ) {
							s >> *ptr;
							ptr += 4;
							len--;
						}
					}
					else if( len > 128 ) {
						// Next -len+1 bytes in the dest are replicated from next source byte.
						// (Interpret len as a negative 8-bit int.)
						len ^= 0xFF;
						len += 2;
						count += len;
                                                if(s.atEnd() || count > pixel_count)
                                                        return false;
						uchar val;
						s >> val;
						while( len != 0 ) {
							*ptr = val;
							ptr += 4;
							len--;
						}
					}
					else if( len == 128 ) {
						// No-op.
					}
				}
			}
		}
		else {
			// We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha, ...)
			// where each channel consists of an 8-bit value for each pixel in the image.

			// Read the data by channel.
			for(uint channel = 0; channel < channel_num; channel++) {

				uchar * ptr = img.bits() + components[channel];

				// Read the data.
				uint count = pixel_count;
				while( count != 0 ) {
					s >> *ptr;
					ptr += 4;
					count--;
				}
			}
		}

		return true;
	}

} // Private


PSDHandler::PSDHandler()
{
}

bool PSDHandler::canRead() const
{
     return canRead(device());
}

bool PSDHandler::read(QImage *image)
{
    QDataStream s( device() );
    s.setByteOrder( QDataStream::BigEndian );

    PSDHeader header;
    s >> header;

    // Check image file format.
    if( s.atEnd() || !IsValid( header ) ) {
        kdDebug(399) << "This PSD file is not valid." << endl;
        return false;
    }

    // Check if it's a supported format.
    if( !IsSupported( header ) ) {
        kdDebug(399) << "This PSD file is not supported." << endl;
        return false;
    }

    QImage img;
    if( !LoadPSD(s, header, img) ) {
        kdDebug(399) << "Error loading PSD file." << endl;
        return false;
    }

    *image = img;
    return true;
}

bool PSDHandler::write(const QImage &)
{
    // TODO Stub!
    return false;
}

QByteArray PSDHandler::name() const
{
    return "psd";
}

bool PSDHandler::canRead(QIODevice *device)
{
       if (!device) {
        qWarning("PSDHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();

    char head[4];
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

    return qstrncmp(head, "8BPS", 4) == 0;
}


class PSDPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList PSDPlugin::keys() const
{
    return QStringList() << "psd" << "PSD";
}

QImageIOPlugin::Capabilities PSDPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "psd" || format == "PSD")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && PSDHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *PSDPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new PSDHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_PLUGIN(PSDPlugin)

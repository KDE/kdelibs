
/*
 * $Id$
 * kimgio import filter for MS Windows .ico files
 *
 * Distributed under the terms of the LGPL
 * Copyright (c) 2000 Malte Starostik <malte@kde.org>
 *
 */   

#include <string.h>
#include <stdlib.h>

#include <qimage.h>
#include <qbitmap.h>
#include <qapplication.h>
#include <qpaintdevicemetrics.h>

#include "ico.h"

// Global header
struct IcoHeader
{
    Q_UINT16 reserved;
    Q_UINT16 type;
    Q_UINT16 count;
};

enum IcoType
{
    Icon = 1,
    Cursor = 2
};

// HACK HACK HACK from qt/src/kernel/qimage.cpp (malte)
// ******** The following block is copied from Qt (qimage.cpp)
// copyright (c) TrollTech AS
extern bool qt_read_dib(QDataStream &, QImage &);
extern bool qt_write_dib(QDataStream &, QImage);

const int BMP_OLD  = 12;            // old Windows/OS2 BMP size
const int BMP_WIN  = 40;            // new Windows BMP size
const int BMP_OS2  = 64;            // new OS/2 BMP size 

const int BMP_RGB  = 0;             // no compression 

struct BMP_INFOHDR {                // BMP information header
    Q_INT32  biSize;                // size of this struct
    Q_INT32  biWidth;               // pixmap width
    Q_INT32  biHeight;              // pixmap height
    Q_INT16  biPlanes;              // should be 1
    Q_INT16  biBitCount;            // number of bits per pixel
    Q_INT32  biCompression;         // compression method
    Q_INT32  biSizeImage;               // size of image
    Q_INT32  biXPelsPerMeter;           // horizontal resolution
    Q_INT32  biYPelsPerMeter;           // vertical resolution
    Q_INT32  biClrUsed;             // number of colors used
    Q_INT32  biClrImportant;            // number of important colors
};
  
QDataStream &operator>>( QDataStream &s, BMP_INFOHDR &bi )
{
    s >> bi.biSize;
    if ( bi.biSize == BMP_WIN || bi.biSize == BMP_OS2 ) {
    s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
    s >> bi.biCompression >> bi.biSizeImage;
    s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
    s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {                  // probably old Windows format
    Q_INT16 w, h;
    s >> w >> h >> bi.biPlanes >> bi.biBitCount;
    bi.biWidth  = w;
    bi.biHeight = h;
    bi.biCompression = BMP_RGB;     // no compression
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
    bi.biClrUsed = bi.biClrImportant = 0;
    }
    return s;
} 

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
// ********* END Qt copyrighted block *********

// Header for every icon in the file
struct IconRec
{
    uchar width;
    uchar height;
    Q_UINT16 colors;
    Q_UINT16 hotspotX;
    Q_UINT16 hotspotY;
    Q_UINT32 dibSize;
    Q_UINT32 dibOffset;
};

void kimgio_ico_read(QImageIO *io)
{
    QDataStream ico(io->ioDevice());
    ico.setByteOrder(QDataStream::LittleEndian);
    IcoHeader hdr;
    ico >> hdr.reserved >> hdr.type >> hdr.count;
    if (hdr.type != Icon || !hdr.count)
        return;

	QPaintDeviceMetrics metrics(QApplication::desktop());
	uchar prefSize = 32;
	uint prefDepth = metrics.depth() > 8 ? 0 : 16;
	if (io->parameters())
	{
		QStringList params = QStringList::split(':', io->parameters());
		if (params.count())
			prefSize = params[0].toInt();
		if (params.count() >= 2)
			prefDepth = params[1].toInt();
	}
	QValueList<IconRec> iconList;
	uint preferred = 0;
	for (uint i = 0; i < hdr.count; ++i)
	{
		if (ico.atEnd())
                   return;
		IconRec rec;
		ico >> rec.width >> rec.height >> rec.colors
			>> rec.hotspotX >> rec.hotspotY >> rec.dibSize >> rec.dibOffset;
		iconList.append(rec);
		if (abs(rec.width - prefSize) > abs(iconList[preferred].width - prefSize))
			continue;
		if (abs(rec.width - prefSize) < abs(iconList[preferred].width - prefSize))
			preferred = i;
        if (prefDepth == 0 && (iconList[preferred].colors == 0 || iconList[preferred].colors > rec.colors))
            continue;
        if (abs(int(rec.colors - prefDepth)) < abs(int(iconList[preferred].colors - prefDepth)))
			preferred = i;
	}
	IconRec header = iconList[preferred];

    if (ico.device()->size() < header.dibOffset + BMP_WIN)
       return;

    ico.device()->at(header.dibOffset);
    BMP_INFOHDR dibHeader;
    ico >> dibHeader;
    if ((dibHeader.biSize != BMP_WIN) &&
        (dibHeader.biSize != BMP_OLD) &&
        (dibHeader.biSize != BMP_OS2))
       return;
    int dibDataSize = header.dibSize - dibHeader.biSize;
    if ((dibDataSize < 0) || 
        (ico.device()->size() < header.dibOffset + dibHeader.biSize + dibDataSize))
       return;
    ico.device()->at(header.dibOffset + dibHeader.biSize);

    QByteArray dibData(dibDataSize + BMP_WIN);
    QDataStream dib(dibData, IO_ReadWrite);
    dib.setByteOrder(QDataStream::LittleEndian);
    dibHeader.biSize = BMP_WIN;
    dibHeader.biHeight = header.height;
    dib << dibHeader;
    ico.device()->readBlock(dibData.data() + BMP_WIN, dibDataSize);
    dib.device()->at(0);
    
    QImage icon;
    if (!qt_read_dib(dib, icon))
        return;
    if (icon.width() != header.width || icon.height() != header.height)
        return;

    QPixmap p;
    p.convertFromImage(icon);
    dibHeader.biBitCount = 1;
    dibHeader.biClrUsed = 2;
    dibHeader.biClrImportant = 2;
    int maskPos = dib.device()->at();
    dib.device()->at(0);
    dib << dibHeader;
    dib << (Q_UINT32)0;
    dib << (Q_UINT32)0xffffff;
    memcpy(dibData.data() + BMP_WIN + 8, dibData.data() + maskPos, dibData.size() - maskPos);
    dib.device()->at(0);
    if (!qt_read_dib(dib, icon))
        return;
    if (icon.width() != header.width || icon.height() != header.height)
        return;
        
    QBitmap mask;
    mask.convertFromImage(icon);
    p.setMask(mask);
        
    io->setImage(p.convertToImage());
    io->setStatus(0);
}

void kimgio_ico_write(QImageIO *io)
{
    if (io->image().isNull())
        return;

    QByteArray dibData;
    QDataStream dib(dibData, IO_ReadWrite);
    dib.setByteOrder(QDataStream::LittleEndian);
    if (!qt_write_dib(dib, io->image()))
        return;

    QImage mask;
    if (io->image().hasAlphaBuffer())
        mask = io->image().createAlphaMask();
    else
        mask = io->image().createHeuristicMask();
    mask.invertPixels();

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


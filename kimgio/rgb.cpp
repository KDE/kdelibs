// kimgio module for SGI images
//
// Copyright (C) 2004  Melchior FRANZ  <mfranz@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Lesser GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.


/* this code supports:
 * reading:
 *     everything, except images with 1 dimension or images with
 *     mapmode != NORMAL (e.g. dithered); Images with 16 bit
 *     precision or more than 4 layers are stripped down.
 * writing:
 *     Run Length Encoded (RLE) or Verbatim (uncompressed)
 *     (whichever is smaller)
 *
 * Please report if you come across rgb/rgba/sgi/bw files that aren't
 * recognized. Also report applications that can't deal with images
 * saved by this filter.
 */


#include "rgb.h"
#include <QtGui/QImage>
#include <kdebug.h>


SGIImage::SGIImage(QIODevice *io) :
    _starttab(0),
    _lengthtab(0)
{
    _dev = io;
    _stream.setDevice(_dev);
}


SGIImage::~SGIImage()
{
    delete[] _starttab;
    delete[] _lengthtab;
}


///////////////////////////////////////////////////////////////////////////////


bool SGIImage::getRow(uchar *dest)
{
    int n, i;
    if (!_rle) {
        for (i = 0; i < _xsize; i++) {
            if (_pos >= _data.end())
                return false;
            dest[i] = uchar(*_pos);
            _pos += _bpc;
        }
        return true;
    }

    for (i = 0; i < _xsize;) {
        if (_bpc == 2)
            _pos++;
        n = *_pos & 0x7f;
        if (!n)
            break;

        if (*_pos++ & 0x80) {
            for (; i < _xsize && n--; i++) {
                *dest++ = *_pos;
                _pos += _bpc;
            }
        } else {
            for (; i < _xsize && n--; i++)
                *dest++ = *_pos;

            _pos += _bpc;
        }
    }
    return i == _xsize;
}


bool SGIImage::readData(QImage& img)
{
    QRgb *c;
    quint32 *start = _starttab;
    QByteArray lguard(_xsize, 0);
    uchar *line = (uchar *)lguard.data();
    unsigned x, y;

    if (!_rle)
        _pos = _data.begin();

    for (y = 0; y < _ysize; y++) {
        if (_rle)
            _pos = _data.begin() + *start++;
        if (!getRow(line))
            return false;
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++)
            *c = qRgb(line[x], line[x], line[x]);
    }

    if (_zsize == 1)
        return true;

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            if (_rle)
                _pos = _data.begin() + *start++;
            if (!getRow(line))
                return false;
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++)
                *c = qRgb(qRed(*c), line[x], line[x]);
        }

        for (y = 0; y < _ysize; y++) {
            if (_rle)
                _pos = _data.begin() + *start++;
            if (!getRow(line))
                return false;
            c = (QRgb *)img.scanLine(_ysize - y - 1);
            for (x = 0; x < _xsize; x++, c++)
                *c = qRgb(qRed(*c), qGreen(*c), line[x]);
        }

        if (_zsize == 3)
            return true;
    }

    for (y = 0; y < _ysize; y++) {
        if (_rle)
            _pos = _data.begin() + *start++;
        if (!getRow(line))
            return false;
        c = (QRgb *)img.scanLine(_ysize - y - 1);
        for (x = 0; x < _xsize; x++, c++)
            *c = qRgba(qRed(*c), qGreen(*c), qBlue(*c), line[x]);
    }

    return true;
}


bool SGIImage::readImage(QImage& img)
{
    qint8 u8;
    qint16 u16;
    qint32 u32;

    kDebug(399) << "reading rgb ";

    // magic
    _stream >> u16;
    if (u16 != 0x01da)
        return false;

    // verbatim/rle
    _stream >> _rle;
    kDebug(399) << (_rle ? "RLE" : "verbatim");
    if (_rle > 1)
        return false;

    // bytes per channel
    _stream >> _bpc;
    kDebug(399) << "bytes per channel: " << int(_bpc);
    if (_bpc == 1)
        ;
    else if (_bpc == 2)
        kDebug(399) << "dropping least significant byte";
    else
        return false;

    // number of dimensions
    _stream >> _dim;
    kDebug(399) << "dimensions: " << _dim;
    if (_dim < 1 || _dim > 3)
        return false;

    _stream >> _xsize >> _ysize >> _zsize >> _pixmin >> _pixmax >> u32;
    kDebug(399) << "x: " << _xsize;
    kDebug(399) << "y: " << _ysize;
    kDebug(399) << "z: " << _zsize;

    // name
    _stream.readRawData(_imagename, 80);
    _imagename[79] = '\0';

    _stream >> _colormap;
    kDebug(399) << "colormap: " << _colormap;
    if (_colormap != NORMAL)
        return false;        // only NORMAL supported

    for (int i = 0; i < 404; i++)
        _stream >> u8;

    if (_dim == 1) {
        kDebug(399) << "1-dimensional images aren't supported yet";
        return false;
    }

    if( _stream.atEnd())
        return false;

    _numrows = _ysize * _zsize;

    img = QImage( _xsize, _ysize, QImage::Format_RGB32 );

    if (_zsize == 2 || _zsize == 4)
        img = img.convertToFormat(QImage::Format_ARGB32);
    else if (_zsize > 4)
        kDebug(399) << "using first 4 of " << _zsize << " channels";

    if (_rle) {
        uint l;
        _starttab = new quint32[_numrows];
        for (l = 0; !_stream.atEnd() && l < _numrows; l++) {
            _stream >> _starttab[l];
            _starttab[l] -= 512 + _numrows * 2 * sizeof(quint32);
        }

        _lengthtab = new quint32[_numrows];
        for (l = 0; l < _numrows; l++)
            _stream >> _lengthtab[l];
    }

    _data = _dev->readAll();

    // sanity check
    if (_rle)
        for (uint o = 0; o < _numrows; o++)
            // don't change to greater-or-equal!
            if (_starttab[o] + _lengthtab[o] > (uint)_data.size()) {
                kDebug(399) << "image corrupt (sanity check failed)";
                return false;
            }

    if (!readData(img)) {
        kDebug(399) << "image corrupt (incomplete scanline)";
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////


void RLEData::write(QDataStream& s)
{
    for (int i = 0; i < size(); i++)
        s << at(i);
}


bool RLEData::operator<(const RLEData& b) const
{
    uchar ac, bc;
    for (int i = 0; i < qMin(size(), b.size()); i++) {
        ac = at(i);
        bc = b[i];
        if (ac != bc)
            return ac < bc;
    }
    return size() < b.size();
}


uint RLEMap::insert(const uchar *d, uint l)
{
    RLEData data = RLEData(d, l, _offset);
    Iterator it = find(data);
    if (it != end())
        return it.value();

    _offset += l;
    return QMap<RLEData, uint>::insert(data, _counter++).value();
}


QVector<const RLEData*> RLEMap::vector()
{
    QVector<const RLEData*> v(size());
    for (Iterator it = begin(); it != end(); ++it)
        v.replace(it.value(), &it.key());

    return v;
}


uchar SGIImage::intensity(uchar c)
{
    if (c < _pixmin)
        _pixmin = c;
    if (c > _pixmax)
        _pixmax = c;
    return c;
}


uint SGIImage::compact(uchar *d, uchar *s)
{
    uchar *dest = d, *src = s, patt, *t, *end = s + _xsize;
    int i, n;
    while (src < end) {
        for (n = 0, t = src; t + 2 < end && !(*t == t[1] && *t == t[2]); t++)
            n++;

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = 0x80 | i;
            while (i--)
                *dest++ = *src++;
        }

        if (src == end)
            break;

        patt = *src++;
        for (n = 1; src < end && *src == patt; src++)
            n++;

        while (n) {
            i = n > 126 ? 126 : n;
            n -= i;
            *dest++ = i;
            *dest++ = patt;
        }
    }
    *dest++ = 0;
    return dest - d;
}


bool SGIImage::scanData(const QImage& img)
{
    quint32 *start = _starttab;
    QByteArray lineguard(_xsize * 2, 0);
    QByteArray bufguard(_xsize, 0);
    uchar *line = (uchar *)lineguard.data();
    uchar *buf = (uchar *)bufguard.data();
    const QRgb *c;
    unsigned x, y;
    uint len;

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++)
            buf[x] = intensity(qRed(*c++));
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    if (_zsize == 1)
        return true;

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++)
                buf[x] = intensity(qGreen(*c++));
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++)
                buf[x] = intensity(qBlue(*c++));
            len = compact(line, buf);
            *start++ = _rlemap.insert(line, len);
        }

        if (_zsize == 3)
            return true;
    }

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++)
            buf[x] = intensity(qAlpha(*c++));
        len = compact(line, buf);
        *start++ = _rlemap.insert(line, len);
    }

    return true;
}


void SGIImage::writeHeader()
{
    _stream << quint16(0x01da);
    _stream << _rle << _bpc << _dim;
    _stream << _xsize << _ysize << _zsize;
    _stream << _pixmin << _pixmax;
    _stream << quint32(0);

    for (int i = 0; i < 80; i++)
        _imagename[i] = '\0';
    _stream.writeRawData(_imagename, 80);

    _stream << _colormap;
    for (int i = 0; i < 404; i++)
        _stream << quint8(0);
}


void SGIImage::writeRle()
{
    _rle = 1;
    kDebug(399) << "writing RLE data";
    writeHeader();
    uint i;

    // write start table
    for (i = 0; i < _numrows; i++)
        _stream << quint32(_rlevector[_starttab[i]]->offset());

    // write length table
    for (i = 0; i < _numrows; i++)
        _stream << quint32(_rlevector[_starttab[i]]->size());

    // write data
    for (i = 0; (int)i < _rlevector.size(); i++)
        const_cast<RLEData*>(_rlevector[i])->write(_stream);
}


void SGIImage::writeVerbatim(const QImage& img)
{
    _rle = 0;
    kDebug(399) << "writing verbatim data";
    writeHeader();

    const QRgb *c;
    unsigned x, y;

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++)
            _stream << quint8(qRed(*c++));
    }

    if (_zsize == 1)
        return;

    if (_zsize != 2) {
        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++)
                _stream << quint8(qGreen(*c++));
        }

        for (y = 0; y < _ysize; y++) {
            c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
            for (x = 0; x < _xsize; x++)
                _stream << quint8(qBlue(*c++));
        }

        if (_zsize == 3)
            return;
    }

    for (y = 0; y < _ysize; y++) {
        c = reinterpret_cast<const QRgb *>(img.scanLine(_ysize - y - 1));
        for (x = 0; x < _xsize; x++)
            _stream << quint8(qAlpha(*c++));
    }
}


bool SGIImage::writeImage(const QImage& image)
{
    kDebug(399) << "writing "; // TODO add filename
    QImage img = image;
    if (img.allGray())
        _dim = 2, _zsize = 1;
    else
        _dim = 3, _zsize = 3;

    if (img.format() == QImage::Format_ARGB32)
        _dim = 3, _zsize++;

    img = img.convertToFormat(QImage::Format_RGB32);
    if (img.isNull()) {
        kDebug(399) << "can't convert image to depth 32";
        return false;
    }

    _bpc = 1;
    _xsize = img.width();
    _ysize = img.height();
    _pixmin = ~0u;
    _pixmax = 0;
    _colormap = NORMAL;
    _numrows = _ysize * _zsize;
    _starttab = new quint32[_numrows];
    _rlemap.setBaseOffset(512 + _numrows * 2 * sizeof(quint32));

    if (!scanData(img)) {
        kDebug(399) << "this can't happen";
        return false;
    }

    _rlevector = _rlemap.vector();

    long verbatim_size = _numrows * _xsize;
    long rle_size = _numrows * 2 * sizeof(quint32);
    for (int i = 0; i < _rlevector.size(); i++)
        rle_size += _rlevector[i]->size();

    kDebug(399) << "minimum intensity: " << _pixmin;
    kDebug(399) << "maximum intensity: " << _pixmax;
    kDebug(399) << "saved scanlines: " << _numrows - _rlemap.size();
    kDebug(399) << "total savings: " << (verbatim_size - rle_size) << " bytes";
    kDebug(399) << "compression: " << (rle_size * 100.0 / verbatim_size) << '%';

    if (verbatim_size <= rle_size)
        writeVerbatim(img);
    else
        writeRle();
    return true;
}


///////////////////////////////////////////////////////////////////////////////


RGBHandler::RGBHandler()
{
}


bool RGBHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("rgb");
        return true;
    }
    return false;
}


bool RGBHandler::read(QImage *outImage)
{
    SGIImage sgi(device());
    return sgi.readImage(*outImage);
}


bool RGBHandler::write(const QImage &image)
{
    SGIImage sgi(device());
    return sgi.writeImage(image);
}


QByteArray RGBHandler::name() const
{
    return "rgb";
}


bool RGBHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RGBHandler::canRead() called with no device");
        return false;
    }

    qint64 oldPos = device->pos();
    QByteArray head = device->readLine(64);
    int readBytes = head.size();

    if (device->isSequential()) {
        while (readBytes > 0)
            device->ungetChar(head[readBytes-- - 1]);

    } else {
        device->seek(oldPos);
    }

    const QRegExp regexp("^\x01\xda\x01[\x01\x02]");
    QString data(head);

    return data.contains(regexp);
}


///////////////////////////////////////////////////////////////////////////////


class RGBPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};


QStringList RGBPlugin::keys() const
{
    return QStringList() << "rgb" << "RGB" << "rgba" << "RGBA" << "bw" << "BW" << "sgi" << "SGI";
}


QImageIOPlugin::Capabilities RGBPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "rgb" || format == "RGB" || format ==  "rgba" || format == "RGBA"
            || format ==  "bw" || format == "BW" || format == "sgi" || format == "SGI")
        return Capabilities(CanRead|CanWrite);

    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && RGBHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;
}


QImageIOHandler *RGBPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RGBHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}


Q_EXPORT_STATIC_PLUGIN(RGBPlugin)
Q_EXPORT_PLUGIN2(rgb, RGBPlugin)


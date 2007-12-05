// kimgio module for SGI images
//
// Copyright (C) 2004  Melchior FRANZ  <mfranz@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Lesser GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.

#ifndef KIMG_RGB_H
#define KIMG_RGB_H


#include <QtGui/QImageIOPlugin>
#include <QtCore/QMap>
#include <QtCore/QVector>


class RGBHandler : public QImageIOHandler
{
public:
    RGBHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);
    QByteArray name() const;
    static bool canRead(QIODevice *device);
};


class RLEData : public QVector<uchar> {
public:
    RLEData() {}
    RLEData(const uchar *d, uint l, uint o) : _offset(o) {
        for (uint i = 0; i < l; i++)
            append(d[i]);
    }
    bool operator<(const RLEData&) const;
    void write(QDataStream& s);
    uint offset() const { return _offset; }

private:
    uint _offset;
};


class RLEMap : public QMap<RLEData, uint> {
public:
    RLEMap() : _counter(0), _offset(0) {}
    uint insert(const uchar *d, uint l);
    QVector<const RLEData*> vector();
    void setBaseOffset(uint o) { _offset = o; }

private:
    uint _counter;
    uint _offset;
};


class SGIImage {
public:
    SGIImage(QIODevice *device);
    ~SGIImage();

    bool readImage(QImage&);
    bool writeImage(const QImage&);

private:
    enum { NORMAL, DITHERED, SCREEN, COLORMAP }; // colormap
    QIODevice *_dev;
    QDataStream _stream;

    quint8 _rle;
    quint8 _bpc;
    quint16 _dim;
    quint16 _xsize;
    quint16 _ysize;
    quint16 _zsize;
    quint32 _pixmin;
    quint32 _pixmax;
    char _imagename[80];
    quint32 _colormap;

    quint32 *_starttab;
    quint32 *_lengthtab;
    QByteArray _data;
    QByteArray::Iterator _pos;
    RLEMap _rlemap;
    QVector<const RLEData*> _rlevector;
    uint _numrows;

    bool readData(QImage&);
    bool getRow(uchar *dest);

    void writeHeader();
    void writeRle();
    void writeVerbatim(const QImage&);
    bool scanData(const QImage&);
    uint compact(uchar *, uchar *);
    uchar intensity(uchar);
};

#endif


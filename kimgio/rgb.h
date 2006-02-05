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


#include <QImageIOHandler>
#include <QMap>
#include <QVector>

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
        RLEData(const uchar *d, uint l, uint o) : m_offset(o) {
            for (uint i = 0; i < l; ++i)
                append(d[i]);
        }
	bool operator<(const RLEData&) const;
	void write(QDataStream& s);
	void print(QString) const;				// TODO remove
	uint offset() const { return m_offset; }
private:
	uint			m_offset;
};


class RLEMap : public QMap<RLEData, uint> {
public:
	RLEMap() : m_counter(0), m_offset(0) {}
	uint insert(const uchar *d, uint l);
	QVector<const RLEData*> vector();
	void setBaseOffset(uint o) { m_offset = o; }
private:
	uint			m_counter;
	uint			m_offset;
};


class SGIImage {
public:
	SGIImage(QIODevice *device);
	~SGIImage();

	bool readImage(QImage&);
	bool writeImage(const QImage&);

private:
	enum { NORMAL, DITHERED, SCREEN, COLORMAP };		// colormap
	QIODevice		*m_dev;
	QDataStream		m_stream;

	quint8			m_rle;
	quint8			m_bpc;
	quint16		m_dim;
	quint16		m_xsize;
	quint16		m_ysize;
	quint16		m_zsize;
	quint32		m_pixmin;
	quint32		m_pixmax;
	char			m_imagename[80];
	quint32		m_colormap;

	quint32		*m_starttab;
	quint32		*m_lengthtab;
	QByteArray		m_data;
	QByteArray::Iterator	m_pos;
	RLEMap			m_rlemap;
	QVector<const RLEData*>	m_rlevector;
	uint			m_numrows;

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


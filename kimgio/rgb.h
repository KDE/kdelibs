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

#include <qmap.h>
#include <qptrvector.h>


class QImage;
class QImageIO;

extern "C" {
void kimgio_rgb_read(QImageIO *);
void kimgio_rgb_write(QImageIO *);
}


class RLEData : public QMemArray<uchar> {
public:
	RLEData() {}
	RLEData(const uchar *d, uint l, uint o) : m_offset(o) { duplicate(d, l); }
	bool operator<(const RLEData&) const;
	void write(QDataStream& s);
	void print(QString) const;				// TODO remove
	uint offset() { return m_offset; }
private:
	uint			m_offset;
};


class RLEMap : public QMap<RLEData, uint> {
public:
	RLEMap() : m_counter(0), m_offset(0) {}
	uint insert(const uchar *d, uint l);
	QPtrVector<RLEData> vector();
	void setBaseOffset(uint o) { m_offset = o; }
private:
	uint			m_counter;
	uint			m_offset;
};


class SGIImage {
public:
	SGIImage(QImageIO *);
	~SGIImage();

	bool readImage(QImage&);
	bool writeImage(QImage&);

private:
	enum { NORMAL, DITHERED, SCREEN, COLORMAP };		// colormap
	QImageIO		*m_io;
	QIODevice		*m_dev;
	QDataStream		m_stream;

	Q_UINT8			m_rle;
	Q_UINT8			m_bpc;
	Q_UINT16		m_dim;
	Q_UINT16		m_xsize;
	Q_UINT16		m_ysize;
	Q_UINT16		m_zsize;
	Q_UINT32		m_pixmin;
	Q_UINT32		m_pixmax;
	char			m_imagename[80];
	Q_UINT32		m_colormap;

	Q_UINT32		*m_starttab;
	Q_UINT32		*m_lengthtab;
	QByteArray		m_data;
	QByteArray::Iterator	m_pos;
	RLEMap			m_rlemap;
	QPtrVector<RLEData>	m_rlevector;
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


// kimgio module for SGI images
//
// Copyright (C) 2004  Melchior FRANZ  <mfranz@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Lesser GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.


#ifndef KIMG_SGI_H
#define KIMG_SGI_H

#include <qimage.h>
#include <qptrlist.h>


class QImageIO;

extern "C" {
void kimgio_rgb_read(QImageIO *);
void kimgio_rgb_write(QImageIO *);
}


class RLEPacket {
	QMemArray<uchar>	m_data;
public:
	RLEPacket(uchar *d, uint len) { m_data.duplicate(d, len); }
	QMemArray<uchar> *data() { return &m_data; }
};


class SGIImage {
	enum { NORMAL, DITHERED, SCREEN, COLORMAP };		// colormap
	typedef QPtrList<RLEPacket> RLEList;
	typedef QPtrListIterator<RLEPacket> RLEListIterator;

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
	Q_UINT32		m_colormap;
	Q_UINT32		*m_starttab;
	Q_UINT32		*m_lengthtab;
	QByteArray		m_data;
	QByteArray::Iterator	m_pos;
	RLEList			m_rlelist;

public:
	SGIImage(QImageIO *);
	~SGIImage();

	bool readImage(QImage&);
	bool writeImage(QImage&);

protected:
	bool readData(QImage&);
	bool getRow(uchar *dest);

	bool writeData(QImage&);
	uint compact(uchar *, uchar *);
	void addRlePacket(uchar *, uint);
};

#endif


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
 *     Run Length Encoded (RLE) files  (no shared patterns yet)
 *
 * Please report if you come across rgb/rgba/sgi/bw files that aren't
 * recognized. Also report applications that can't deal with images
 * saved by this filter.
 */


#include "rgb.h"
#include <kdebug.h>


///////////////////////////////////////////////////////////////////////////////


void kimgio_rgb_read(QImageIO *io)
{
	SGIImage rgb(io);
	QImage img;

	if (!rgb.readImage(img)) {
		io->setImage(0);
		io->setStatus(-1);
		return;
	}

	io->setImage(img);
	io->setStatus(0);
}


void kimgio_rgb_write(QImageIO *io)
{
	SGIImage rgb(io);
	QImage img = io->image();

	if (!rgb.writeImage(img))
		io->setStatus(-1);

	io->setStatus(0);
}


///////////////////////////////////////////////////////////////////////////////


SGIImage::SGIImage(QImageIO *io) :
	m_io(io),
	m_starttab(0),
	m_lengthtab(0)
{
	m_dev = io->ioDevice();
	m_stream.setDevice(m_dev);
	m_rlelist.setAutoDelete(true);
}


SGIImage::~SGIImage()
{
	delete[] m_starttab;
	delete[] m_lengthtab;
}


///////////////////////////////////////////////////////////////////////////////


bool SGIImage::getRow(uchar *dest)
{
	int n, i;
	if (!m_rle) {
		for (i = 0; i < m_xsize; i++) {
			*dest++ = uchar(*m_pos);
			m_pos += m_bpc;
		}
		return true;
	}

	for (i = 0; i < m_xsize;) {
		if (m_bpc == 2)
			m_pos++;
		n = *m_pos & 0x7f;
		if (!n)
			break;

		if (*m_pos++ & 0x80) {
			for (; i < m_xsize && n--; i++) {
				*dest++ = *m_pos;
				m_pos += m_bpc;
			}
		} else {
			for (; i < m_xsize && n--; i++)
				*dest++ = *m_pos;

			m_pos += m_bpc;
		}
	}
	return i == m_xsize;
}


bool SGIImage::readData(QImage& img)
{
	QRgb *c;
	Q_UINT32 *start = m_starttab;
	uchar line[m_xsize];
	unsigned x, y;

	if (!m_rle)
		m_pos = m_data.begin();

	for (y = 0; y < m_ysize; y++) {
		c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
		if (m_rle)
			m_pos = m_data.begin() + *start++;
		if (!getRow(line))
			return false;
		for (x = 0; x < m_xsize; x++, c++)
			*c = qRgb(line[x], line[x], line[x]);
	}

	if (m_zsize == 1)
		return true;

	if (m_zsize != 2) {
		for (y = 0; y < m_ysize; y++) {
			c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
			if (m_rle)
				m_pos = m_data.begin() + *start++;
			if (!getRow(line))
				return false;
			for (x = 0; x < m_xsize; x++, c++)
				*c = qRgb(qRed(*c), line[x], line[x]);
		}

		for (y = 0; y < m_ysize; y++) {
			c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
			if (m_rle)
				m_pos = m_data.begin() + *start++;
			if (!getRow(line))
				return false;
			for (x = 0; x < m_xsize; x++, c++)
				*c = qRgb(qRed(*c), qGreen(*c), line[x]);
		}

		if (m_zsize == 3)
			return true;
	}

	for (y = 0; y < m_ysize; y++) {
		c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
		if (m_rle)
			m_pos = m_data.begin() + *start++;
		if (!getRow(line))
			return false;
		for (x = 0; x < m_xsize; x++, c++)
			*c = qRgba(qRed(*c), qGreen(*c), qBlue(*c), line[x]);
	}

	return true;
}


bool SGIImage::readImage(QImage& img)
{
	Q_INT8 u8;
	Q_INT16 u16;
	Q_INT32 u32;

	// magic
	m_stream >> u16;
	//if (u16 == 0xda01)
	//	m_stream.setByteOrder(QDataStream::LittleEndian);	// not spec compliant!
	//else 
	if (u16 != 0x01da)
		return false;

	// verbatim/rle
	m_stream >> m_rle;
	kdDebug(399) << (m_rle ? "RLE" : "verbatim") << endl;
	if (m_rle > 1)
		return false;

	// bytes per channel
	m_stream >> m_bpc;
	kdDebug(399) << "bytes per channel: " << int(m_bpc) << endl;
	if (m_bpc == 1)
		;
	else if (m_bpc == 2)
		kdDebug(399) << "dropping least significant byte" << endl;
	else
		return false;

	// number of dimensions
	m_stream >> m_dim;
	kdDebug(399) << "dimensions: " << m_dim << endl;
	if (m_dim < 1 || m_dim > 3)
		return false;

	m_stream >> m_xsize >> m_ysize >> m_zsize >> m_pixmin >> m_pixmax >> u32;
	kdDebug(399) << "x: " << m_xsize << endl;
	kdDebug(399) << "y: " << m_ysize << endl;
	kdDebug(399) << "z: " << m_zsize << endl;

	// name
	for (int i = 0; i < 80; i++)
		m_stream >> u8;

	m_stream >> m_colormap;
	kdDebug(399) << "colormap: " << m_colormap << endl;
	if (m_colormap != NORMAL)
		return false;		// only NORMAL supported

	// unused
	for (int i = 0; i < 404; i++)
		m_stream >> u8;

	if (m_dim == 1) {
		kdDebug(399) << "1-dimensional images aren't supported" << endl;
		return false;
	}

	if (!img.create(m_xsize, m_ysize, 32)) {
		kdDebug(399) << "cannot create image" << endl;
		return false;
	}

	if (m_zsize == 2 || m_zsize == 4)
		img.setAlphaBuffer(true);
	else if (m_zsize > 4)
		kdDebug(399) << "using first 4 of " << m_zsize << " channels" << endl;

	if (m_rle) {
		long l, n = m_ysize * m_zsize;
		m_starttab = new Q_UINT32[n];
		for (l = 0; l < n; l++) {
			m_stream >> m_starttab[l];
			m_starttab[l] -= n * 8 + 512;
		}

		m_lengthtab = new Q_UINT32[n];
		for (l = 0; l < n; l++)
			m_stream >> m_lengthtab[l];
	}

	m_data = m_dev->readAll();

	// sanity ckeck
	if (m_rle)
		for (long o = 0; o < m_ysize * m_zsize; o++)
			if (m_starttab[o] + m_lengthtab[o] > m_data.size())
				return false;
	return readData(img);
}


///////////////////////////////////////////////////////////////////////////////


void SGIImage::addRlePacket(uchar *s, uint l)
{
	m_rlelist.append(new RLEPacket(s, l));
}


uint SGIImage::compact(uchar *d, uchar *s)
{
	uchar *dest = d, *src = s, patt, *cnt;		// make shortcut for s + m_xsize
	int n;
	while (src - s < m_xsize) {
		if (src - s + 1 == m_xsize) {		// last bit
			*dest++ = 0x81;
			*dest++ = *src;
			break;
		} else if (*src == src[1]) {
			patt = *src++;
			for (n = 1; src - s < m_xsize && n < 126 && *src == patt; src++)
				n++;
			*dest++ = n;
			*dest++ = patt;
		} else {
			cnt = dest++;
			for (n = 0; src - s < m_xsize && n < 126 && *src != src[1]; n++)
				*dest++ = *src++;
			*cnt = 0x80 | n;
		}
	}
	*dest++ = 0;
	return dest - d;
}


bool SGIImage::writeData(QImage& img)
{
	Q_UINT32 *start = m_starttab;
	Q_UINT32 *length = m_lengthtab;
	Q_UINT32 pos = 512L + m_ysize * m_zsize * 8;
	uchar line[m_xsize * 2];
	uchar buf[m_xsize];
	QRgb *c;
	unsigned x, y;
	uint len;

	for (y = 0; y < m_ysize; y++) {
		c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
		for (x = 0; x < m_xsize; x++)
			buf[x] = qRed(*c++);
		len = compact(line, buf);
		addRlePacket(line, len);
		*start++ = pos;
		*length++ = len;
		pos += len;
	}

	if (m_zsize == 1)
		return true;

	if (m_zsize != 2) {
		for (y = 0; y < m_ysize; y++) {
			c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
			for (x = 0; x < m_xsize; x++)
				buf[x] = qGreen(*c++);
			len = compact(line, buf);
			addRlePacket(line, len);
			*start++ = pos;
			*length++ = len;
			pos += len;
		}

		for (y = 0; y < m_ysize; y++) {
			c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
			for (x = 0; x < m_xsize; x++)
				buf[x] = qBlue(*c++);
			len = compact(line, buf);
			addRlePacket(line, len);
			*start++ = pos;
			*length++ = len;
			pos += len;
		}

		if (m_zsize == 3)
			return true;
	}

	for (y = 0; y < m_ysize; y++) {
		c = reinterpret_cast<QRgb*>(img.scanLine(m_ysize - y - 1));
		for (x = 0; x < m_xsize; x++)
			buf[x] = qAlpha(*c++);
		len = compact(line, buf);
		addRlePacket(line, len);
		*start++ = pos;
		*length++ = len;
		pos += len;
	}

	return true;
}


bool SGIImage::writeImage(QImage& img)
{
	m_rle = 1;
	m_bpc = 1;				// one byte per pixel & channel
	if (img.allGray())
		m_dim = 2, m_zsize = 1;
	else
		m_dim = 3, m_zsize = 3;

	if (img.hasAlphaBuffer())
		m_dim = 3, m_zsize++;

	m_xsize = img.width();
	m_ysize = img.height();
	m_pixmin = 0;				// FIXME
	m_pixmax = 255;
	m_colormap = NORMAL;

	m_stream << Q_UINT16(0x01da);		// magic
	m_stream << m_rle << m_bpc << m_dim << m_xsize << m_ysize << m_zsize << m_pixmin << m_pixmax;
	m_stream << Q_UINT32(0);		// dummy

	int i;
	for (i = 0; i < 80; i++)		// no name
		m_stream << Q_UINT8(0);

	m_stream << m_colormap;
	for (i = 0; i < 404; i++)
		m_stream << Q_UINT8(0);		// wasting space ...

	if (img.depth() != 32)
		img.convertDepth(32);

	m_starttab = new Q_UINT32[m_ysize * m_zsize];
	m_lengthtab = new Q_UINT32[m_ysize * m_zsize];

	if (!writeData(img))
		return false;

	for (i = 0; i < m_ysize * m_zsize; i++)
		m_stream << m_starttab[i];

	for (i = 0; i < m_ysize * m_zsize; i++)
		m_stream << m_lengthtab[i];

	RLEListIterator it(m_rlelist);
	RLEPacket *pack;
	for (; (pack = it.current()); ++it) {
		QMemArray<uchar> *a = pack->data();
		for (unsigned j = 0; j < a->size(); j++)
			m_stream << (*a)[j];
	}
	return true;
}



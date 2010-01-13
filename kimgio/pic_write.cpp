/**
 * PIC_RW - Qt PIC Support
 * Copyright (C) 2007 Ruben Lopez <r.lopez@bren.es>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------------
 */

/* This code is based on the GIMP-PIC plugin by Halfdan Ingvarsson, 
 * and relicensed from GPL to LGPL to accomodate the KDE licensing policy
 * with his permission.
 * These is the original copyright:
 * Copyright (C) 1998 Halfdan Ingvarsson
 */

#include "pic_rw.h"
#include <netinet/in.h>
#include <iostream>
#include <qimage.h>

/**
 * Writes the PIC header info.
 * @param dev IO Device
 * @param msg Header message
 * @param width Image width
 * @param height Image height
 * @param alpha Image has alpha?
 * @return True on success
 */
static bool writeHeader(QIODevice *dev, std::string msg, unsigned width, unsigned height, bool alpha) {
    PICHeader h;
    PICChannel c;
    unsigned count = 0;

    memset(&h, 0, sizeof (PICHeader));
    h.magic = htonl(PIC_MAGIC_NUMBER);
    h.version = 3.71f;
    strcpy(h.comment, msg.c_str());
    strncpy(h.id, "PICT", 4);
    h.width = htons(width);
    h.height = htons(height);
    h.ratio = 1.0f;
    h.fields = htons(BOTH);
    count = dev->write((const char*) & h, sizeof (PICHeader));
    if (count != sizeof (PICHeader)) {
        return false;
    }

    memset(&c, 0, sizeof (PICChannel));
    c.size = 8;
    c.type = RLE;
    c.channel = RED | GREEN | BLUE;
    if (alpha) {
        c.chained = 1;
    }
    count = dev->write((const char*) & c, sizeof (PICChannel));
    if (count != sizeof (PICChannel)) {
        return false;
    }

    if (alpha) {
        c.channel = ALPHA;
        c.chained = 0;
        count = dev->write((const char*) & c, sizeof (PICChannel));
        if (count != sizeof (PICChannel)) {
            return false;
        }
    }
    return true;
}

inline unsigned convertABGRtoRGBA(unsigned pixel) {
    unsigned r = pixel & 0xFF;
    unsigned g = (pixel >> 8) & 0xFF;
    unsigned b = (pixel >> 16) & 0xFF;
    unsigned a = (pixel >> 24) & 0xFF;
    return a | (b << 8) | (g << 16) | (r << 24);
}

/**
 * Encodes a portion of the image in RLE coding
 * @param image The image that we want to encode
 * @param output The output buffer
 * @param channels The number of channels to write
 * @param offset Offset in bytes to copy
 * @param max The maximum number of pixels to write
 * @param oConsumed The number of pixels consumed from image
 * @param oProduced The number of bytes produced in out
 * @return True on success
 */
static bool encodeRLE(const unsigned *image, unsigned char *output, bool rgb, unsigned max, unsigned &oConsumed, unsigned &oProduced) {
    const unsigned *in = image;
    unsigned char *out = output;
    unsigned count = 0;
    unsigned channels = 3;
    unsigned offset = 1;
    unsigned mask = 0x00FFFFFF;
    if (!rgb) {
        channels = 1;
        offset = 0;
        mask = 0xFF000000;
    }
    for (; (*in & mask) == (*image & mask) && count < 65536 && count < max; in++, count++) {
    }
    if (count > 127) {
        /* Sequence of > 127 identical pixels */
        *out++ = 128;
        *out++ = count >> 8;
        *out++ = count & 0xFF;
        unsigned pixel = convertABGRtoRGBA(*image);
        memcpy(out, ((char*) & pixel) + offset, channels);
        out += channels;
        oConsumed = count;
        oProduced = out - output;
    }
    else if (count > 1) {
        /* Sequece of < 128 identical pixels */
        *out++ = (count + 127);
        unsigned pixel = convertABGRtoRGBA(*image);
        memcpy(out, ((char*) & pixel) + offset, channels);
        out += channels;
        oConsumed = count;
        oProduced = out - output;
    }
    else {
        in = image + 1;
        unsigned previous = *image;
        count = 0;
        while ((*in & mask) != (previous & mask) && count < 128 && count < max) {
            previous = *in;
            in++;
            count++;
        }
        // This only happens when it is the end of the row, and it is ok
        if (count == 0) {
            count = 1;
        }
        *out++ = (count - 1);
        in = image;
        for (unsigned c = 0; c < count; ++c) {
            unsigned pixel = convertABGRtoRGBA(*in);
            memcpy(out, ((char*) & pixel) + offset, channels);
            out += channels;
            in++;
        }
        oConsumed = count;
        oProduced = out - output;
    }
    return true;
}

/**
 * Writes a row to the file
 * @return True on success
 */
static bool writeRow(QIODevice *dev, unsigned *row, unsigned width, bool alpha) {
    unsigned char *buf = new unsigned char[width * 4];
    unsigned posIn = 0;
    unsigned posOut = 0;

    memset(buf, 0, width * 4);

    unsigned consumed = 0;
    unsigned produced = 0;

    /* Write the RGB part of the scanline */
    while (posIn < width) {
        if (!encodeRLE(row + posIn, buf + posOut, true, width - posIn, consumed, produced)) {
            delete buf;
            return false;
        }
        posIn += consumed;
        posOut += produced;
    }

    /* Write the alpha channel */
    if (alpha) {
        posIn = 0;
        while (posIn < width) {
            if (!encodeRLE(row + posIn, buf + posOut, false, width - posIn, consumed, produced)) {
                delete buf;
                return false;
            }
            posIn += consumed;
            posOut += produced;
        }
    }

    dev->write((const char*) buf, posOut);
    delete buf;
    return true;
}

#define FAIL() { \
	std::cout << "ERROR Writing PIC!" << std::endl; \
	return; \
}

/// Pic write handler for Qt / KDE

void pic_write(QIODevice *dev, const QImage *img) {
    bool alpha = img->hasAlphaChannel();
    if (!writeHeader(dev, "Created with KDE", img->width(), img->height(), alpha)) {
        FAIL();
    }

    for (int r = 0; r < img->height(); r++) {
        unsigned *row = (unsigned*) img->scanLine(r);
        if (!writeRow(dev, row, img->width(), alpha)) {
            FAIL();
        }
    }
}

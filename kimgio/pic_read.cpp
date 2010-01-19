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
#include <algorithm>

/**
 * Reads the PIC header and checks that it is OK
 * @param dev The QT device to read from
 * @param hdr A pointer to the PIC header
 * @param peek Keep bytes in the device
 * @return true on success
 */
bool picReadHeader(QIODevice *dev, PICHeader *hdr, bool peek) {
    int result = 0;
    if (peek) {
        result = dev->peek((char*) hdr, HEADER_SIZE);
    } else {
        result = dev->read((char*) hdr, HEADER_SIZE);
    }

    hdr->magic = ntohl(hdr->magic);
    hdr->width = ntohs(hdr->width);
    hdr->height = ntohs(hdr->height);
    hdr->fields = ntohs(hdr->fields);

    if (hdr->magic != PIC_MAGIC_NUMBER || strncmp(hdr->id, "PICT", 4)) {
        return false;
    }

    return result == HEADER_SIZE;
}

#define CHANNEL_BYTE(ch, mask) (( ch & mask) ? 1 : 0)

/**
 * Gets the channels definition and returns the number of bytes per pixel
 * @param channels The channels bitfield
 * @return The number of bytes per pixel
 */
static int channels2bpp(char channels) {
    return CHANNEL_BYTE(channels, RED)
            + CHANNEL_BYTE(channels, GREEN)
            + CHANNEL_BYTE(channels, BLUE)
            + CHANNEL_BYTE(channels, ALPHA);
}

/**
 * Reads the channels info
 * @param dev The QT device to read from
 * @param channels A pointer to 8 channels
 * @return true on success
 */
static bool readChannels(QIODevice *dev, PICChannel *channels, int &bpp) {
    int c = 0;
    memset(channels, 0, sizeof ( PICChannel) *8);
    do {
        int result = dev->read((char*) & channels[c], CHANNEL_SIZE);
        if (result != CHANNEL_SIZE) {
            return false;
        } else {
            bpp += channels2bpp(channels[c].channel);
            c++;
        }
    }    while (channels[c - 1].chained);
    return true;
}

/**
 * Makes a component map based on the channels info
 * @param channels The channel information
 * @param cmap The component map to be built
 */
inline static void makeComponentMap(unsigned channel, unsigned char *cmap) {
    std::fill(cmap, cmap + 8, 0);

    unsigned compos[] = {ALPHA, BLUE, GREEN, RED};
    unsigned rgba[] = {3, 2, 1, 0};
    unsigned pos = 0;
    for (unsigned compo = 0; compo < 4; compo++) {
        if (CHANNEL_BYTE(channel, compos[compo])) {
            cmap[pos++] = rgba[compo];
        }
    }
}

/**
 * Converts a PIC pixel to 32bits RGBA
 * @param src_pixel The source PIC pixel as readed from file
 * @param target_pixel The target buffer where to write the pixel info
 * @param cmap The component map that maps each component in PIC format to RGBA format
 * @param components The number of components in the source pixel
 */
inline static void pic2RGBA(unsigned char *src_pixel, unsigned char *target_pixel, unsigned char *cmap, unsigned components) {
    for (unsigned i = 0; i < components; i++) {
        target_pixel[cmap[i]] = src_pixel[i];
    }
}

/**
 * Counts the number of channels in the PICChannel header
 * @param channels The header
 * @return The number of used channels
 */
inline static unsigned getNumChannels(PICChannel *channels) {
    unsigned result = 0;
    for (unsigned i = 0; i < 8; i++) {
        if (channels[i].channel != 0) {
            result++;
        } else {
            return result;
        }
    }
    return result;
}

/**
 * Decodes a Run-lenght encoded chunk
 * @param dev The device to read from
 * @param row The row pointer to write to
 * @param max The maximum length to write
 * @param channels The channels header
 * @return The number of generated pixels
 */
static int decodeRLE(QIODevice *dev, void *row, unsigned max, unsigned bpp, unsigned channels) {
    unsigned char buf[512];
    unsigned *ptr = (unsigned *) row;
    unsigned char component_map[8];
    unsigned len = 0;

    makeComponentMap(channels, component_map);

    if (dev->read((char*) buf, 1) != 1) {
        return -1;
    }

    /* If last bit is 1, then it is 2 to 127 repetitions */
    if (buf[0] > 128) {
        len = buf[0] - 127;
        if (len > max) {
            return -1;
        }
        unsigned count = dev->read((char*) buf, bpp);
        if (count != bpp) {
            return -1;
        }
        for (unsigned i = 0; i < len; i++) {
            pic2RGBA(buf, (unsigned char*) (ptr + i), component_map, bpp);
        }
    }        /* If the value is exactly 10000000, it means that it is more than 127 repetitions */
    else if (buf[0] == 128) {
        unsigned count = dev->read((char*) buf, bpp + 2);
        if (count != bpp + 2) {
            return -1;
        }
        len = (buf[0] << 8) | buf[1];
        if (len > max) {
            return -1;
        }
        for (unsigned i = 0; i < len; i++) {
            pic2RGBA(buf + 2, (unsigned char*) (ptr + i), component_map, bpp);
        }
    }        /** No repetitions */
    else {
        len = buf[0] + 1;
        if (len > max) {
            return -1;
        }
        unsigned count = dev->read((char*) buf, len * bpp);
        if (count != len * bpp) {
            return -1;
        }
        for (unsigned i = 0; i < len; i++) {
            pic2RGBA(buf + (i * bpp), (unsigned char*) (ptr + i), component_map, bpp);
        }
    }
    return len;
}

/**
 * Reads a row from the file
 * @param dev The device to read from
 * @param row The row pointer to write to
 * @param width The image width
 * @param bpp The bytes per pixel
 * @param channels The channels header info
 */
static bool readRow(QIODevice *dev, unsigned *row, unsigned width, PICChannel *channels) {
    for (int c = 0; channels[c].channel != 0; c++) {
        unsigned remain = width;
        unsigned bpp = channels2bpp(channels[c].channel);
        if (channels[c].type == (int) RLE) {
            unsigned *rowpos = row;
            while (remain > 0) {
                int readed = decodeRLE(dev, rowpos, remain, bpp, channels[c].channel);
                if (readed < 0) {
                    return false;
                }
                remain -= readed;
                rowpos += readed;
            }
        } else {
            unsigned char component_map[8];
            unsigned count = dev->read((char*) row, width * bpp);
            if (count != width * bpp) {
                return false;
            }

            makeComponentMap(channels[c].channel, component_map);
            for (unsigned i = 0; i < width; i++) {
                pic2RGBA(((unsigned char*) row) + (i * bpp), (unsigned char*) (row + i), component_map, bpp);
            }
        }
    }
    return true;
}

#define FAIL() { \
    std::cout << "ERROR Reading PIC!" << std::endl; \
    return; \
}

bool hasAlpha(PICChannel *channels) {
    int channel = 0;
    do {
        if (CHANNEL_BYTE(channels[channel].channel, ALPHA)) {
            return true;
        }
        channel++;
    }    while (channels[channel - 1].chained);
    return false;
}

/**
 * KDE image reading function. Must have this exact name in order to work
 */
void pic_read(QIODevice *dev, QImage *result) {
    PICHeader header;
    PICChannel channels[8];
    int bpp = 0;
    if (!picReadHeader(dev, &header) || !readChannels(dev, channels, bpp)) {
        FAIL();
    }
    QImage img(header.width, header.height, QImage::Format_ARGB32);

    for (int r = 0; r < header.height; r++) {
        unsigned *row = (unsigned*) img.scanLine(r);
        std::fill(row, row + header.width, 0);
        if (!readRow(dev, row, header.width, channels)) {
            FAIL();
        }
    }
    //  img->setAlphaBuffer(hasAlpha(channels));
    *result = img;
}

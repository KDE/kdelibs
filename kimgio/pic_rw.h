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

#ifndef __PIC_RW_H__
#define __PIC_RW_H__

#define PIC_MAGIC_NUMBER  0x5380f634

#include <QtGui/QImageIOPlugin>
#include <qfile.h>
#include <Qt/qcolor.h>
#include <kdebug.h>

/**
 * How fields are distributed over the image
 */
typedef enum {
    NONE = 0, /* No picture */
    ODD = 1, /* Odd scanlines */
    EVEN = 2, /* Even scanlines */
    BOTH = 3 /* Every scanline */
} PICFields;

/**
 * Type of a channel
 */
typedef enum {
    UNCOMPRESSED = 0, /* Image is uncompressed */
    RLE = 2 /* Run length compression */
} PICChannelType;

/**
 * Channel codes
 */
typedef enum {
    RED = 0x80, /* Red channel */
    GREEN = 0x40, /* Green channel */
    BLUE = 0x20, /* Blue channel */
    ALPHA = 0x10 /* Alpha channel */
} PICChannelCode;

/**
 * PIC format header
 */
typedef struct {
    qint32 magic; /* PIC_MAGIC_NUMBER */
    float version; /* Version of format */
    char comment[80]; /* Prototype description */
    char id[4]; /* "PICT" */
    qint16 width; /* Image width, in pixels */
    qint16 height; /* Image height, in pixels */
    float ratio; /* Pixel aspect ratio */
    qint16 fields; /* Picture field type */
    qint16 pad; /* Unused */
} PICHeader;

/**
 * PIC channel header
 */
typedef struct {
    char chained; /* 1 if another packet follows, else 0 */
    char size; /* Bits per pixel by channel */
    char type; /* RLE or uncompressed */
    char channel; /* Channel code (which planes are affected by this channel) */
} PICChannel;

#define HEADER_SIZE sizeof(PICHeader)
#define CHANNEL_SIZE sizeof(PICChannel)


/**
 * Reads the PIC header and checks that it is OK
 * @param dev The QT device to read from
 * @param hdr A pointer to the PIC header
 * @param peek Keep bytes in the device
 * @return true on success
 */
bool picReadHeader(QIODevice *dev, PICHeader *hdr, bool peek = false);

/// Pic read handler for Qt / KDE
void pic_read(QIODevice *dev, QImage *img);

/// Pic write handler for Qt / KDE
void pic_write(QIODevice *dev, const QImage *img);


#endif//__PIC_RW_H__

/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Lubos Lunak (l.lunak@kde.org)
    Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kxutils_p.h"

#if HAVE_X11
#include <qbitmap.h>
#include <QX11Info>
#include <QDebug>

#include <xcb/xcb.h>

namespace KXUtils
{

static uint8_t defaultDepth()
{
    xcb_connection_t *c = QX11Info::connection();
    int screen = QX11Info::appScreen();

    xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(c));
    for (; it.rem; --screen, xcb_screen_next(&it)) {
        if (screen == 0) {
            return it.data->root_depth;
        }
    }
    return 0;
}

static QImage::Format findFormat()
{
    xcb_connection_t *c = QX11Info::connection();
    int screen = QX11Info::appScreen();

    xcb_screen_iterator_t screenIt = xcb_setup_roots_iterator(xcb_get_setup(c));
    for (; screenIt.rem; --screen, xcb_screen_next(&screenIt)) {
        if (screen != 0) {
            continue;
        }
        xcb_depth_iterator_t depthIt = xcb_screen_allowed_depths_iterator(screenIt.data);
        for (; depthIt.rem; xcb_depth_next(&depthIt)) {
            xcb_visualtype_iterator_t visualIt = xcb_depth_visuals_iterator(depthIt.data);
            for (; visualIt.rem; xcb_visualtype_next(&visualIt)) {
                if (screenIt.data->root_visual != visualIt.data->visual_id) {
                    continue;
                }
                xcb_visualtype_t *visual = visualIt.data;
                if ((depthIt.data->depth == 24 || depthIt.data->depth == 32) &&
                        visual->red_mask   == 0x00ff0000 &&
                        visual->green_mask == 0x0000ff00 &&
                        visual->blue_mask  == 0x000000ff) {
                    return QImage::Format_ARGB32_Premultiplied;
                }
                if (depthIt.data->depth == 16 &&
                        visual->red_mask   == 0xf800 &&
                        visual->green_mask == 0x07e0 &&
                        visual->blue_mask  == 0x001f) {
                    return QImage::Format_RGB16;
                }
                break;
            }
        }
    }
    return QImage::Format_Invalid;
}

template <typename T> T fromNative(xcb_pixmap_t pixmap)
{
    xcb_connection_t *c = QX11Info::connection();

    const xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry_unchecked(c,  pixmap);
    ScopedCPointer<xcb_get_geometry_reply_t> geo(xcb_get_geometry_reply(c, geoCookie, Q_NULLPTR));
    if (geo.isNull()) {
        // getting geometry for the pixmap failed
        return T();
    }

    const xcb_get_image_cookie_t imageCookie = xcb_get_image_unchecked(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap,
                                                                       0, 0, geo->width, geo->height, ~0);
    ScopedCPointer<xcb_get_image_reply_t> xImage(xcb_get_image_reply(c, imageCookie, Q_NULLPTR));
    if (xImage.isNull()) {
        // request for image data failed
        return T();
    }
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    switch (xImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 30: {
        // Qt doesn't have a matching image format. We need to convert manually
        uint32_t *pixels = reinterpret_cast<uint32_t *>(xcb_get_image_data(xImage.data()));
        for (int i = 0; i < xImage.data()->length; ++i) {
            int r = (pixels[i] >> 22) & 0xff;
            int g = (pixels[i] >> 12) & 0xff;
            int b = (pixels[i] >>  2) & 0xff;

            pixels[i] = qRgba(r, g, b, 0xff);
        }
        QImage image(reinterpret_cast<uchar*>(pixels), geo->width, geo->height,
                     xcb_get_image_data_length(xImage.data())/geo->height, QImage::Format_ARGB32_Premultiplied);
        if (image.isNull()) {
            return T();
        }
        return T::fromImage(image);
    }
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        if (xImage->depth == defaultDepth()) {
            format = findFormat();
            if (format == QImage::Format_Invalid) {
                return T();
            }
        } else {
            // we don't know
            return T();
        }
    }
    QImage image(xcb_get_image_data(xImage.data()), geo->width, geo->height,
                 xcb_get_image_data_length(xImage.data())/geo->height, format);
    if (image.isNull()) {
        return T();
    }
    if (image.format() == QImage::Format_MonoLSB) {
        // work around an abort in QImage::color
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }
    return T::fromImage(image);
}

// Create QPixmap from X pixmap. Take care of different depths if needed.
QPixmap createPixmapFromHandle( WId pixmap, WId pixmap_mask )
{
    xcb_connection_t *c = QX11Info::connection();

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    qDebug() << "Byte order not supported";
    return QPixmap();
#endif
    const xcb_setup_t *setup = xcb_get_setup(c);
    if (setup->image_byte_order != XCB_IMAGE_ORDER_LSB_FIRST) {
        qDebug() << "Byte order not supported";
        return QPixmap();
    }

    QPixmap pix = fromNative<QPixmap>(pixmap);
    if (pixmap_mask != XCB_PIXMAP_NONE) {
        QBitmap mask = fromNative<QBitmap>(pixmap_mask);
        if (mask.size() != pix.size()) {
            return QPixmap();
        }
        pix.setMask(mask);
    }
    return pix;
}

// Functions for X timestamp comparing. For Time being 32bit they're fairly simple
// (the #if 0 part), but on 64bit architectures Time is 64bit unsigned long,
// so there special care needs to be taken to always use only the lower 32bits.
#if 0
int timestampCompare( Time time1, Time time2 ) // like strcmp()
    {
    if( time1 == time2 )
        return 0;
    return ( time1 - time2 ) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
    }

Time timestampDiff( Time time1, Time time2 ) // returns time2 - time1
    { // no need to handle wrapping?
    return time2 - time1;
    }
#else
int timestampCompare( unsigned long time1_, unsigned long time2_ ) // like strcmp()
    {
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    if( time1 == time2 )
        return 0;
    return quint32( time1 - time2 ) < 0x7fffffffU ? 1 : -1; // time1 > time2 -> 1, handle wrapping
    }

int timestampDiff( unsigned long time1_, unsigned long time2_ ) // returns time2 - time1
    { // no need to handle wrapping?
    quint32 time1 = time1_;
    quint32 time2 = time2_;
    return quint32( time2 - time1 );
    }
#endif


} // namespace

#endif

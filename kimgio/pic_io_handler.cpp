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

#include "pic_io_handler.h"
#include "pic_rw.h"

bool SoftimagePICHandler::canRead() const {
    if (!SoftimagePICHandler::canRead(device())) {
        return false;
    }
    setFormat("pic");
    return true;
}

bool SoftimagePICHandler::read(QImage *image) {
    pic_read(device(), image);
    return true;
}

bool SoftimagePICHandler::write(const QImage &image) {
    pic_write(device(), &image);
    return true;
}

bool SoftimagePICHandler::canRead(QIODevice *device) {
    PICHeader hdr;
    if (picReadHeader(device, &hdr, true)) {
        if (strncmp(hdr.id, "PICT", 4) == 0) {
            return true;
        }
    }
    return false;
}

QVariant SoftimagePICHandler::option(ImageOption option) const {
    if (option == Size) {
        PICHeader hdr;
        if (picReadHeader(device(), &hdr, true)) {
            return QSize(hdr.width, hdr.height);
        } else {
            return QSize(-1, -1);
        }
    }
    return QVariant();
}

bool SoftimagePICHandler::supportsOption(ImageOption option) const {
    return ( option == Size);
}

QByteArray SoftimagePICHandler::name() const {
    return "pic";
}


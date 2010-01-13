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

#ifndef __PIC_IO_HANDLER_H__
#define __PIC_IO_HANDLER_H__

#include "pic_rw.h"
#include <qimage.h>
#include <Qt/qvariant.h>

#include <iostream>

class SoftimagePICHandler : public QImageIOHandler {
public:
    virtual bool canRead() const;
    virtual bool read(QImage * image);
    virtual bool write(const QImage &);
    static bool canRead(QIODevice *device);
    QByteArray name() const;

    QVariant option(ImageOption option) const;
    bool supportsOption(ImageOption option) const;
};

#endif//__PIC_IO_HANDLER_H__

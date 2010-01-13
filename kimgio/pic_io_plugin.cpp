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

#include "pic_io_plugin.h"
#include "pic_io_handler.h"

QImageIOPlugin::Capabilities SoftimagePICPlugin::capabilities(QIODevice *device, const QByteArray &format) const {
    if (format == "pic") {
        return Capabilities(CanRead | CanWrite);
    }
    if (!(format.isEmpty() && device->isOpen())) {
        return 0;
    }

    Capabilities cap;
    if (device->isReadable() && SoftimagePICHandler::canRead(device)) {
        cap |= CanRead;
    }
    if (device->isWritable()) {
        cap |= CanWrite;
    }
    return cap;
}

QStringList SoftimagePICPlugin::keys() const {
    return QStringList() << "pic";
}

QImageIOHandler * SoftimagePICPlugin::create(QIODevice *device, const QByteArray &format) const {
    QImageIOHandler * handler = new SoftimagePICHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(SoftimagePICPlugin)
Q_EXPORT_PLUGIN2(softimagePICPlugin, SoftimagePICPlugin)


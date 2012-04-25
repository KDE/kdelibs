/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fusestoragevolume.h"
#include "fusedevice.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>

namespace Solid
{
namespace Backends
{
namespace Fuse
{

FuseStorageVolume::FuseStorageVolume(FuseDevice * device)
    : QObject(), m_device(device)
{
}

FuseStorageVolume::~FuseStorageVolume()
{
}

bool FuseStorageVolume::isIgnored() const
{
    return false;
}

QString FuseStorageVolume::encryptedContainerUdi() const
{
    return QString();
}

QString FuseStorageVolume::fsType() const
{
    return QString::fromLatin1("fuse");
}

QString FuseStorageVolume::label() const
{
    return QString();
}

QString FuseStorageVolume::uuid() const
{
    if (!m_device) {
        qDebug() << "DEVICE IS NULL!?";
        return QString();
    }
    // We are using the mount path to generate the UUID
    // it would be better if we could get the source URI, but
    // fuse doesn't save that in mtab
    //
    // UUID will be version 3 - MD5 based instead of SHA1 (v 5)

    QByteArray hash = QCryptographicHash::hash(
            m_device->udi().toUtf8(),
            QCryptographicHash::Md5);

    // "90015098-3cd2-4fb0-d696-3f7d28e17f72"
    // "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    // {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
    //           8    12   16

    QString result(hash.toHex().insert(20, '-').insert(16, '-').insert(12, '-').insert(8, '-'));

    qDebug() << "UUID is " << result;

    return result;
}

qulonglong FuseStorageVolume::size() const
{
    return 1024;
}

Solid::StorageVolume::UsageType FuseStorageVolume::usage() const
{
    return Solid::StorageVolume::FileSystem;
}


}
}
}

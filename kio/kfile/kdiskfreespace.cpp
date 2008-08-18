/*
 * kdiskfreespace.cpp
 *
 * Copyright 2007 David Faure <faure@kde.org>
 * Copyright 2008 Dirk Mueller <mueller@kde.org>
 * Copyright 2008 Sebastian Trug <trueg@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdiskfreespace.h"
#include "kdiskfreespaceinfo.h"
#include <QtCore/QTimer>

#include <kdebug.h>


class KDiskFreeSpace::Private
{
public:
    Private(KDiskFreeSpace *parent)
        : m_parent(parent)
    {}

    bool _k_calculateFreeSpace();

    KDiskFreeSpace   *m_parent;
    QString           m_path;
};


KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent), d(new Private(this))
{
}


KDiskFreeSpace::~KDiskFreeSpace()
{
    delete d;
}


bool KDiskFreeSpace::readDF( const QString & mountPoint )
{
    d->m_path = mountPoint;
    return d->_k_calculateFreeSpace();
}


bool KDiskFreeSpace::Private::_k_calculateFreeSpace()
{
    KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo( m_path );
    if ( info.isValid() ) {
        quint64 sizeKiB = info.size() / 1024;
        quint64 availKiB = info.available() / 1024;
        emit m_parent->foundMountPoint( info.mountPoint(), sizeKiB, sizeKiB-availKiB, availKiB );
    }

    emit m_parent->done();

    m_parent->deleteLater();

    return info.isValid();
}

KDiskFreeSpace * KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    job->d->m_path = path;
    QTimer::singleShot(0, job, SLOT(_k_calculateFreeSpace()));
    return job;
}

#include "kdiskfreespace.moc"


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
#include <QtCore/QFile>
#include <QtCore/QTextIStream>
#include <QtCore/QTimer>

#include <kdebug.h>
#include <kprocess.h>
#include <kmountpoint.h>
#include <kio/global.h>
#include <config-kfile.h>

class KDiskFreeSpace::Private
{
public:
    Private(KDiskFreeSpace *parent)
        : m_parent(parent),
          total(0),
          avail(0),
          error(-1)
    {}

    bool _k_calculateFreeSpace();

    KDiskFreeSpace   *m_parent;
    QString           m_path;

    QString mountPoint;
    KIO::filesize_t total;
    KIO::filesize_t avail;
    int error;
};

KDiskFreeSpace::KDiskFreeSpace(QObject *parent)
    : QObject(parent), d(new Private(this))
{
}


KDiskFreeSpace::~KDiskFreeSpace()
{
    delete d;
}


QString KDiskFreeSpace::mountPoint() const
{
    return d->mountPoint;
}


KIO::filesize_t KDiskFreeSpace::size() const
{
    return d->total;
}


KIO::filesize_t KDiskFreeSpace::used() const
{
    return d->total - d->avail;
}


KIO::filesize_t KDiskFreeSpace::avail() const
{
    return d->avail;
}


int KDiskFreeSpace::error() const
{
    return d->error;
}


bool KDiskFreeSpace::readDF( const QString & mountPoint )
{
    d->m_path = mountPoint;
    return d->_k_calculateFreeSpace();
}

#ifdef Q_OS_WIN
#include <QtCore/QDir>
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif

bool KDiskFreeSpace::Private::_k_calculateFreeSpace()
{
    // determine the mount point
    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath( m_path );
    if (mp)
        mountPoint = mp->mountPoint();

    error = -1;

#ifdef Q_OS_WIN
    quint64 availUser;
    QFileInfo fi(mountPoint);
    QString dir = QDir::toNativeSeparators(fi.absoluteDir().canonicalPath());

    if(GetDiskFreeSpaceExW((LPCWSTR)dir.utf16(),
                           (PULARGE_INTEGER)&availUser,
                           (PULARGE_INTEGER)&total,
                           (PULARGE_INTEGER)&avail) != 0) {
        error = 0;
    }
#else
    struct statvfs statvfs_buf;

    if (!statvfs(QFile::encodeName(m_path).constData(), &statvfs_buf)) {
        avail = statvfs_buf.f_bavail * statvfs_buf.f_frsize;
        total = statvfs_buf.f_blocks * statvfs_buf.f_frsize;
        error = 0;
    }
#endif

    if (!error) {
        quint64 totalKib = total / 1024;
        quint64 availKib = avail / 1024;
        emit m_parent->foundMountPoint( mountPoint, totalKib, totalKib-availKib, availKib );
    }

    emit m_parent->done();
    emit m_parent->finished( m_parent );
    m_parent->deleteLater();

    return !error;
}

KDiskFreeSpace * KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    job->d->m_path = path;
    QTimer::singleShot(0, job, SLOT(_k_calculateFreeSpace()));
    return job;
}

#include "kdiskfreespace.moc"


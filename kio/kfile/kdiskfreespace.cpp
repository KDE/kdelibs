/*
 * kdiskfreespace.cpp
 *
 * Copyright 2007 David Faure <faure@kde.org>
 * Copyright 2008 Dirk Mueller <mueller@kde.org>
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

struct KDiskFreeSpace::Private
{
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

#ifdef Q_OS_WIN
#include <QtCore/QDir>
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif

bool KDiskFreeSpace::Private::_k_calculateFreeSpace()
{
    // determine the mount point
    QString mountPoint;

    KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath( m_path );
    if (mp)
        mountPoint = mp->mountPoint();

    quint64 availUser, total, avail;
    bool bRet = false;
#ifdef Q_OS_WIN
    QFileInfo fi(mountPoint);
    QString dir = QDir::toNativeSeparators(fi.absoluteDir().canonicalPath());

    if(GetDiskFreeSpaceExW((LPCWSTR)dir.utf16(),
                           (PULARGE_INTEGER)&availUser,
                           (PULARGE_INTEGER)&total,
                           (PULARGE_INTEGER)&avail) != 0) {
        availUser = availUser / 1024;
        total = total / 1024;
        avail = avail / 1024;
        emit foundMountPoint( mountPoint, total, total-avail, avail );
        bRet = true;
    }
#else
    struct statvfs statvfs_buf;

    if (!statvfs(QFile::encodeName(m_path).constData(), &statvfs_buf)) {
        avail = statvfs_buf.f_bavail * statvfs_buf.f_frsize / 1024;
        total = statvfs_buf.f_blocks * statvfs_buf.f_frsize / 1024;
        emit m_parent->foundMountPoint( mountPoint, total, total-avail, avail );
        bRet = true;
    }
#endif

    emit m_parent->done();
    m_parent->deleteLater();

    return bRet;
}

KDiskFreeSpace * KDiskFreeSpace::findUsageInfo( const QString & path )
{
    KDiskFreeSpace * job = new KDiskFreeSpace;
    job->d->m_path = path;
    QTimer::singleShot(0, job, SLOT(_k_calculateFreeSpace()));
    return job;
}

#include "kdiskfreespace.moc"


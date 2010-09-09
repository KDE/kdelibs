/*
    This file is part of the KDE project

    Copyright (C) 2010 Mario Bensi <mbensi@ipsquad.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "fstabwatcher.h"
#include "soliddefs_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QStringList>

using namespace Solid::Backends::Fstab;

SOLID_GLOBAL_STATIC(FstabWatcher, globalFstabWatcher)

#define MTAB "/etc/mtab"
#ifdef Q_OS_SOLARIS
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

FstabWatcher::FstabWatcher()
    : m_isRoutineInstalled(false)
    , m_fileSystemWatcher(new QFileSystemWatcher(this))
{
    m_fileSystemWatcher->addPath(MTAB);
    m_fileSystemWatcher->addPath(FSTAB);
    connect(m_fileSystemWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
}

FstabWatcher::~FstabWatcher()
{
    qRemovePostRoutine(globalFstabWatcher.destroy);
}

FstabWatcher *FstabWatcher::instance()
{
    FstabWatcher *fstabWatcher = globalFstabWatcher;

    if (fstabWatcher && !fstabWatcher->m_isRoutineInstalled) {
        qAddPostRoutine(globalFstabWatcher.destroy);
        fstabWatcher->m_isRoutineInstalled = true;
    }
    return fstabWatcher;
}


void FstabWatcher::onFileChanged(const QString &path)
{
    if (path == MTAB) {
        emit mtabChanged();
        if (!m_fileSystemWatcher->files().contains(MTAB)) {
            m_fileSystemWatcher->addPath(MTAB);
        }
    }
    if (path == FSTAB) {
        emit fstabChanged();
        if (!m_fileSystemWatcher->files().contains(FSTAB)) {
            m_fileSystemWatcher->addPath(FSTAB);
        }
    }
}



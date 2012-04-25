/*
    Copyright 2006-2010 Kevin Ottens <ervin@kde.org>
    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>
    Copyright 2012 Ivan Cukic <ivan.cukic@kde.org>

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

#include "fusehandling.h"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>
#include <QtCore/QTime>

#include <solid/soliddefs_p.h>

#include <config.h>
#include <stdlib.h>

#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTENT_H)
#include <sys/mntent.h>
#endif

// This is the *BSD branch
#ifdef HAVE_SYS_MOUNT_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#ifndef HAVE_GETMNTINFO
# ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
#  undef MNTTAB
#  define MNTTAB _PATH_MOUNTED
# else
#  ifndef MNTTAB
#   ifdef MTAB_FILE
#    define MNTTAB MTAB_FILE
#   else
#    define MNTTAB "/etc/mnttab"
#   endif
#  endif
# endif
#endif

// There are (at least) four kind of APIs:
// setmntent + getmntent + struct mntent (linux...)
//             getmntent + struct mnttab
// mntctl                + struct vmount (AIX)
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)
// getfsent + char* (BSD 4.3 and friends)

#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define MOUNTOPTIONS(var) var->mnt_opts
#define FSNAME(var) var->mnt_fsname
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define MOUNTOPTIONS(var) var.mnt_mntopts
#define FSNAME(var) var.mnt_special
#endif

#include <QDebug>

SOLID_GLOBAL_STATIC(Solid::Backends::Fuse::FuseHandling, globalFuseCache)

Solid::Backends::Fuse::FuseHandling::FuseHandling()
    : m_mtabCacheValid(false)
{ }

bool _k_isFuseFileSystem(const QString &fstype, const QString &devName)
{
    return fstype.startsWith("fuse.");
}

QStringList Solid::Backends::Fuse::FuseHandling::deviceList()
{
    _k_updateMtabMountPointsCache();

    QStringList devices = globalFuseCache->m_mtabCache.values();
    return devices;
}

QStringList Solid::Backends::Fuse::FuseHandling::mountPoints(const QString &device)
{
    _k_updateMtabMountPointsCache();

    return QStringList() << device;
}

QProcess *Solid::Backends::Fuse::FuseHandling::callSystemCommand(const QString &commandName,
                                                                 const QStringList &args,
                                                                 QObject *obj, const char *slot)
{
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=/sbin:/bin:/usr/sbin/:/usr/bin");

    QProcess *process = new QProcess(obj);

    QObject::connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
                     obj, slot);

    process->setEnvironment(env);
    process->start(commandName, args);

    if (process->waitForStarted()) {
        return process;
    } else {
        delete process;
        return 0;
    }
}

QProcess *Solid::Backends::Fuse::FuseHandling::callSystemCommand(const QString &commandName,
                                                                 const QString &device,
                                                                 QObject *obj, const char *slot)
{
    return callSystemCommand(commandName, QStringList() << device, obj, slot);
}

void Solid::Backends::Fuse::FuseHandling::_k_updateMtabMountPointsCache()
{
    if (globalFuseCache->m_mtabCacheValid)
        return;

    globalFuseCache->m_mtabCache.clear();

    STRUCT_SETMNTENT mnttab;
    if ((mnttab = SETMNTENT(MNTTAB, "r")) == 0)
        return;

    STRUCT_MNTENT fe;
    while (GETMNTENT(mnttab, fe))
    {
        QString type = QFile::decodeName(MOUNTTYPE(fe));
        if (_k_isFuseFileSystem(type, QString())) {
            const QString device = QFile::decodeName(FSNAME(fe));
            const QString mountpoint = QFile::decodeName(MOUNTPOINT(fe));
            globalFuseCache->m_mtabCache << mountpoint;
        }
    }
    ENDMNTENT(mnttab);

    globalFuseCache->m_mtabCacheValid = true;
}

QStringList Solid::Backends::Fuse::FuseHandling::currentMountPoints(const QString &device)
{
    _k_updateMtabMountPointsCache();
    return QStringList() << device;
}

void Solid::Backends::Fuse::FuseHandling::flushMtabCache()
{
    globalFuseCache->m_mtabCacheValid = false;
}


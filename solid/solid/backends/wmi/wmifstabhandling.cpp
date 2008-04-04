/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "wmifstabhandling.h"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QTextStream>

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTENT_H)
#include <sys/mntent.h>
#endif

#ifdef Q_OS_SOLARIS
#define FSTAB "/etc/vfstab"
#else
#define FSTAB "/etc/fstab"
#endif

QString _k_resolveSymLink(const QString &filename)
{
    QString resolved = filename;
    QString tmp = QFile::symLinkTarget(filename);

    while (!tmp.isEmpty()) {
        resolved = tmp;
        tmp = QFile::symLinkTarget(resolved);
    }

    return resolved;
}

bool Solid::Backends::Wmi::FstabHandling::isInFstab(const QString &device)
{
    const QString deviceToFind = _k_resolveSymLink(device);

    if (deviceToFind.isEmpty()) {
        return false;
    }

#ifdef HAVE_SETMNTENT

    struct mntent *fstab;
    if ((fstab = setmntent(FSTAB, "r")) == 0) {
        return false;
    }

    struct mntent *fe;
    while ((fe = getmntent(fstab)) != 0) {
        const QString device = _k_resolveSymLink(QFile::decodeName(fe->mnt_fsname));

        if (device==deviceToFind) {
            endmntent(fstab);
            return true;
        }
    }

    endmntent(fstab);

#else

    QFile fstab(FSTAB);
    if (!fstab.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream stream(&fstab);
    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine().simplified();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // not empty or commented out by '#'
        QStringList items = line.split(' ');

#ifdef Q_OS_SOLARIS
        if (items.count() < 5) {
            continue;
        }
#else
        if (items.count() < 4) {
            continue;
        }
#endif

        const QString device = _k_resolveSymLink(items.first());

        if (device==deviceToFind) {
            fstab.close();
            return true;
        }
   }

   fstab.close();
#endif

   return false;
}

QProcess *Solid::Backends::Wmi::FstabHandling::callSystemCommand(const QString &commandName,
                                                                 const QString &device,
                                                                 QObject *obj, const char *slot)
{
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive), "PATH=/sbin:/bin:/usr/sbin/:/usr/bin");

    QProcess *process = new QProcess(obj);

    QObject::connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
                     obj, slot);

    process->setEnvironment(env);
    process->start(commandName, QStringList() << device);

    if (process->waitForStarted()) {
        return process;
    } else {
        delete process;
        return 0;
    }
}


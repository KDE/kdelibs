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

#ifndef SOLID_BACKENDS_FUSE_FUSEHANDLING_H
#define SOLID_BACKENDS_FUSE_FUSEHANDLING_H

#include <QtCore/QString>
#include <QtCore/QMultiHash>

#include <QString>
#include <QSet>

class QProcess;
class QObject;

namespace Solid
{
namespace Backends
{
namespace Fuse
{

class FuseHandling
{
public:
    FuseHandling();

    static QStringList deviceList();
    static QStringList currentMountPoints(const QString &device);
    static QStringList mountPoints(const QString &device);
    static QProcess *callSystemCommand(const QString &commandName,
                                       const QStringList &args,
                                       QObject *obj, const char *slot);
    static QProcess *callSystemCommand(const QString &commandName,
                                       const QString &device,
                                       QObject *obj, const char *slot);
    static void flushMtabCache();
    static void flushFuseCache();

private:
    static void _k_updateMtabMountPointsCache();
    static void _k_updateFuseMountPointsCache();

    QSet<QString> m_mtabCache;
    bool m_mtabCacheValid;

};

}
}
}

#endif



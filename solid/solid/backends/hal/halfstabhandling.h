/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_HAL_FSTABHANDLING_H
#define SOLID_BACKENDS_HAL_FSTABHANDLING_H

#include <QtCore/QString>

class QProcess;
class QObject;

namespace Solid
{
namespace Backends
{
namespace Hal
{
class FstabHandling
{
public:
    static bool isInFstab(const QString &device);
    static QStringList possibleMountPoints(const QString &device);
    static QProcess *callSystemCommand(const QString &commandName,
                                       const QStringList &args,
                                       QObject *obj, const char *slot);
    static QProcess *callSystemCommand(const QString &commandName,
                                       const QString &device,
                                       QObject *obj, const char *slot);
};
}
}
}

#endif



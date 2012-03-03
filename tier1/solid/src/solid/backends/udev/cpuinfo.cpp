/*
    Copyright 2010 Alex Merry <alex.merry@kdemail.net>

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

#include "cpuinfo.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace UDev
{

QString extractCpuInfoLine(int processorNumber, const QString &regExpStr)
{
    if (processorNumber == -1) {
        return QString();
    }

    QFile cpuInfoFile("/proc/cpuinfo");
    if (!cpuInfoFile.open(QIODevice::ReadOnly)) {
        return QString();
    }
    QStringList cpuInfo = QString(cpuInfoFile.readAll()).split('\n', QString::SkipEmptyParts);
    cpuInfoFile.close();

    const QRegExp processorRegExp("processor\\s+:\\s+(\\d+)");
    const QRegExp regExp(regExpStr);

    int line = 0;
    while (line < cpuInfo.size()) {
        if (processorRegExp.exactMatch(cpuInfo.at(line))) {
            int recordProcNum = processorRegExp.capturedTexts()[1].toInt();
            if (recordProcNum == processorNumber) {
                ++line;
                while (line < cpuInfo.size()) {
                    if (regExp.exactMatch(cpuInfo.at(line))) {
                        return regExp.capturedTexts()[1];
                    }
                    ++line;
                }
            }
        }
        ++line;
    }

    return QString();
}

}
}
}

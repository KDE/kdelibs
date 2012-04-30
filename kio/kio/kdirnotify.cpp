/* This file is part of the KDE libraries

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdirnotify.h"

/*
 * Implementation of interface class OrgKdeKDirNotifyInterface
 */

OrgKdeKDirNotifyInterface::OrgKdeKDirNotifyInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgKdeKDirNotifyInterface::~OrgKdeKDirNotifyInterface()
{
}

static void emitSignal(const QString &signalName, const QVariantList &args)
{
    QDBusMessage message =
        QDBusMessage::createSignal("/", QLatin1String(org::kde::KDirNotify::staticInterfaceName()), signalName);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

void OrgKdeKDirNotifyInterface::emitFileRenamed(const QString &src, const QString &dst)
{
    emitSignal(QLatin1String("FileRenamed"), QVariantList() << src << dst);
}

void OrgKdeKDirNotifyInterface::emitFileMoved(const QString &src, const QString &dst)
{
    emitSignal(QLatin1String("FileMoved"), QVariantList() << src << dst);
}

void OrgKdeKDirNotifyInterface::emitFilesAdded(const QString &directory)
{
    emitSignal(QLatin1String("FilesAdded"), QVariantList() << directory);
}

static QStringList urlListToStringList(const QList<QUrl> &urls)
{
    QStringList lst;
    for(QList<QUrl>::const_iterator it = urls.constBegin();
        it != urls.constEnd(); ++it) {
        lst.append(it->toString());
    }
    return lst;
}

void OrgKdeKDirNotifyInterface::emitFilesChanged(const QList<QUrl> &fileList)
{
    emitSignal(QLatin1String("FilesChanged"), QVariantList() << QVariant(urlListToStringList(fileList)));
}

void OrgKdeKDirNotifyInterface::emitFilesRemoved(const QList<QUrl> &fileList)
{
    emitSignal(QLatin1String("FilesRemoved"), QVariantList() << QVariant(urlListToStringList(fileList)));
}

void OrgKdeKDirNotifyInterface::emitEnteredDirectory(const QString &url)
{
    emitSignal(QLatin1String("enteredDirectory"), QVariantList() << url);
}

void OrgKdeKDirNotifyInterface::emitLeftDirectory(const QString &url)
{
    emitSignal(QLatin1String("leftDirectory"), QVariantList() << url);
}


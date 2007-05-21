/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_PLUGINFACTORY_H
#define PHONON_PLUGINFACTORY_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

class QUrl;
class QObject;
class QIcon;

namespace Phonon
{
class AbstractMediaStream;

class PluginFactory
{
    public:
        virtual ~PluginFactory() {}

        virtual AbstractMediaStream *createKioMediaStream(const QUrl &url, QObject *parent) = 0;
        virtual QIcon icon(const QString &name) = 0;
        virtual void notification(const char *notificationName, const QString &text,
                const QStringList &actions, QObject *receiver,
                const char *actionSlot) = 0;
        virtual QString applicationName() const = 0;
        virtual QObject *createBackend() = 0;
        virtual QObject *createBackend(const QString &library, const QString &version) = 0;
        virtual bool isMimeTypeAvailable(const QString &mimeType) = 0;
};

} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::PluginFactory, "PluginFactory1.phonon.kde.org")

#endif // PHONON_PLUGINFACTORY_H

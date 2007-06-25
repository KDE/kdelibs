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

#include "backends/hal/halstorageaccess.h"
#include "halstorageaccess.h"

#include <QtDBus/QDBusConnection>

StorageAccess::StorageAccess(HalDevice *device)
    : DeviceInterface(device), m_setupInProgress(false), m_teardownInProgress(false)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
}

StorageAccess::~StorageAccess()
{

}


bool StorageAccess::isAccessible() const
{
    return m_device->property("volume.is_mounted").toBool();
}

QString StorageAccess::filePath() const
{
    return m_device->property("volume.mount_point").toString();
}


bool StorageAccess::setup()
{
    if (m_teardownInProgress || m_setupInProgress) {
        return false;
    }
    m_setupInProgress = true;


    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      "org.freedesktop.Hal.Device.Volume",
                                                      "Mount");

    msg << "" << "" << QStringList();

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

bool StorageAccess::teardown()
{
    if (m_teardownInProgress || m_setupInProgress) {
        return false;
    }
    m_teardownInProgress = true;


    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      "org.freedesktop.Hal.Device.Volume",
                                                      "Unmount");

    msg << QStringList();

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

void StorageAccess::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("volume.is_mounted"))
    {
        emit accessibilityChanged(isAccessible());
    }
}

void StorageAccess::slotDBusReply(const QDBusMessage &/*reply*/)
{
    if (m_setupInProgress) {
        m_setupInProgress = false;
        emit setupDone(Solid::NoError, QVariant());
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        emit teardownDone(Solid::NoError, QVariant());
    }
}

void StorageAccess::slotDBusError(const QDBusError &error)
{
    // TODO: Better error reporting here
    if (m_setupInProgress) {
        m_setupInProgress = false;
        emit setupDone(Solid::UnauthorizedOperation,
                       error.name()+": "+error.message());
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        emit teardownDone(Solid::UnauthorizedOperation,
                          error.name()+": "+error.message());
    }
}

#include "backends/hal/halstorageaccess.moc"

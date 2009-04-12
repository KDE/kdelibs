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

#include "wmistorageaccess.h"

#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include <unistd.h>

using namespace Solid::Backends::Wmi;

StorageAccess::StorageAccess(WmiDevice *device)
    : DeviceInterface(device), m_setupInProgress(false), m_teardownInProgress(false),
      m_passphraseRequested(false)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
}

StorageAccess::~StorageAccess()
{

}


bool StorageAccess::isAccessible() const
{
    // if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume.Crypto")) {

        //Might be a bit slow, but I see no cleaner way to do this with WMI...
        // QDBusInterface manager("org.freedesktop.Wmi",
                               // "/org/freedesktop/Wmi/Manager",
                               // "org.freedesktop.Wmi.Manager",
                               // QDBusConnection::systemBus());

        // QDBusReply<QStringList> reply = manager.call("FindDeviceStringMatch",
                                                     // "volume.crypto_luks.clear.backing_volume",
                                                     // m_device->udi());

        // QStringList list = reply;

        // return reply.isValid() && !list.isEmpty();

    // } else {
        return m_device->property("volume.is_mounted").toBool();
    // }
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


    // if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume.Crypto")) {
        // return requestPassphrase();
    // } else if (FstabHandling::isInFstab(m_device->property("block.device").toString())) {
        // return callSystemMount();
    // } else {
        // return callWmiVolumeMount();
    // }
    return false;
}

bool StorageAccess::teardown()
{
    if (m_teardownInProgress || m_setupInProgress) {
        return false;
    }
    m_teardownInProgress = true;

    // if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume.Crypto")) {
        // return callCryptoTeardown();
    // } else if (FstabHandling::isInFstab(m_device->property("block.device").toString())) {
        // return callSystemUnmount();
    // } else {
        // return callWmiVolumeUnmount();
    // }
    return false;
}

void StorageAccess::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("volume.is_mounted"))
    {
        emit accessibilityChanged(isAccessible(), m_device->udi());
    }
}

void Solid::Backends::Wmi::StorageAccess::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
/*    
    Q_UNUSED(exitStatus);
    if (m_setupInProgress) {
        m_setupInProgress = false;

        if (exitCode==0) {
            emit setupDone(Solid::NoError, QVariant(), m_device->udi());
        } else {
            emit setupDone(Solid::UnauthorizedOperation,
                           m_process->readAllStandardError(),
                           m_device->udi());
        }
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        if (exitCode==0) {
            emit teardownDone(Solid::NoError, QVariant(), m_device->udi());
        } else {
            emit teardownDone(Solid::UnauthorizedOperation,
                              m_process->readAllStandardError(),
                              m_device->udi());
        }
    }

    delete m_process;
 */
}

QString generateReturnObjectPath()
{
    static int number = 1;

    return "/org/kde/solid/WmiStorageAccess_"+QString::number(number++);
}

bool StorageAccess::callWmiVolumeMount()
{
    // QDBusConnection c = QDBusConnection::systemBus();
    // QString udi = m_device->udi();
    // QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Wmi", udi,
                                                      // "org.freedesktop.Wmi.Device.Volume",
                                                      // "Mount");
    // QStringList options;
    // QStringList wmiOptions = m_device->property("volume.mount.valid_options").toStringList();

    // if (wmiOptions.contains("uid=")) {
        // options << "uid="+QString::number(::getuid());
    // }

    // msg << "" << "" << options;

    // return c.callWithCallback(msg, this,
                              // SLOT(slotDBusReply(const QDBusMessage &)),
                              // SLOT(slotDBusError(const QDBusError &)));
    return false;
}

bool StorageAccess::callWmiVolumeUnmount()
{
    // QDBusConnection c = QDBusConnection::systemBus();
    // QString udi = m_device->udi();
    // QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Wmi", udi,
                                                      // "org.freedesktop.Wmi.Device.Volume",
                                                      // "Unmount");

    // msg << QStringList();

    // return c.callWithCallback(msg, this,
                              // SLOT(slotDBusReply(const QDBusMessage &)),
                              // SLOT(slotDBusError(const QDBusError &)));
    return false;
}

bool Solid::Backends::Wmi::StorageAccess::callSystemMount()
{
/*
    const QString device = m_device->property("block.device").toString();
    m_process = FstabHandling::callSystemCommand("mount", device,
                                                 this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
*/
    return 0;
}

bool Solid::Backends::Wmi::StorageAccess::callSystemUnmount()
{
/*
    const QString device = m_device->property("block.device").toString();
    m_process = FstabHandling::callSystemCommand("umount", device,
                                                 this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
*/
    return 0;
}

#include "backends/wmi/wmistorageaccess.moc"

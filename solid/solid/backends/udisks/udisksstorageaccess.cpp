/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>
    Copyright (C) 2009 Lukas Tinkl <ltinkl@redhat.com>

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

#include "udisksstorageaccess.h"
#include "udisks.h"

#include <QtCore/QProcess>
#include <QtDBus/QtDBus>
#include <QtGui/QApplication>
#include <QtGui/QWidget>

using namespace Solid::Backends::UDisks;

UDisksStorageAccess::UDisksStorageAccess(UDisksDevice *device)
    : DeviceInterface(device), m_setupInProgress(false), m_teardownInProgress(false), m_passphraseRequested(false)
{
    connect(device, SIGNAL(changed()), this, SLOT(slotChanged()));

    updateCache();
}

UDisksStorageAccess::~UDisksStorageAccess()
{
}

bool UDisksStorageAccess::isAccessible() const
{
    return m_device->property("DeviceIsMounted").toBool();
}

QString UDisksStorageAccess::filePath() const
{
    if (!isAccessible())
        return QString();

    return m_device->property("DeviceMountPaths").toStringList().first(); // FIXME Solid doesn't support multiple mount points
}

bool UDisksStorageAccess::setup()
{
    if ( m_teardownInProgress || m_setupInProgress )
        return false;
    m_setupInProgress = true;

    if (m_device->property("DeviceIsLuks").toBool())
        return requestPassphrase();
    else
        return mount();
}

bool UDisksStorageAccess::teardown()
{
    if ( m_teardownInProgress || m_setupInProgress )
        return false;
    m_teardownInProgress = true;

    if (m_device->property("DeviceIsLuks").toBool())
        return callCryptoTeardown();
    else
        return unmount();
}

void UDisksStorageAccess::slotChanged()
{
    const bool old_isAccessible = m_isAccessible;
    updateCache();

    if (old_isAccessible != m_isAccessible)
    {
        emit accessibilityChanged(m_isAccessible, m_device->udi());
    }
}

void UDisksStorageAccess::updateCache()
{
    m_isAccessible = isAccessible();
}

void UDisksStorageAccess::slotDBusReply( const QDBusMessage & reply )
{
    Q_UNUSED(reply);
    if (m_setupInProgress)
    {
        m_setupInProgress = false;
        emit setupDone(Solid::NoError, QVariant(), m_device->udi());
    }
    else if (m_teardownInProgress)
    {
        m_teardownInProgress = false;
        emit teardownDone(Solid::NoError, QVariant(), m_device->udi());

        if ( m_device->property("DriveIsMediaEjectable").toBool() )
        {
            QString devnode = m_device->property("DeviceFile").toString();

#if defined(Q_OS_OPENBSD)
            QString program = "cdio";
            QStringList args;
            args << "-f" << devnode << "eject";
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
            devnode.remove("/dev/").replace("([0-9]).", "\\1");
            QString program = "cdcontrol";
            QStringList args;
            args << "-f" << devnode << "eject";
#else
            QString program = "eject";
            QStringList args;
            args << devnode;
#endif

            QProcess::startDetached( program, args );
        }
    }
}

void UDisksStorageAccess::slotDBusError( const QDBusError & error )
{
    // TODO: Better error reporting here

    // TODO support policykit-1; error: org.freedesktop.PolicyKit.Error.NotAuthorized
    // actions: org.freedesktop.udisks.filesystem-mount: If the operation is on a non-system-internal device
    //          org.freedesktop.udisks.filesystem-mount-system-internal: If the operation is on a system-internal device
    //          org.freedesktop.udisks.filesystem-unmount-others: To unmount a device mounted by another user

    if (m_setupInProgress)
    {
        m_setupInProgress = false;
        emit setupDone(Solid::UnauthorizedOperation, error.name()+": "+error.message(), m_device->udi());
    }
    else if (m_teardownInProgress)
    {
        m_teardownInProgress = false;
        emit teardownDone(Solid::UnauthorizedOperation, error.name()+": "+error.message(), m_device->udi());
    }
}

bool UDisksStorageAccess::mount()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "FilesystemMount");
    QString fstype;

    if (m_device->property("IdUsage").toString() == "filesystem")
        fstype = m_device->property("IdType").toString();

    msg << fstype;
    msg << QStringList();   // options, unused now

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

bool UDisksStorageAccess::unmount()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "FilesystemUnmount");
    QString fstype;

    msg << QStringList();   // options, unused now

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

QString UDisksStorageAccess::generateReturnObjectPath()
{
    static int number = 1;

    return "/org/kde/solid/UDisksStorageAccess_"+QString::number(number++);
}

bool UDisksStorageAccess::requestPassphrase()
{
    QString udi = m_device->udi();
    QString returnService = QDBusConnection::sessionBus().baseService();
    m_lastReturnObject = generateReturnObjectPath();

    QDBusConnection::sessionBus().registerObject(m_lastReturnObject, this, QDBusConnection::ExportScriptableSlots);

    QWidget *activeWindow = QApplication::activeWindow();
    uint wId = 0;
    if (activeWindow!=0)
        wId = (uint)activeWindow->winId();

    QString appId = QCoreApplication::applicationName();

    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showPassphraseDialog", udi, returnService,
                                                m_lastReturnObject, wId, appId);
    m_passphraseRequested = reply.isValid();
    if (!m_passphraseRequested)
        qWarning() << "Failed to call the SolidUiServer, D-Bus said:" << reply.error();

    return m_passphraseRequested;
}

void UDisksStorageAccess::passphraseReply( const QString & passphrase )
{
    if (m_passphraseRequested)
    {
        QDBusConnection::sessionBus().unregisterObject(m_lastReturnObject);
        m_passphraseRequested = false;
        if (!passphrase.isEmpty())
            callCryptoSetup(passphrase);
        else
        {
            m_setupInProgress = false;
            emit setupDone(Solid::NoError, QVariant(), m_device->udi());
        }
    }
}

void UDisksStorageAccess::callCryptoSetup( const QString & passphrase )
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "LuksUnlock");

    msg << passphrase;
    msg << QStringList();   // options, unused now

    c.callWithCallback(msg, this,
                       SLOT(slotDBusReply(const QDBusMessage &)),
                       SLOT(slotDBusError(const QDBusError &)));
}

bool UDisksStorageAccess::callCryptoTeardown()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD_DBUS_SERVICE, m_device->udi(), UD_DBUS_INTERFACE_DISKS_DEVICE, "LuksLock");
    msg << QStringList();   // options, unused now

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}


#include "backends/udisks/udisksstorageaccess.moc"

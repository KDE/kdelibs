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

#include "halstorageaccess.h"

#include "halfstabhandling.h"

#include <QtCore/QLocale>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>
#include <QtGui/QApplication>
#include <QtGui/QWidget>

#include <unistd.h>
#include <stdlib.h>

#ifdef Q_OS_FREEBSD
#include <langinfo.h>
#endif

using namespace Solid::Backends::Hal;

StorageAccess::StorageAccess(HalDevice *device)
    : DeviceInterface(device), m_setupInProgress(false), m_teardownInProgress(false), m_ejectInProgress(false),
      m_passphraseRequested(false)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
    // Delay connecting to DBus signals to avoid the related time penalty
    // in hot paths such as predicate matching
    QTimer::singleShot(0, this, SLOT(connectDBusSignals()));
}

StorageAccess::~StorageAccess()
{

}

void StorageAccess::connectDBusSignals()
{
    m_device->connectActionSignal("setupRequested",  this, SLOT(slotSetupRequested()));
    m_device->connectActionSignal("teardownRequested", this, SLOT(slotTeardownRequested()));
    m_device->connectActionSignal("setupDone",  this, SLOT(slotSetupDone(int, QDBusVariant, const QString&)));
    m_device->connectActionSignal("teardownDone",  this, SLOT(slotTeardownDone(int, QDBusVariant, const QString&)));
    m_device->connectActionSignal("ejectDone",  this, SLOT(slotEjectDone(int, QDBusVariant, const QString&)));

}

void StorageAccess::slotSetupDone(int error, QDBusVariant errorData, const QString& udi)
{
    m_setupInProgress = false;
    emit setupDone(static_cast<Solid::ErrorType>(error), HalDevice::variantFromDBusVariant(errorData), udi);
}

void StorageAccess::slotTeardownDone(int error, QDBusVariant errorData, const QString &udi)
{
    m_teardownInProgress = false;
    emit teardownDone(static_cast<Solid::ErrorType>(error), HalDevice::variantFromDBusVariant(errorData), udi);
}

void StorageAccess::slotEjectDone(int error, QDBusVariant errorData, const QString &udi)
{
    m_ejectInProgress = false;
    emit ejectDone(static_cast<Solid::ErrorType>(error), HalDevice::variantFromDBusVariant(errorData), udi);
}

bool StorageAccess::isAccessible() const
{
    if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Hal.Device.Volume.Crypto")) {

        // Might be a bit slow, but I see no cleaner way to do this with HAL...
        QDBusInterface manager("org.freedesktop.Hal",
                               "/org/freedesktop/Hal/Manager",
                               "org.freedesktop.Hal.Manager",
                               QDBusConnection::systemBus());

        QDBusReply<QStringList> reply = manager.call("FindDeviceStringMatch",
                                                     "volume.crypto_luks.clear.backing_volume",
                                                     m_device->udi());

        QStringList list = reply;

        return reply.isValid() && !list.isEmpty();

    } else {
        return m_device->property("volume.is_mounted").toBool();
    }
}

QString StorageAccess::filePath() const
{
    QString result = m_device->property("volume.mount_point").toString();

    if (result.isEmpty()) {
        QStringList mountpoints
            = FstabHandling::possibleMountPoints(m_device->property("block.device").toString());
        if (mountpoints.size()==1) {
            result = mountpoints.first();
        }
    }

    return result;
}


bool StorageAccess::setup()
{
    if (m_teardownInProgress || m_setupInProgress || isAccessible()) {
        return false;
    }
    m_setupInProgress = true;
    m_device->broadcastActionRequested("setupRequested");

    if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Hal.Device.Volume.Crypto")) {
        return requestPassphrase();
    } else if (FstabHandling::isInFstab(m_device->property("block.device").toString())) {
        return callSystemMount();
    } else {
        return callHalVolumeMount();
    }
}

bool StorageAccess::teardown()
{
    if (m_teardownInProgress || m_setupInProgress || !isAccessible()) {
        return false;
    }
    m_teardownInProgress = true;
    m_device->broadcastActionRequested("teardownRequested");

    if (m_device->property("info.interfaces").toStringList().contains("org.freedesktop.Hal.Device.Volume.Crypto")) {
        return callCryptoTeardown();
    } else if (FstabHandling::isInFstab(m_device->property("block.device").toString())) {
        return callSystemUnmount();
    } else {
        return callHalVolumeUnmount();
    }
}

void StorageAccess::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("volume.is_mounted"))
    {
        emit accessibilityChanged(isAccessible(), m_device->udi());
    }
}

void StorageAccess::slotDBusReply(const QDBusMessage &/*reply*/)
{
    if (m_setupInProgress) {
        m_setupInProgress = false;
        m_device->broadcastActionDone("setupDone", Solid::NoError, QVariant(), m_device->udi());
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        m_device->broadcastActionDone("teardownDone", Solid::NoError, QVariant(), m_device->udi());

        HalDevice drive(m_device->property("block.storage_device").toString());
        if (drive.property("storage.drive_type").toString()!="cdrom"
         && drive.property("storage.requires_eject").toBool()) {

            QString devnode = m_device->property("block.device").toString();

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

            m_ejectInProgress = true;
            m_process = FstabHandling::callSystemCommand("eject", args,
                                                         this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
        }
    } else if (m_ejectInProgress) {
        m_ejectInProgress = false;
        m_device->broadcastActionDone("ejectDone", Solid::NoError, QVariant(), m_device->udi());
    }
}

void StorageAccess::slotDBusError(const QDBusError &error)
{
    // TODO: Better error reporting here
    if (m_setupInProgress) {
        m_setupInProgress = false;
        m_device->broadcastActionDone("setupDone", Solid::UnauthorizedOperation,
                       QString(error.name()+": "+error.message()),
                       m_device->udi());
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        m_device->broadcastActionDone("teardownDone", Solid::UnauthorizedOperation,
                          QString(error.name()+": "+error.message()),
                          m_device->udi());
    } else if (m_ejectInProgress) {
        m_ejectInProgress = false;
        m_device->broadcastActionDone("ejectDone", Solid::UnauthorizedOperation,
                       QString(error.name()+": "+error.message()),
                       m_device->udi());
    }
}

void Solid::Backends::Hal::StorageAccess::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (m_setupInProgress) {
        m_setupInProgress = false;

        if (exitCode==0) {
            m_device->broadcastActionDone("setupDone", Solid::NoError, QVariant(), m_device->udi());
        } else {
            m_device->broadcastActionDone("setupDone", Solid::UnauthorizedOperation,
                           m_process->readAllStandardError(),
                           m_device->udi());
        }
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        if (exitCode==0) {
            m_device->broadcastActionDone("teardownDone", Solid::NoError, QVariant(), m_device->udi());
        } else {
            m_device->broadcastActionDone("teardownDone", Solid::UnauthorizedOperation,
                              m_process->readAllStandardError(),
                              m_device->udi());
        }
    } else if (m_ejectInProgress) {
        if (exitCode==0)  {
            m_ejectInProgress = false;
            m_device->broadcastActionDone("ejectDone", Solid::NoError, QVariant(), m_device->udi());
        } else {
            callHalVolumeEject();
        }
    }

    delete m_process;
}

void StorageAccess::slotSetupRequested()
{
    m_setupInProgress = true;
    emit setupRequested(m_device->udi());
}

void StorageAccess::slotTeardownRequested()
{
    m_teardownInProgress = true;
    emit teardownRequested(m_device->udi());
}

QString generateReturnObjectPath()
{
    static int number = 1;

    return "/org/kde/solid/HalStorageAccess_"+QString::number(number++);
}

bool StorageAccess::requestPassphrase()
{
    QString udi = m_device->udi();
    QString returnService = QDBusConnection::sessionBus().baseService();
    m_lastReturnObject = generateReturnObjectPath();

    QDBusConnection::sessionBus().registerObject(m_lastReturnObject, this,
                                                 QDBusConnection::ExportScriptableSlots);


    QWidget *activeWindow = QApplication::activeWindow();
    uint wId = 0;
    if (activeWindow!=0) {
        wId = (uint)activeWindow->winId();
    }

    QString appId = QCoreApplication::applicationName();

    QDBusInterface soliduiserver("org.kde.kded", "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showPassphraseDialog", udi,
                                                returnService, m_lastReturnObject,
                                                wId, appId);
    m_passphraseRequested = reply.isValid();
    if (!m_passphraseRequested) {
        qWarning() << "Failed to call the SolidUiServer, D-Bus said:" << reply.error();
    }
    return m_passphraseRequested;
}

void StorageAccess::passphraseReply(const QString &passphrase)
{
    if (m_passphraseRequested) {
        QDBusConnection::sessionBus().unregisterObject(m_lastReturnObject);
        m_passphraseRequested = false;
        if (!passphrase.isEmpty()) {
            callCryptoSetup(passphrase);
        } else {
            m_setupInProgress = false;
            m_device->broadcastActionDone("setupDone", Solid::NoError, QVariant(), m_device->udi());
        }
    }
}

bool StorageAccess::callHalVolumeMount()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      "org.freedesktop.Hal.Device.Volume",
                                                      "Mount");

    // HAL 0.5.12 supports using alternative drivers for the same filesystem.
    // This is mainly used to integrate the ntfs-3g driver.
    // Unfortunately, the primary driver gets used unless we
    // specify some other driver (fstype) to the Mount method.
    // TODO: Allow the user to choose the driver.

    QString fstype = m_device->property("volume.fstype").toString();
    QStringList halOptions = m_device->property("volume.mount.valid_options").toStringList();

    QString alternativePreferred = m_device->property("volume.fstype.alternative.preferred").toString();
    if (!alternativePreferred.isEmpty()) {
        QStringList alternativeFstypes = m_device->property("volume.fstype.alternative").toStringList();
        if (alternativeFstypes.contains(alternativePreferred)) {
            fstype = alternativePreferred;
            halOptions = m_device->property("volume.mount."+fstype+".valid_options").toStringList();
        }
    }

    QStringList options;

#ifdef Q_OS_FREEBSD
    QString uid="-u=";
#else
    QString uid="uid=";
#endif
    if (halOptions.contains(uid)) {
        options << uid+QString::number(::getuid());
    }

#ifdef Q_OS_FREEBSD
    char *cType;
    if ( fstype=="vfat" && halOptions.contains("-L=")) {
        if ( (cType = getenv("LC_ALL")) || (cType = getenv("LC_CTYPE")) || (cType = getenv("LANG")) )
              options << "-L="+QString(cType);
    }
    else if ( (fstype.startsWith(QLatin1String("ntfs")) || fstype=="iso9660" || fstype=="udf") && halOptions.contains("-C=") ) {
        if ((cType = getenv("LC_ALL")) || (cType = getenv("LC_CTYPE")) || (cType = getenv("LANG")) )
            options << "-C="+QString(nl_langinfo(CODESET));
    }
#else
    if (fstype=="vfat" || fstype=="ntfs" || fstype=="iso9660" || fstype=="udf" ) {
        if (halOptions.contains("utf8"))
            options<<"utf8";
        else if (halOptions.contains("iocharset="))
            options<<"iocharset=utf8";
        if (halOptions.contains("shortname="))
            options<<"shortname=mixed";
        if (halOptions.contains("flush"))
            options<<"flush";
    }
    // pass our locale to the ntfs-3g driver so it can translate local characters
    else if ( halOptions.contains("locale=") ) {
        // have to obtain LC_CTYPE as returned by the `locale` command
        // check in the same order as `locale` does
        char *cType;
        if ( (cType = getenv("LC_ALL")) || (cType = getenv("LC_CTYPE")) || (cType = getenv("LANG")) ) {
            options << "locale="+QString(cType);
        }
    }
#endif

    msg << "" << fstype << options;

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

bool StorageAccess::callHalVolumeUnmount()
{
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

bool StorageAccess::callHalVolumeEject()
{
    QString udi = m_device->udi();
    QString interface = "org.freedesktop.Hal.Device.Volume";

    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      interface, "Eject");

    msg << QStringList();

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

bool Solid::Backends::Hal::StorageAccess::callSystemMount()
{
    const QString device = m_device->property("block.device").toString();
    m_process = FstabHandling::callSystemCommand("mount", device,
                                                 this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
}

bool Solid::Backends::Hal::StorageAccess::callSystemUnmount()
{
    const QString device = m_device->property("block.device").toString();
    m_process = FstabHandling::callSystemCommand("umount", device,
                                                 this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));

    return m_process!=0;
}

void StorageAccess::callCryptoSetup(const QString &passphrase)
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      "org.freedesktop.Hal.Device.Volume.Crypto",
                                                      "Setup");

    msg << passphrase;

    c.callWithCallback(msg, this,
                       SLOT(slotDBusReply(const QDBusMessage &)),
                       SLOT(slotDBusError(const QDBusError &)));
}

bool StorageAccess::callCryptoTeardown()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", udi,
                                                      "org.freedesktop.Hal.Device.Volume.Crypto",
                                                      "Teardown");

    return c.callWithCallback(msg, this,
                              SLOT(slotDBusReply(const QDBusMessage &)),
                              SLOT(slotDBusError(const QDBusError &)));
}

#include "backends/hal/halstorageaccess.moc"

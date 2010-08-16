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

#ifndef UDISKSSTORAGEACCESS_H
#define UDISKSSTORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>
#include "udisksdeviceinterface.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusError>

namespace Solid
{
namespace Backends
{
namespace UDisks
{
class UDisksStorageAccess : public DeviceInterface, virtual public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    UDisksStorageAccess(UDisksDevice *device);
    virtual ~UDisksStorageAccess();

    virtual bool isAccessible() const;
    virtual QString filePath() const;
    virtual bool isIgnored() const;
    virtual bool setup();
    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupRequested(const QString &udi);
    void teardownRequested(const QString &udi);

public Q_SLOTS:
    Q_SCRIPTABLE Q_NOREPLY void passphraseReply( const QString & passphrase );

private Q_SLOTS:
    void slotChanged();
    void slotDBusReply( const QDBusMessage & reply );
    void slotDBusError( const QDBusError & error );

private:
    void updateCache();

    bool mount();
    bool unmount();

    bool requestPassphrase();
    void callCryptoSetup( const QString & passphrase );
    bool callCryptoTeardown();

    QString generateReturnObjectPath();

private:
    bool m_isAccessible;
    bool m_setupInProgress;
    bool m_teardownInProgress;
    bool m_passphraseRequested;
    QString m_lastReturnObject;
};
}
}
}

#endif // UDISKSSTORAGEACCESS_H

/* This file is part of libkdbus

   Copyright (c) 2011 David Faure <faure@kde.org>
   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>


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

#include "kdbusservice.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

class KDBusServicePrivate
{
public:
    KDBusServicePrivate()
        : registered(false) {}

    QString generateServiceName(KDBusService::StartupOptions options)
    {
        const QCoreApplication *app = QCoreApplication::instance();
        const QString domain = app->organizationDomain();
        const QStringList parts = domain.split(QLatin1Char('.'), QString::SkipEmptyParts);

        QString reversedDomain;
        if (parts.isEmpty()) {
            reversedDomain = QLatin1String("local.");
        } else {
            foreach (const QString& part, parts) {
                reversedDomain.prepend(QLatin1Char('.'));
                reversedDomain.prepend(part);
            }
        }

        const QString baseName = reversedDomain + app->applicationName();

        if (options & KDBusService::Multiple) {
            const QString pid = QString::number(QCoreApplication::applicationPid());
            return baseName + '-' + pid;
        } else {
            return baseName;
        }
    }

    bool registered;
    QString serviceName;
    QString errorMessage;
};

KDBusService::KDBusService(StartupOptions options, QObject *parent)
    : QObject(parent), d(new KDBusServicePrivate)
{
    QDBusConnectionInterface *bus = 0;

    if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        d->errorMessage = QLatin1String("Session bus not found\n"
                                        "To circumvent this problem try the following command (with Linux and bash)\n"
                                        "export $(dbus-launch)");
    }

    if (bus) {
        d->serviceName = d->generateServiceName(options);
        QString objectPath = '/' + d->serviceName;
        objectPath.replace(QLatin1Char('.'), QLatin1Char('/'));

        d->registered = bus->registerService(d->serviceName) == QDBusConnectionInterface::ServiceRegistered;


        if (!d->registered) {
            if (options & Unique) {
                // Already running so it's ok!
                QDBusInterface iface(d->serviceName, objectPath);
                // TODO: Increase the timeout once David's patch is available in stable Qt
                QDBusReply<int> reply = iface.call(QLatin1String("Activate"));
                if (reply.isValid()) {
                    exit(reply);
                } else {
                    d->errorMessage = reply.error().message();
                }

            } else {
                d->errorMessage = QLatin1String("Couldn't register name '")
                                + d->serviceName
                                + QLatin1String("' with DBUS - another process owns it already!");
            }

        } else {
            //TODO: handle aboutToQuit() on the app
            QDBusConnection::sessionBus().registerObject(QLatin1String("/MainApplication"), QCoreApplication::instance(),
                                                         QDBusConnection::ExportScriptableSlots |
                                                         QDBusConnection::ExportScriptableProperties |
                                                         QDBusConnection::ExportAdaptors);
            QDBusConnection::sessionBus().registerObject(objectPath, this,
                                                         QDBusConnection::ExportScriptableSlots |
                                                         QDBusConnection::ExportScriptableProperties |
                                                         QDBusConnection::ExportAdaptors);
        }
    }

    if (!d->registered && ((options & NoExitOnFailure) == 0)) {
        qCritical() << d->errorMessage;
        exit(1);
    }
}

KDBusService::~KDBusService()
{
    delete d;
}

bool KDBusService::isRegistered() const
{
    return d->registered;
}

QString KDBusService::errorMessage() const
{
    return d->errorMessage;
}

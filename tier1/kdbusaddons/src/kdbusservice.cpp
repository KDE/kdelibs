/* This file is part of libkdbusaddons

   Copyright (c) 2011 David Faure <faure@kde.org>
   Copyright (c) 2011 Kevin Ottens <ervin@kde.org>

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
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdbusservice.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include "kdbusservice_adaptor.h"

class KDBusServicePrivate
{
public:
    KDBusServicePrivate()
        : registered(false) {}

    QString generateServiceName()
    {
        const QCoreApplication *app = QCoreApplication::instance();
        const QString domain = app->organizationDomain();
        const QStringList parts = domain.split(QLatin1Char('.'), QString::SkipEmptyParts);

        QString reversedDomain;
        if (parts.isEmpty()) {
            reversedDomain = QLatin1String("local.");
        } else {
            Q_FOREACH (const QString& part, parts) {
                reversedDomain.prepend(QLatin1Char('.'));
                reversedDomain.prepend(part);
            }
        }

        return reversedDomain + app->applicationName();
    }

    bool registered;
    QString serviceName;
    QString errorMessage;
};

KDBusService::KDBusService(StartupOptions options, QObject *parent)
    : QObject(parent), d(new KDBusServicePrivate)
{
    new KDBusServiceAdaptor(this);
    QDBusConnectionInterface *bus = 0;

    if (!QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        d->errorMessage = QLatin1String("Session bus not found\n"
                                        "To circumvent this problem try the following command (with Linux and bash)\n"
                                        "export $(dbus-launch)");
    }

    if (bus) {
        d->serviceName = d->generateServiceName();
        QString objectPath = QLatin1Char('/') + d->serviceName;
        objectPath.replace(QLatin1Char('.'), QLatin1Char('/'));

        if (options & Multiple) {
            const QString pid = QString::number(QCoreApplication::applicationPid());
            d->serviceName += QLatin1Char('-') + pid;
        }

        QDBusConnection::sessionBus().registerObject(QLatin1String("/MainApplication"), QCoreApplication::instance(),
                                                     QDBusConnection::ExportScriptableSlots |
                                                     QDBusConnection::ExportScriptableProperties |
                                                     QDBusConnection::ExportAdaptors);
        QDBusConnection::sessionBus().registerObject(objectPath, this,
                                                     QDBusConnection::ExportAdaptors);

        d->registered = bus->registerService(d->serviceName) == QDBusConnectionInterface::ServiceRegistered;


        if (!d->registered) {
            if (options & Unique) {
                // Already running so it's ok!
                QDBusInterface iface(d->serviceName, objectPath);
                iface.setTimeout(5 * 60 * 1000); // Application can take time to answer
                QVariantMap platform_data;

                // TODO getter for startup id in qapp (documented to be empty after showing the first window)
                // platform_data.insert("desktop-startup-id", ?);
                QDBusReply<void> reply = iface.call(QLatin1String("Activate"), platform_data);
                if (reply.isValid()) {
                    exit(0);
                } else {
                    d->errorMessage = reply.error().message();
                }
            } else {
                d->errorMessage = QLatin1String("Couldn't register name '")
                                + d->serviceName
                                + QLatin1String("' with DBUS - another process owns it already!");
            }

        } else {
            if (QCoreApplication* app = QCoreApplication::instance()) {
                connect(app, SIGNAL(aboutToQuit()), this, SLOT(unregister()));
            }
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

void KDBusService::unregister()
{
    QDBusConnectionInterface *bus = 0;
    if (!d->registered || !QDBusConnection::sessionBus().isConnected() || !(bus = QDBusConnection::sessionBus().interface())) {
        return;
    }
    bus->unregisterService(d->serviceName);
}

void KDBusService::Activate(const QVariantMap &platform_data)
{
    // TODO (via hook) KStartupInfo::setStartupId(platform_data.value("desktop-startup-id"))
    Q_EMIT activateRequested();
    // TODO (via hook) KStartupInfo::appStarted(platform_data.value("desktop-startup-id"))
}

void KDBusService::Open(const QStringList &uris, const QVariantMap &platform_data)
{
    // TODO (via hook) KStartupInfo::setStartupId(platform_data.value("desktop-startup-id"))
    Q_EMIT openRequested(QUrl::fromStringList(uris));
    // TODO (via hook) KStartupInfo::appStarted(platform_data.value("desktop-startup-id"))
}

void KDBusService::ActivateAction(const QString &action_name, const QVariantList &maybeParameter, const QVariantMap &platform_data)
{
    // TODO (via hook) KStartupInfo::setStartupId(platform_data.value("desktop-startup-id"))
    // This is a workaround for DBus not supporting null variants.
    const QVariant param = maybeParameter.count() == 1 ? maybeParameter.first() : QVariant();
    Q_EMIT activateActionRequested(action_name, param);
    // TODO (via hook) KStartupInfo::appStarted(platform_data.value("desktop-startup-id"))
}

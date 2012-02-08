/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "accessmanager.h"
#include "private/accessmanager_p.h"

#include "authorizationmanager.h"
#include "authorizationmanager_p.h"
#include "service.h"
#include "serviceaccessjob.h"

#include "config-plasma.h"

#include <QtCore/QMap>
#include <QtCore/QTimer>

#include <dnssd/remoteservice.h>
#include <dnssd/servicebrowser.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <QtJolie/Message>

namespace Plasma
{

class RemoteObjectDescription::Private
{
public:
    QString name;
    QString description;
    QString icon;
    KUrl url;
};

RemoteObjectDescription::RemoteObjectDescription()
    : d(new Private)
{
}

RemoteObjectDescription::RemoteObjectDescription(const RemoteObjectDescription &other)
    : d(new Private(*other.d))
{
}

RemoteObjectDescription &RemoteObjectDescription::operator=(const RemoteObjectDescription &other)
{
    *d = *other.d;
    return *this;
}


void RemoteObjectDescription::setName(const QString &name)
{
    d->name = name;
}

QString RemoteObjectDescription::name() const
{
    return d->name;
}


void RemoteObjectDescription::setUrl(const KUrl &url)
{
    d->url = url;
}

KUrl RemoteObjectDescription::url() const
{
    return d->url;
}

void RemoteObjectDescription::setDescription(const QString &description)
{
    d->description = description;
}

QString RemoteObjectDescription::description() const
{
    return d->description;
}

void RemoteObjectDescription::setIcon(const QString &icon)
{
    d->icon = icon;
}

QString RemoteObjectDescription::icon() const
{
    return d->icon;
}



class AccessManagerSingleton
{
    public:
        AccessManager self;
};

K_GLOBAL_STATIC(AccessManagerSingleton, privateAccessManagerSelf)

AccessManager *AccessManager::self()
{
    return &privateAccessManagerSelf->self;
}

AccessManager::AccessManager()
    : QObject(),
      d(new AccessManagerPrivate(this))
{
    KGlobal::dirs()->addResourceType("trustedkeys", "config", "trustedkeys/");
}

AccessManager::~AccessManager()
{
    delete d;
}

AccessAppletJob *AccessManager::accessRemoteApplet(const KUrl &location) const
{
    AuthorizationManager::self()->d->prepareForServiceAccess();

    KUrl resolvedLocation;
    if (location.scheme() == "plasma+zeroconf") {
        if (d->zeroconfServices.contains(location.host())) {
            resolvedLocation = d->services[location.host()].url();
        } else {
#ifndef NDEBUG
            kDebug() << "There's no zeroconf service with this name.";
#endif
        }
    } else {
        resolvedLocation = location;
    }

    AccessAppletJob *job = new AccessAppletJob(resolvedLocation);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotJobFinished(KJob*)));
    QTimer::singleShot(0, job, SLOT(slotStart()));
    return job;
}

QList<RemoteObjectDescription> AccessManager::remoteApplets() const
{
    return d->services.values();
}

QStringList AccessManager::supportedProtocols()
{
    QStringList list;
    list << "plasma" << "plasma+zeroconf";
    return list;
}

AccessManagerPrivate::AccessManagerPrivate(AccessManager *manager)
    : q(manager),
      browser(new DNSSD::ServiceBrowser("_plasma._tcp"))
{
#ifdef ENABLE_REMOTE_WIDGETS
    q->connect(browser, SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),
               q, SLOT(slotAddService(DNSSD::RemoteService::Ptr)));
    q->connect(browser, SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr)),
               q, SLOT(slotRemoveService(DNSSD::RemoteService::Ptr)));
    browser->startBrowse();
#else
    kWarning() << "libplasma is compiled without support for remote widgets. not monitoring remote widgets on the network";
#endif
}

AccessManagerPrivate::~AccessManagerPrivate()
{
    delete browser;
}

void AccessManagerPrivate::slotJobFinished(KJob *job)
{
    emit q->finished(static_cast<AccessAppletJob*>(job));
}

void AccessManagerPrivate::slotAddService(DNSSD::RemoteService::Ptr service)
{
#ifndef NDEBUG
    kDebug();
#endif
    if (!service->resolve()) {
#ifndef NDEBUG
        kDebug() << "Zeroconf service can't be resolved";
#endif
        return;
    }

    if (!services.contains(service->serviceName())) {
        RemoteObjectDescription metadata;
#ifndef NDEBUG
        kDebug() << "textdata = " << service->textData();
#endif
#ifndef NDEBUG
        kDebug() << "hostname: " << service->hostName();
#endif
        QHostAddress address = DNSSD::ServiceBrowser::resolveHostName(service->hostName());
        QString ip = address.toString();
#ifndef NDEBUG
        kDebug() << "result for resolve = " << ip;
#endif

        KUrl url(QString("plasma://%1:%2/%3").arg(ip)
                                             .arg(service->port())
                                             .arg(service->serviceName()));

        if (service->textData().isEmpty()) {
#ifndef NDEBUG
            kDebug() << "no textdata?";
#endif
            metadata.setName(service->serviceName());
            metadata.setUrl(url);
        } else {
#ifndef NDEBUG
            kDebug() << "service has got textdata";
#endif
            QMap<QString, QByteArray> textData = service->textData();
            metadata.setName(textData["name"]);
            metadata.setDescription(textData["description"]);
            metadata.setIcon(textData["icon"]);
            metadata.setUrl(url);
        }

#ifndef NDEBUG
        kDebug() << "location = " << metadata.url();
#endif
#ifndef NDEBUG
        kDebug() << "name = " << metadata.name();
#endif
#ifndef NDEBUG
        kDebug() << "description = " << metadata.name();
#endif

        services[service->serviceName()] = metadata;
        zeroconfServices[service->serviceName()] = service;
        emit q->remoteAppletAnnounced(metadata);
    }
}

void AccessManagerPrivate::slotRemoveService(DNSSD::RemoteService::Ptr service)
{
#ifndef NDEBUG
    kDebug();
#endif
    emit q->remoteAppletUnannounced(services[service->serviceName()]);
    services.remove(service->serviceName());
    zeroconfServices.remove(service->serviceName());
}

} // Plasma namespace


#include "moc_accessmanager.cpp"

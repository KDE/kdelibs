/*
    This file is part of the Nepomuk KDE project.
    Copyright (C) 2011  Vishesh Handa <handa.vish@gmail.com>
    Copyright (C) 2011 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "resourcewatcher.h"
#include "resourcewatcherconnectioninterface.h"
#include "resourcewatchermanagerinterface.h"

#include <QtDBus/QDBusObjectPath>

#include "resource.h"

#include "kurl.h"
#include "kdebug.h"

namespace {
QList<QUrl> convertUris(const QStringList& uris) {
    QList<QUrl> us;
    foreach(const QString& uri, uris) {
        us << KUrl(uri);
    }
    return us;
}
}

class Nepomuk::ResourceWatcher::Private {
public:
    QList<Types::Class> m_types;
    QList<Nepomuk::Resource> m_resources;
    QList<Types::Property> m_properties;

    org::kde::nepomuk::ResourceWatcherConnection * m_connectionInterface;
    org::kde::nepomuk::ResourceWatcher * m_watchManagerInterface;
};

Nepomuk::ResourceWatcher::ResourceWatcher(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    d->m_watchManagerInterface
            = new org::kde::nepomuk::ResourceWatcher( "org.kde.nepomuk.DataManagement",
                                                      "/resourcewatcher",
                                                      QDBusConnection::sessionBus() );
    d->m_connectionInterface = 0;
}

Nepomuk::ResourceWatcher::~ResourceWatcher()
{
    stop();
    delete d;
}

bool Nepomuk::ResourceWatcher::start()
{
    //
    // Convert to list of strings
    //
    QList<QString> uris;
    foreach( const Nepomuk::Resource & res, d->m_resources ) {
        uris << KUrl(res.resourceUri()).url();
    }

    QList<QString> props;
    foreach( const Types::Property & prop, d->m_properties ) {
        props << KUrl(prop.uri()).url();
    }

    QList<QString> types_;
    foreach( const Types::Class & cl, d->m_types ) {
        types_ << KUrl(cl.uri()).url();
    }

    //
    // Create the dbus object to watch
    //
    QDBusPendingReply<QDBusObjectPath> reply = d->m_watchManagerInterface->watch( uris, props, types_ );
    QDBusObjectPath path = reply.value();

    if(!path.path().isEmpty()) {
        d->m_connectionInterface = new org::kde::nepomuk::ResourceWatcherConnection( "org.kde.nepomuk.DataManagement",
                                                                                     path.path(),
                                                                                     QDBusConnection::sessionBus() );
        connect( d->m_connectionInterface, SIGNAL(propertyAdded(QString,QString,QDBusVariant)),
                 this, SLOT(slotPropertyAdded(QString,QString,QDBusVariant)) );
        connect( d->m_connectionInterface, SIGNAL(propertyRemoved(QString,QString,QDBusVariant)),
                 this, SLOT(slotPropertyRemoved(QString,QString,QDBusVariant)) );
        connect( d->m_connectionInterface, SIGNAL(resourceCreated(QString,QStringList)),
                 this, SLOT(slotResourceCreated(QString,QStringList)) );
        connect( d->m_connectionInterface, SIGNAL(propertyChanged(QString,QString,QVariantList,QVariantList)),
                 this, SLOT(slotPropertyChanged(QString,QString,QVariantList,QVariantList)) );
        connect( d->m_connectionInterface, SIGNAL(resourceRemoved(QString,QStringList)),
                 this, SLOT(slotResourceRemoved(QString,QStringList)) );
        connect( d->m_connectionInterface, SIGNAL(resourceTypeAdded(QString,QString)),
                 this, SLOT(slotResourceTypeAdded(QString,QString)) );
        connect( d->m_connectionInterface, SIGNAL(resourceTypeRemoved(QString,QString)),
                 this, SLOT(slotResourceTypeRemoved(QString,QString)) );
        return true;
    }
    else {
        return false;
    }
}

void Nepomuk::ResourceWatcher::stop()
{
    if (d->m_connectionInterface) {
        d->m_connectionInterface->close();
        delete d->m_connectionInterface;
        d->m_connectionInterface = 0;
    }
}

void Nepomuk::ResourceWatcher::addProperty(const Nepomuk::Types::Property& property)
{
    d->m_properties << property;
}

void Nepomuk::ResourceWatcher::addResource(const Nepomuk::Resource& res)
{
    d->m_resources << res;
}

void Nepomuk::ResourceWatcher::addType(const Nepomuk::Types::Class& type)
{
    d->m_types << type;
}

QList< Nepomuk::Types::Property > Nepomuk::ResourceWatcher::properties() const
{
    return d->m_properties;
}

QList<Nepomuk::Resource> Nepomuk::ResourceWatcher::resources() const
{
    return d->m_resources;
}

QList< Nepomuk::Types::Class > Nepomuk::ResourceWatcher::types() const
{
    return d->m_types;
}

void Nepomuk::ResourceWatcher::setProperties(const QList< Nepomuk::Types::Property >& properties_)
{
    d->m_properties = properties_;
}

void Nepomuk::ResourceWatcher::setResources(const QList< Nepomuk::Resource >& resources_)
{
    d->m_resources = resources_;
}

void Nepomuk::ResourceWatcher::setTypes(const QList< Nepomuk::Types::Class >& types_)
{
    d->m_types = types_;
}

void Nepomuk::ResourceWatcher::slotResourceCreated(const QString &res, const QStringList &types)
{
    emit resourceCreated(Nepomuk::Resource::fromResourceUri(KUrl(res)), convertUris(types));
}

void Nepomuk::ResourceWatcher::slotResourceRemoved(const QString &res, const QStringList &types)
{
    emit resourceRemoved(KUrl(res), convertUris(types));
}

void Nepomuk::ResourceWatcher::slotResourceTypeAdded(const QString &res, const QString &type)
{
    emit resourceTypeAdded(KUrl(res), KUrl(type));
}

void Nepomuk::ResourceWatcher::slotResourceTypeRemoved(const QString &res, const QString &type)
{
    emit resourceTypeRemoved(KUrl(res), KUrl(type));
}

void Nepomuk::ResourceWatcher::slotPropertyAdded(const QString& res, const QString& prop, const QDBusVariant& object)
{
    emit propertyAdded( Resource::fromResourceUri(KUrl(res)), Types::Property( KUrl(prop) ), object.variant() );
}

void Nepomuk::ResourceWatcher::slotPropertyRemoved(const QString& res, const QString& prop, const QDBusVariant& object)
{
    emit propertyRemoved( Resource::fromResourceUri(KUrl(res)), Types::Property( KUrl(prop) ), object.variant() );
}

void Nepomuk::ResourceWatcher::slotPropertyChanged(const QString& res, const QString& prop, const QVariantList& oldObjs, const QVariantList& newObjs)
{
    emit propertyChanged( Resource::fromResourceUri(KUrl(res)), Types::Property( KUrl(prop) ),
                          oldObjs, newObjs );
}

#include "resourcewatcher.moc"


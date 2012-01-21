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
    QString convertUri(const QUrl& uri) {
        return KUrl(uri).url();
    }

    QStringList convertUris(const QList<QUrl>& uris) {
        QStringList cs;
        foreach(const QUrl& uri, uris) {
            cs << convertUri(uri);
        }
        return cs;
    }

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
    QList<QUrl> m_types;
    QList<QUrl> m_resources;
    QList<QUrl> m_properties;

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
    QList<QString> uris = convertUris(d->m_resources);
    QList<QString> props = convertUris(d->m_properties);
    QList<QString> types_ = convertUris(d->m_types);

    //
    // Create the dbus object to watch
    //
    QDBusPendingReply<QDBusObjectPath> reply = d->m_watchManagerInterface->watch( uris, props, types_ );
    QDBusObjectPath path = reply.value();

    if(!path.path().isEmpty()) {
        d->m_connectionInterface = new org::kde::nepomuk::ResourceWatcherConnection( "org.kde.nepomuk.DataManagement",
                                                                                     path.path(),
                                                                                     QDBusConnection::sessionBus() );
        connect( d->m_connectionInterface, SIGNAL(propertyAdded(QString,QString,QVariantList)),
                 this, SLOT(slotPropertyAdded(QString,QString,QVariantList)) );
        connect( d->m_connectionInterface, SIGNAL(propertyRemoved(QString,QString,QVariantList)),
                 this, SLOT(slotPropertyRemoved(QString,QString,QVariantList)) );
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
    d->m_properties << property.uri();
    if(d->m_connectionInterface) {
        d->m_connectionInterface->addProperty(convertUri(property.uri()));
    }
}

void Nepomuk::ResourceWatcher::addResource(const QUrl& res)
{
    d->m_resources << res;
    if(d->m_connectionInterface) {
        d->m_connectionInterface->addResource(convertUri(res));
    }
}

void Nepomuk::ResourceWatcher::addType(const Nepomuk::Types::Class& type)
{
    d->m_types << type.uri();
    if(d->m_connectionInterface) {
        d->m_connectionInterface->addType(convertUri(type.uri()));
    }
}

void Nepomuk::ResourceWatcher::removeProperty(const Nepomuk::Types::Property& property)
{
    d->m_properties.removeAll(property.uri());
    if(d->m_connectionInterface) {
        d->m_connectionInterface->removeProperty(convertUri(property.uri()));
    }
}

void Nepomuk::ResourceWatcher::removeResource(const QUrl& res)
{
    d->m_resources.removeAll(res);
    if(d->m_connectionInterface) {
        d->m_connectionInterface->removeResource(convertUri(res));
    }
}

void Nepomuk::ResourceWatcher::removeType(const Nepomuk::Types::Class& type)
{
    d->m_types.removeAll(type.uri());
    if(d->m_connectionInterface) {
        d->m_connectionInterface->removeType(convertUri(type.uri()));
    }
}

QList< Nepomuk::Types::Property > Nepomuk::ResourceWatcher::properties() const
{
    QList< Nepomuk::Types::Property > props;
    foreach(const QUrl& uri, d->m_properties)
        props << Types::Property(uri);
    return props;
}

QList<QUrl> Nepomuk::ResourceWatcher::resources() const
{
    return d->m_resources;
}

QList< Nepomuk::Types::Class > Nepomuk::ResourceWatcher::types() const
{
    QList<Nepomuk::Types::Class> types;
    foreach(const QUrl& uri, d->m_types)
        types << Types::Class(uri);
    return types;
}

void Nepomuk::ResourceWatcher::setProperties(const QList< Nepomuk::Types::Property >& properties_)
{
    d->m_properties.clear();
    foreach(const Nepomuk::Types::Property& p, properties_) {
        d->m_properties << p.uri();
    }

    if(d->m_connectionInterface) {
        d->m_connectionInterface->setProperties(convertUris(d->m_properties));
    }
}

void Nepomuk::ResourceWatcher::setResources(const QList<QUrl>& resources_)
{
    d->m_resources = resources_;

    if(d->m_connectionInterface) {
        d->m_connectionInterface->setResources(convertUris(d->m_resources));
    }
}

void Nepomuk::ResourceWatcher::setTypes(const QList< Nepomuk::Types::Class >& types_)
{
    d->m_types.clear();
    foreach(const Nepomuk::Types::Class& t, types_) {
        d->m_types << t.uri();
    }

    if(d->m_connectionInterface) {
        d->m_connectionInterface->setTypes(convertUris(d->m_types));
    }
}

void Nepomuk::ResourceWatcher::slotResourceCreated(const QString &res, const QStringList &types)
{
    emit resourceCreated((KUrl(res)), convertUris(types));
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

void Nepomuk::ResourceWatcher::slotPropertyAdded(const QString& res, const QString& prop, const QVariantList &objects)
{
    foreach(const QVariant& v, objects) {
        emit propertyAdded( KUrl(res), Types::Property( KUrl(prop) ), v );
    }
}

void Nepomuk::ResourceWatcher::slotPropertyRemoved(const QString& res, const QString& prop, const QVariantList &objects)
{
    foreach(const QVariant& v, objects) {
        emit propertyRemoved( KUrl(res), Types::Property( KUrl(prop) ), v );
    }
}

void Nepomuk::ResourceWatcher::slotPropertyChanged(const QString& res, const QString& prop, const QVariantList& oldObjs, const QVariantList& newObjs)
{
    emit propertyChanged( KUrl(res), Types::Property( KUrl(prop) ),
                          oldObjs, newObjs );
}

#include "resourcewatcher.moc"


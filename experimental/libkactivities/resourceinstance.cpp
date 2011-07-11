/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "resourceinstance.h"
#include "manager_p.h"

#include <QCoreApplication>

namespace Activities {

#ifdef Q_OS_WIN64
__inline int toInt(WId wid)
{
	return (int)((__int64)wid);
}

#else
__inline int toInt(WId wid)
{
	return (int)wid;
}
#endif

class ResourceInstancePrivate {
public:
    WId wid;
    ResourceInstance::OpenReason reason;
    QUrl uri;
    QString mimetype;
    QString application;

    void closeResource();
    void openResource();

    enum Type {
        Accessed = 0,
        Opened = 1,
        Modified = 2,
        Closed = 3,
        FocussedIn = 4,
        FocussedOut = 5
    };

    static void registerResourceEvent(const QString & application, WId wid, const QUrl & uri, Type event, ResourceInstance::OpenReason reason)
    {
        Manager::self()->RegisterResourceEvent(application, toInt(wid), uri.toString(), uint(event), uint(reason));
    }
};

void ResourceInstancePrivate::closeResource()
{
    registerResourceEvent(application, wid, uri, Closed, reason);
}

void ResourceInstancePrivate::openResource()
{
    registerResourceEvent(application, wid, uri, Opened, reason);
}

ResourceInstance::ResourceInstance(WId wid, OpenReason reason, const QString & application, QObject * parent)
    : QObject(parent), d(new ResourceInstancePrivate())
{
    d->wid = wid;
    d->reason = reason;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;

}

ResourceInstance::ResourceInstance(WId wid, QUrl resourceUri, const QString & mimetype, OpenReason reason, const QString & application, QObject * parent)
    : QObject(parent), d(new ResourceInstancePrivate())
{
    d->wid = wid;
    d->reason = reason;
    d->uri = resourceUri;
    d->mimetype = mimetype;
    d->application = application.isEmpty() ? QCoreApplication::instance()->applicationName() : application;

    d->openResource();
}

ResourceInstance::~ResourceInstance()
{
    d->closeResource();
    delete d;
}

void ResourceInstance::notifyModified()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::Modified, d->reason);
}

void ResourceInstance::notifyFocussedIn()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocussedIn, d->reason);
}

void ResourceInstance::notifyFocussedOut()
{
    d->registerResourceEvent(d->application, d->wid, d->uri, ResourceInstancePrivate::FocussedOut, d->reason);
}

void ResourceInstance::setUri(const QUrl & newUri)
{
    if (d->uri == newUri)
        return;

    if (!d->uri.isEmpty()) {
        d->closeResource();
    }

    d->uri = newUri;

    d->openResource();
}

void ResourceInstance::setMimetype(const QString & mimetype)
{
    d->mimetype = mimetype;
    // TODO: update the service info
    Manager::self()->RegisterResourceMimeType(d->uri.toString(), mimetype);
}

QUrl ResourceInstance::uri()
{
    return d->uri;
}

QString ResourceInstance::mimetype() const
{
    return d->mimetype;
}

WId ResourceInstance::winId() const
{
    return d->wid;
}

ResourceInstance::OpenReason ResourceInstance::openReason() const
{
    return d->reason;
}

void ResourceInstance::notifyAccessed(const QUrl & uri, const QString & application)
{
    ResourceInstancePrivate::registerResourceEvent(
            application.isEmpty() ? QCoreApplication::instance()->applicationName() : application,
            0, uri, ResourceInstancePrivate::Accessed, User);
}

} // namespace Activities

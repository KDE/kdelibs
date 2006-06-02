/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

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

#include "knotificationmanager_p.h"
#include "knotification.h"

#include <QHash>
#include <QWidget>
#include <dbus/qdbus.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

typedef QHash<QString,QString> Dict;

struct KNotificationManager::Private
{
    QHash<int , KNotification*> notifications;
    QDBusInterface *knotify;
};

KNotificationManager * KNotificationManager::s_self = 0L;

KNotificationManager * KNotificationManager::self()
{
    static KStaticDeleter<KNotificationManager> deleter;
    if(!s_self)
        deleter.setObject( s_self, new KNotificationManager() );
    return s_self;
}


KNotificationManager::KNotificationManager()
    : d(new Private)
{
    d->knotify =
        QDBus::sessionBus().findInterface(QLatin1String("org.kde.knotify"),
                                          QLatin1String("/Notify"));
    QObject::connect(d->knotify, SIGNAL(notificationClosed(int,int)),
                     this, SLOT(notificationClosed(int,int)));
    QObject::connect(d->knotify, SIGNAL(notificationActivated(int,int)),
                     this, SLOT(notificationActivated(int,int)));
}


KNotificationManager::~KNotificationManager()
{
    s_self = 0L;
    delete d->knotify;
    delete d;
}

void KNotificationManager::notificationActivated( int id, int action )
{
    kDebug(299) << k_funcinfo << id << " " << action << endl;
    if(d->notifications.contains(id))
    {
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->activate( action );
    }
}

void KNotificationManager::notificationClosed( int id )
{
    if(d->notifications.contains(id))
    {
        KNotification *n = d->notifications[id];
        d->notifications.remove(id);
        n->close();
    }
}


void KNotificationManager::close( int id )
{
    QDBusReply<void> reply = d->knotify->call("closeNotification", id);
    if (reply.isError())
    {
        kWarning(299) << k_funcinfo << "error while contacting knotify server" << endl;
    }
}

unsigned int KNotificationManager::notify( KNotification* n, const QPixmap &pix,
                                           const QStringList &actions,
                                           const KNotification::ContextList & contexts,
                                           const QString &appname)
{
    kDebug(299) << k_funcinfo << endl;
    WId winId=n->widget() ? n->widget()->topLevelWidget()->winId()  : 0;

    QByteArray pixmapData;
    QDataStream arg(&pixmapData, QIODevice::WriteOnly);
    arg << pix;

    QVariantList contextList;
    typedef QPair<QString,QString> Context;
    foreach (const Context& ctx, contexts)
        contextList << (QVariantList() << ctx.first << ctx.second);
    
    QDBusReply<int> reply =
        d->knotify->call("event.ssa(ss)ayasx", n->eventId(),
                         (appname.isEmpty() ? kapp->instanceName() : appname),
                         contextList, n->text(), pixmapData, actions, qlonglong(winId));
    if (reply.isError())
    {
        kWarning(299) << k_funcinfo << "error while contacting knotify server" << endl;
    }
    else
    {
        d->notifications.insert(reply, n);
        kDebug(299) << k_funcinfo << "got id " << reply.value() << endl;
        return reply;
    }
    return 0;
}


void KNotificationManager::remove( int id)
{
    d->notifications.remove(id);
}

#include "knotificationmanager_p.moc"
